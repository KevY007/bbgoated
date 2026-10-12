// host/perf/core/fold_core.h: clears held for the target's next pass (bbgoated).
#include "host/perf/core/fold_core.h"

#include <cstdio>

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    bbg::HeldClears<int> h;
    h.hold(10, 1);
    h.hold(20, 2);
    h.hold(10, 3);  // a newer clear of image 10
    check(h.size() == 2, "newer_replaces: one entry per image");
    int v = 0;
    check(h.take(10, v) && v == 3, "newer_replaces: the newer clear is the one held");
    h.hold(10, 3);
    check(h.take(20, v) && v == 2, "take_removes: the pass takes image 20's clear");
    check(!h.has(20), "take_removes: gone after the take");
    v = 99;
    check(!h.take(20, v) && v == 99, "take_removes: a second take finds nothing and leaves its argument");
    check(h.drop(10) && h.empty(), "forget_drops: a destroyed image's clear goes");
    check(!h.drop(10), "forget_drops: nothing left to drop");
    h.hold(1, 11);
    h.hold(2, 12);
    h.hold(3, 13);
    const auto all = h.take_all();
    check(all.size() == 3 && all[0] == 11 && all[1] == 12 && all[2] == 13 && h.empty(), "materialize_in_order: oldest first, list emptied");
    check(h.take_all().empty(), "materialize_in_order: an empty list gives nothing");

    // A sampled binding names its target by address, not by image: the first
    // held clear whose entry matches is taken, the others stay.
    h.hold(1, 100);
    h.hold(2, 200);
    h.hold(3, 300);
    check(h.take_first([](int e) { return e >= 200; }, v) && v == 200 && h.size() == 2 && !h.has(2), "take_first: the oldest match");
    check(!h.take_first([](int e) { return e == 7; }, v) && v == 200 && h.size() == 2, "take_first: no match leaves the list");

    // A newer clear stands for an older one of the same image only when it
    // clears every aspect the older one did (a fill clears depth and stencil;
    // a draw's DB_RENDER_CONTROL clear may clear depth alone).
    const std::uint32_t depth = 2, stencil = 4, color = 1;  // VK_IMAGE_ASPECT_*_BIT
    check(bbg::fold_covers(color, color), "covers: colour over colour");
    check(bbg::fold_covers(depth | stencil, depth), "covers: depth and stencil over depth");
    check(!bbg::fold_covers(depth, depth | stencil), "covers: depth alone leaves the older stencil clear");
    check(!bbg::fold_covers(stencil, depth), "covers: stencil alone leaves the older depth clear");

    // A pass's load op does a held clear only for the whole of layer 0, inside its render area.
    check(bbg::fold_by_load_op(0, 1920, 1080, 1920, 1080), "load op: the whole image at layer 0");
    check(!bbg::fold_by_load_op(1, 1920, 1080, 1920, 1080), "load op: another layer is recorded as before");
    check(!bbg::fold_by_load_op(0, 1920, 1080, 960, 540), "load op: a render area smaller than the image is recorded as before");
    check(!bbg::fold_by_load_op(0, 960, 540, 1920, 1080), "load op: an area larger than the image cannot be");
    if (!fail) std::printf("bbg_fold_clears_test: ok\n");
    return fail ? 1 : 0;
}
