// See async_skip.h.
#include "host/perf/vk/async_skip.h"

#include "host/gpu_internal.h"
#include "host/perf/bbg_perf.h"
#include "host/perf/core/async_skip_core.h"
#include "log.h"

namespace gpu {
namespace {
bbg::PassHistory g_history;
bbg::SkipBudget g_budget;
std::uint64_t g_flip = ~0ull, g_draws_at_flip = 0, g_last_frame_draws = 0;
std::uint64_t g_skipped = 0, g_waited = 0, g_next = 0, g_from = 0;

// The draws of the flip before this one: g.draws at this flip's first draw,
// less that at the last flip's (0 when a flip went by with no draw).
void tick(std::uint64_t flip) {
    if (flip == g_flip) return;
    const std::uint64_t draws = g.draws.load(std::memory_order_relaxed);
    g_last_frame_draws = flip == g_flip + 1 ? draws - g_draws_at_flip : 0;
    g_draws_at_flip = draws;
    g_flip = flip;
}
// At flip 16 and every 300 flips: the draws since the last line that waited
// for a worker's pipeline, skipped or built in place.
void report() {
    if (!bbg::perf_report_due(g_next)) return;
    const std::uint64_t flip = hle_video_flip_count();
    const double f = static_cast<double>(flip > g_from ? flip - g_from : 1);
    host_log("bbg async skip: %.2f draws/flip skipped while their pipeline compiled, %.2f built in place", g_skipped / f, g_waited / f);
    g_skipped = g_waited = 0;
    g_from = flip;
}
}  // namespace

bool async_skip_on() {
    static const bool on = bbg::perf_on("ASYNC_SKIP");
    return on;
}

bool async_skip_draw_locked(const AsyncSkipDraw& a) {
    if (!async_skip_on()) return false;
    const std::uint64_t flip = hle_video_flip_count();
    tick(flip);
    g_history.note(a.pass, flip);  // a skipped draw's pass counts as drawn: its pass stays "every frame"
    bbg::SkipDraw d;
    d.count = a.count;
    d.indirect = a.indirect;
    d.tessellated = a.tessellated;
    d.stores = a.stores;
    d.occlusion = a.occlusion;
    const bool skip = bbg::skip_eligible(d, g_last_frame_draws, loading_screen_now(), g_history.every_frame(a.pass, flip, 8)) &&
                      g_budget.allow(a.pipeline_key, flip, 45);
    ++(skip ? g_skipped : g_waited);
    report();
    return skip;
}

void async_skip_note_pass_locked(std::uint64_t pass) {
    if (!async_skip_on()) return;
    const std::uint64_t flip = hle_video_flip_count();
    tick(flip);
    g_history.note(pass, flip);
    report();  // from every drawn draw too: a window with nothing to compile still says so
}

}  // namespace gpu
