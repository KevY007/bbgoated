// See fold_clears.h.
#include "host/perf/vk/fold_clears.h"

#include "host/perf/bbg_perf.h"
#include "host/perf/core/fold_core.h"
#include "log.h"

#include <cstdint>
#include <string>

namespace gpu {

bool g_fold_pending = false;

namespace {
bbg::HeldClears<FoldClear> g_held;
std::uint64_t g_done_color = 0, g_done_depth = 0, g_as_before = 0, g_dropped = 0, g_next = 0, g_from = 0;
std::uint64_t key(VkImage image) { return static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(image)); }
void record(const FoldClear& c) {
    ++g_as_before;
    fold_record_as_before(c);
}
void report() {
    if (!bbg::perf_report_due(g_next)) return;
    const std::uint64_t flip = hle_video_flip_count();
    const double f = static_cast<double>(flip > g_from ? flip - g_from : 1);
    host_log("bbg fold clears: by their pass's load op colour %.1f/flip depth %.1f/flip, recorded as before %.1f/flip, dropped with "
             "their image %.1f/flip",
             g_done_color / f, g_done_depth / f, g_as_before / f, g_dropped / f);
    g_done_color = g_done_depth = g_as_before = g_dropped = 0;
    g_from = flip;
}
}  // namespace

int fold_clears_mode() {
    static const int m = [] {
        const std::string v = bbg::perf_value("FOLD_CLEARS");
        return v == "all" ? 2 : v == "depth" ? 1 : 0;
    }();
    return m;
}

void fold_hold_locked(const FoldClear& c) {
    // A newer clear of the same image stands for the held one unless it leaves
    // one of its aspects out (a depth-only clear after a fill): then the older
    // is recorded first.
    FoldClear older;
    if (g_held.take(key(c.image), older)) {
        g_fold_pending = !g_held.empty();
        if (!bbg::fold_covers(c.aspects, older.aspects)) record(older);
    }
    if (g_held.size() >= 8) fold_record_all_locked();  // a short list: past it, everything as before
    g_held.hold(key(c.image), c);
    g_fold_pending = true;
    report();
}

void fold_record_locked(const FoldClear& c) { record(c); }

bool fold_take_locked(VkImage image, FoldClear& out) {
    if (!g_held.take(key(image), out)) return false;
    g_fold_pending = !g_held.empty();
    return true;
}

void fold_record_all_locked() {
    // Only while a command buffer records: the holds are made after
    // begin_recording_locked and the submission records them before it ends,
    // so this is never false with anything held - and a Rec::begin inside
    // begin_recording_locked must not record into the buffer it is beginning.
    if (!g_fold_pending || !g.recording) return;
    const auto all = g_held.take_all();
    g_fold_pending = false;  // first: the recorder's own rec() calls must not come back here
    for (const FoldClear& c : all) record(c);
}

void fold_record_image_locked(VkImage image) {
    FoldClear c;
    if (!fold_take_locked(image, c)) return;
    record(c);  // its pass end records the other held clears first (independent images)
}

void fold_record_sampled_locked(const std::uint32_t* w) {
    // texture_view_uncached aliases a target at the T#'s base or at the V#'s byte address.
    const std::uint64_t base = tsharp_base(w);
    const std::uint64_t vbase = static_cast<std::uint64_t>(w[0]) | (static_cast<std::uint64_t>(w[1] & 0xff) << 32);
    FoldClear c;
    while (g_fold_pending && g_held.take_first([&](const FoldClear& e) { return e.base == base || e.base == vbase; }, c)) {
        g_fold_pending = !g_held.empty();
        record(c);
    }
}

void fold_forget_image_locked(VkImage image) {
    if (!g_held.drop(key(image))) return;
    g_fold_pending = !g_held.empty();
    ++g_dropped;
}

void fold_note_done_locked(bool depth) {
    ++(depth ? g_done_depth : g_done_color);
    report();
}

}  // namespace gpu
