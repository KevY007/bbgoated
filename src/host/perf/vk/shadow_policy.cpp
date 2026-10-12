// See shadow_policy.h.
#include "host/perf/vk/shadow_policy.h"

#include "hle/modules.h"
#include "host/perf/bbg_perf.h"
#include "host/perf/core/shadow_policy_core.h"
#include "log.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gpu {
namespace {
std::unordered_map<std::uint64_t, bbg::PagePolicy> g_pages;  // by chunk << 32 | page
bbg::PagePolicy& page(std::uint32_t chunk, std::uint32_t p) { return g_pages[(std::uint64_t{chunk} << 32) | p]; }
unsigned pingpong_limit() {
    static const unsigned n = static_cast<unsigned>(bbg::perf_uint("SHADOW_PINGPONG", 0));
    return n;
}
std::uint64_t g_retried = 0, g_bounced_out = 0, g_policy_next = 0;
struct Range {
    std::uint64_t n[kAuditWhyCount] = {};
};
std::unordered_map<std::uint64_t, Range> g_ranges;  // by va >> 20
std::uint64_t g_why[kAuditWhyCount] = {};
std::uint64_t g_audit_next = 0, g_audit_from = 0;
const char* const kWhy[kAuditWhyCount] = {"not-mirrorable", "no-mirror", "dynamic", "gpu-written", "no-room", "no-import", "unwatchable", "rewritten"};

void policy_report() {
    if (!bbg::perf_report_due(g_policy_next)) return;
    static const char* const kHot[] = {"upstream", "frames", "never"};
    host_log("bbg shadow policy: hot=%s retried %llu bounced-out %llu (pages tracked %zu)", kHot[shadow_hot_rule()],
             static_cast<unsigned long long>(g_retried), static_cast<unsigned long long>(g_bounced_out), g_pages.size());
}
}  // namespace

int shadow_hot_rule() {
    static const int r = [] {
        const std::string v = bbg::perf_value("SHADOW_HOT");
        return v == "frames" ? kShadowHotFrames : v == "never" ? kShadowHotNever : kShadowHotUpstream;
    }();
    return r;
}

bool shadow_policy_any() {
    static const bool on = shadow_hot_rule() != kShadowHotUpstream || pingpong_limit() || shadow_retry_ms();
    return on;
}

bool shadow_policy_copy_locked(std::uint32_t chunk, std::uint32_t p, std::uint64_t now_ms) {
    bbg::PagePolicy& s = page(chunk, p);
    const bool frames = shadow_hot_rule() == kShadowHotFrames && bbg::frames_rule_copy(s, hle_video_flip_count());
    const bool bounced = pingpong_limit() && bbg::pingpong_copy(s, now_ms, pingpong_limit());
    if (bounced) ++g_bounced_out;
    policy_report();
    return frames || bounced;
}

void shadow_policy_gpu_wrote_locked(std::uint32_t chunk, std::uint32_t p) {
    if (pingpong_limit()) page(chunk, p).gpu_since_copy = true;
}

void shadow_policy_went_dynamic_locked(std::uint32_t chunk, std::uint32_t p, bool rewritten, std::uint64_t now_ms) {
    bbg::PagePolicy& s = page(chunk, p);
    s.dynamic_ms = now_ms;
    s.dynamic_rewritten = rewritten;
}

std::uint64_t shadow_retry_ms() {
    static const std::uint64_t ms = bbg::perf_uint("SHADOW_RETRY_S", 0) * 1000;
    return ms;
}

bool shadow_policy_retry_due_locked(std::uint32_t chunk, std::uint32_t p, std::uint64_t now_ms) {
    return bbg::retry_due(page(chunk, p), now_ms, shadow_retry_ms());
}

void shadow_policy_retried_locked(std::uint32_t chunk, std::uint32_t p) {
    bbg::PagePolicy& s = page(chunk, p);
    s.dynamic_rewritten = false;
    s.flips = 0;
    s.at = 0;
    s.bounces = 0;
    ++g_retried;
}

std::uint64_t shadow_floor_bytes() {
    static const std::uint64_t b = bbg::perf_uint("SHADOW_FLOOR_MB", 0) << 20;
    return b;
}

bool shadow_audit_on() {
    static const bool on = bbg::perf_on("SHADOW_AUDIT");
    return on;
}

void shadow_audit_host_locked(std::uint64_t va, std::uint64_t, int why) {
    ++g_why[why];
    ++g_ranges[va >> 20].n[why];
    if (!bbg::perf_report_due(g_audit_next)) return;
    const std::uint64_t flip = hle_video_flip_count();
    const double f = static_cast<double>(flip > g_audit_from ? flip - g_audit_from : 1);
    std::string out = "bbg shadow audit: host binds/flip by reason:";
    char b[96];
    for (int k = 0; k < kAuditWhyCount; ++k) {
        std::snprintf(b, sizeof(b), " %s %.1f", kWhy[k], g_why[k] / f);
        out += b;
    }
    std::vector<std::pair<std::uint64_t, std::uint64_t>> top;  // total, range
    for (const auto& [r, n] : g_ranges) {
        std::uint64_t t = 0;
        for (std::uint64_t x : n.n) t += x;
        top.emplace_back(t, r);
    }
    std::sort(top.rbegin(), top.rend());
    out += "; top ranges:";
    for (std::size_t i = 0; i < top.size() && i < 12; ++i) {
        const Range& n = g_ranges[top[i].second];
        const int main_why = static_cast<int>(std::max_element(n.n, n.n + kAuditWhyCount) - n.n);
        std::snprintf(b, sizeof(b), " 0x%llx=%.1f/flip(%s)", static_cast<unsigned long long>(top[i].second << 20), top[i].first / f, kWhy[main_why]);
        out += b;
    }
    host_log("%s", out.c_str());
    g_ranges.clear();
    std::fill(g_why, g_why + kAuditWhyCount, 0);
    g_audit_from = flip;
}

}  // namespace gpu
