// See redzone_audit.h.
#include "host/redzone_audit.h"

#include <algorithm>
#include <iterator>

namespace {

struct Site {
    std::uint32_t off;
    std::uint16_t len;
};

constexpr Site kSites[] = {
#include "host/redzone_sites.inc"
};
constexpr int kCount = static_cast<int>(std::size(kSites));

}  // namespace

int redzone_site_count() { return kCount; }

int redzone_site_index(std::uint64_t image_offset) {
    const Site* it = std::upper_bound(std::begin(kSites), std::end(kSites), image_offset,
                                      [](std::uint64_t v, const Site& s) { return v < s.off; });
    if (it == std::begin(kSites)) return -1;
    --it;
    return image_offset < static_cast<std::uint64_t>(it->off) + it->len ? static_cast<int>(it - std::begin(kSites)) : -1;
}

#if !defined(BBHOST_REDZONE_AUDIT_TABLE_ONLY)

#include "hle/modules.h"
#include "log.h"

#include <atomic>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>

#if defined(_WIN32)
#include <windows.h>

namespace {

std::atomic<std::uint32_t> g_hits[kCount];
std::atomic<std::uint64_t> g_write_faults{0}, g_in_sites{0};

// Ahead of the write watch's handler: counts, then lets it (or anything
// else) handle the exception as before.
LONG CALLBACK audit_veh(EXCEPTION_POINTERS* ep) {
    const EXCEPTION_RECORD* r = ep->ExceptionRecord;
    if (r->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && r->NumberParameters >= 2 && r->ExceptionInformation[0] == 1) {
        g_write_faults.fetch_add(1, std::memory_order_relaxed);
        const std::uint64_t slide = hle_image_slide(), rip = ep->ContextRecord->Rip;
        if (slide && rip >= slide) {
            const int i = redzone_site_index(rip - slide);
            if (i >= 0) {
                g_hits[i].fetch_add(1, std::memory_order_relaxed);
                g_in_sites.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

void report_forever() {
    std::uint64_t last = ~0ull;
    for (;;) {
        Sleep(30000);
        const std::uint64_t all = g_write_faults.load(), in = g_in_sites.load();
        if (all == last) continue;
        last = all;
        std::vector<std::pair<std::uint32_t, int>> top;
        for (int i = 0; i < kCount; ++i) {
            if (const std::uint32_t n = g_hits[i].load()) top.push_back({n, i});
        }
        std::sort(top.rbegin(), top.rend());
        std::string sites;
        for (std::size_t k = 0; k < top.size() && k < 8; ++k) {
            char b[48];
            // As bbhost names guest addresses: the image offset + 0x400000.
            std::snprintf(b, sizeof(b), " 0x%x=%u", kSites[top[k].second].off + 0x400000u, top[k].first);
            sites += b;
        }
        host_log("redzone audit: %llu write faults, %llu inside red-zone store sites (%zu sites hit)%s",
                 static_cast<unsigned long long>(all), static_cast<unsigned long long>(in), top.size(), sites.c_str());
    }
}

}  // namespace

void redzone_audit_install() {
    const char* e = std::getenv("BBHOST_REDZONE_AUDIT");
    if (!e || e[0] != '1') return;
    if (!AddVectoredExceptionHandler(1, audit_veh)) {
        host_log("redzone audit: could not install the handler");
        return;
    }
    std::thread(report_forever).detach();
    host_log("redzone audit: on - write faults inside the %d red-zone store sites are counted (BBHOST_REDZONE_AUDIT=1)", kCount);
}
#else
void redzone_audit_install() {}  // Linux delivers signals below the red zone: nothing to audit
#endif

#endif  // !BBHOST_REDZONE_AUDIT_TABLE_ONLY
