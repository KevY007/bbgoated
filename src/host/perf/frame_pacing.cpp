// See frame_pacing.h.
#include "host/perf/frame_pacing.h"

#include "core/host_clock.h"
#include "core/portable.h"
#include "hle/modules.h"
#include "host/gpu.h"
#include "host/perf/bbg_perf.h"
#include "host/perf/core/frame_pacing_core.h"
#include "log.h"

#include <algorithm>
#include <mutex>

namespace bbg {
namespace {
unsigned ahead() {
    static const unsigned k = static_cast<unsigned>(std::min<std::uint64_t>(perf_uint("FRAMES_AHEAD", 0), 3));
    return k;
}
std::mutex g_mu;  // flips come from the command processor; the report from the same
FrameNeeds g_needs;
std::uint64_t g_waits = 0, g_wait_ns = 0, g_longest_ns = 0, g_gave_up = 0, g_next = 0, g_from_flip = 0;
constexpr std::uint64_t kGiveUpNs = 250'000'000ull;  // a GPU taken for stuck must not hold the game
}  // namespace

FramesAheadGuard::FramesAheadGuard(std::uint64_t gpu_need, bool recorded) {
    if (!ahead() || !recorded || !gpu_need) return;
    std::lock_guard<std::mutex> lk(g_mu);
    target_ = g_needs.push(gpu_need, ahead());
}

FramesAheadGuard::~FramesAheadGuard() {
    if (!target_) return;
    // Only submissions already handed over are waited for: hle_video_finish_flip
    // runs after the job's host_gpu_submit (gnm_exec.cpp, the job loop), an
    // earlier flip's work went before this one's, and a need no submission
    // has passed since is not waited for at all (FrameNeeds::push).
    // host_gpu_submissions_completed takes the renderer's lock briefly and is
    // ~0 once there is no GPU; nothing is held while this waits.
    const std::uint64_t t0 = host_clock_monotonic_ns();
    bool waited = false;
    while (host_gpu_submissions_completed() < target_) {
        if (host_clock_monotonic_ns() - t0 > kGiveUpNs) {
            std::lock_guard<std::mutex> lk(g_mu);
            ++g_gave_up;
            break;
        }
        host_sleep_us(100);
        waited = true;
    }
    std::lock_guard<std::mutex> lk(g_mu);
    if (waited) {
        const std::uint64_t ns = host_clock_monotonic_ns() - t0;
        ++g_waits;
        g_wait_ns += ns;
        g_longest_ns = std::max(g_longest_ns, ns);
    }
    if (perf_report_due(g_next)) {
        const std::uint64_t flip = hle_video_flip_count();
        host_log("bbg frames ahead (%u): waited %.1f ms over %llu flips (%llu waits), longest %.2f ms, %llu gave up", ahead(), g_wait_ns / 1e6,
                 static_cast<unsigned long long>(flip - g_from_flip), static_cast<unsigned long long>(g_waits), g_longest_ns / 1e6,
                 static_cast<unsigned long long>(g_gave_up));
        g_waits = g_wait_ns = g_longest_ns = 0;
        g_from_flip = flip;
    }
}
}  // namespace bbg
