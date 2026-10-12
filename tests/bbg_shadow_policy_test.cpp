// host/perf/core/shadow_policy_core.h: the buffer shadow's alternative hot
// rules (bbgoated, after bbport's memory module).
#include "host/perf/core/shadow_policy_core.h"

#include <cstdio>

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    bbg::PagePolicy s;
    check(!bbg::frames_rule_copy(s, 100), "one copy: stays");
    check(!bbg::frames_rule_copy(s, 100), "a second copy in the same flip counts once");
    check(!bbg::frames_rule_copy(s, 130), "two flips in 60: stays");
    check(bbg::frames_rule_copy(s, 159), "three flips in 60: host memory");
    bbg::PagePolicy t;
    bbg::frames_rule_copy(t, 10);
    bbg::frames_rule_copy(t, 40);
    check(!bbg::frames_rule_copy(t, 75), "the copy at flip 10 left the 60-flip window");
    bbg::PagePolicy p;
    check(!bbg::pingpong_copy(p, 1000, 2), "a copy with no GPU write is no bounce");
    p.gpu_since_copy = true;
    check(!bbg::pingpong_copy(p, 2000, 2), "first bounce");
    p.gpu_since_copy = true;
    check(bbg::pingpong_copy(p, 3000, 2), "second bounce within 8 s: host memory");
    bbg::PagePolicy q;
    q.gpu_since_copy = true;
    bbg::pingpong_copy(q, 1000, 2);
    q.gpu_since_copy = true;
    check(!bbg::pingpong_copy(q, 10000, 2), "bounces 9 s apart do not add up");
    bbg::PagePolicy r;
    r.dynamic_ms = 5000;
    r.dynamic_rewritten = true;
    check(!bbg::retry_due(r, 9000, 10000), "retry not before its time");
    check(bbg::retry_due(r, 15000, 10000), "retry due");
    r.dynamic_rewritten = false;
    check(!bbg::retry_due(r, 99000, 10000), "only pages sent away for being rewritten come back");
    if (!fail) std::printf("bbg_shadow_policy_test: ok\n");
    return fail ? 1 : 0;
}
