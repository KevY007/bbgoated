// See dlaa_aa.h.
#include "host/dlaa_aa.h"

DlaaAaAction dlaa_aa_step(bool dlaa_active, bool game_aa_on, bool held) {
    if (dlaa_active) return game_aa_on ? kDlaaAaHold : kDlaaAaKeep;
    return held ? kDlaaAaRelease : kDlaaAaKeep;
}

bool dlaa_active(bool configured, bool ngx_ok, bool failed, bool idle, bool enabled) {
    return configured && ngx_ok && !failed && !idle && enabled;
}

bool dlaa_idle(std::uint64_t last_eval_flip, std::uint64_t flip) {
    return last_eval_flip && flip > last_eval_flip + 600;
}

bool dlaa_test_fail(std::uint64_t fail_flip, std::uint64_t flip, std::uint64_t evaluations) {
    return fail_flip && flip >= fail_flip && evaluations;
}
