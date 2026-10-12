// host/dlaa_aa.cpp (bbgoated): the game's anti-aliasing goes off while DLAA runs
// and comes back when DLAA is turned off - if the player had it on.
#include "host/dlaa_aa.h"

#include <cstdio>

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    {
        DlaaAaState s;
        check(dlaa_aa_step(&s, false, true) == kDlaaAaKeep, "DLAA off, AA on: nothing to do");
        check(dlaa_aa_step(&s, true, true) == kDlaaAaTurnOff, "DLAA turns on: the game's AA goes off");
        check(dlaa_aa_step(&s, true, false) == kDlaaAaKeep, "DLAA on, AA off: nothing to do");
        check(dlaa_aa_step(&s, true, true) == kDlaaAaTurnOff, "AA turned back on while DLAA runs: off again");
        check(dlaa_aa_step(&s, false, false) == kDlaaAaTurnOn, "DLAA off: the player's AA comes back");
        check(dlaa_aa_step(&s, false, true) == kDlaaAaKeep, "and stays as the player has it");
    }
    {
        DlaaAaState s;
        check(dlaa_aa_step(&s, true, false) == kDlaaAaKeep, "AA was off already when DLAA started");
        check(dlaa_aa_step(&s, false, false) == kDlaaAaKeep, "DLAA off: AA stays off (the player had it off)");
    }
    if (!fail) std::printf("dlaa_aa_test: ok\n");
    return fail ? 1 : 0;
}
