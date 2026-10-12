// host/perf/core/frame_pacing_core.h: which flip's GPU work the command
// processor waits for at FRAMES_AHEAD=k (bbgoated).
#include "host/perf/core/frame_pacing_core.h"

#include <cstdio>

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    bbg::FrameNeeds n;
    check(n.push(10, 1) == 0, "the first flip has nothing before it");
    check(n.push(21, 1) == 10, "k=1: flip N waits for flip N-1's work");
    check(n.push(33, 1) == 21, "k=1 again");
    // Flip N-1's need is flip N's own when nothing was submitted between them:
    // it may be the recording still open, which only the waiting thread would
    // hand over (BBHOST_SUBMIT_EVERY_JOB=0), so it is not waited for.
    check(n.push(33, 1) == 0, "handed_over_only: no submission since flip N-1");
    check(n.push(40, 1) == 33, "and the next flip waits for it again");
    bbg::FrameNeeds m;
    m.push(5, 2);
    m.push(9, 2);
    check(m.push(14, 2) == 5, "k=2: flip N waits for flip N-2's work");
    check(m.push(20, 0) == 0, "k=0: off");
    check(m.push(20, 9) == 0, "k past the ring: off");
    if (!fail) std::printf("bbg_frame_pacing_test: ok\n");
    return fail ? 1 : 0;
}
