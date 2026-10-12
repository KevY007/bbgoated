// host/perf/core/async_skip_core.h: when a draw may be skipped while its
// pipeline compiles (bbgoated, bbport's rules).
#include "host/perf/core/async_skip_core.h"

#include <cstdio>

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    bbg::PassHistory h;
    for (std::uint64_t f = 100; f <= 108; ++f) h.note(7, f);
    check(h.every_frame(7, 108, 8), "pass_every_frame: drawn in each of the last 8 flips");
    h.note(9, 100);
    h.note(9, 108);
    check(!h.every_frame(9, 108, 8), "a gap: not every frame");
    check(!h.every_frame(7, 109, 8), "not yet drawn this flip");
    check(!h.every_frame(11, 108, 8), "a pass never drawn");
    for (std::uint64_t p = 1000; p < 6000; ++p) h.note(p, 108);
    check(h.every_frame(7, 108, 8), "past 4096 passes the recent ones are kept");
    bbg::SkipDraw d;
    d.count = 36;
    check(bbg::skip_eligible(d, 900, false, true), "a mesh in a pass drawn every frame may be skipped");
    check(!bbg::skip_eligible(d, 900, false, false), "a pass not drawn every frame waits");
    d.occlusion = true;
    check(!bbg::skip_eligible(d, 900, false, true), "occlusion_never");
    d.occlusion = false;
    d.count = 3;
    check(!bbg::skip_eligible(d, 900, false, true), "fullscreen_never");
    d.count = 12;
    check(!bbg::skip_eligible(d, 900, false, true), "fullscreen_never: 12 vertices still wait");
    d.count = 36;
    check(!bbg::skip_eligible(d, 200, false, true), "a frame under 300 draws waits");
    check(!bbg::skip_eligible(d, 900, true, true), "a loading screen waits");
    d.indirect = true;
    check(!bbg::skip_eligible(d, 900, false, true), "indirect draws wait");
    d.indirect = false;
    d.tessellated = true;
    check(!bbg::skip_eligible(d, 900, false, true), "tessellated draws wait");
    d.tessellated = false;
    d.stores = true;
    check(!bbg::skip_eligible(d, 900, false, true), "stores wait");
    bbg::SkipBudget b;
    check(b.allow(42, 1000, 45), "budget: the first flip");
    check(b.allow(42, 1045, 45), "budget: 45 flips on");
    check(!b.allow(42, 1046, 45), "budget_expires");
    check(b.allow(43, 1046, 45), "budget: each pipeline its own");
    if (!fail) std::printf("bbg_async_skip_test: ok\n");
    return fail ? 1 : 0;
}
