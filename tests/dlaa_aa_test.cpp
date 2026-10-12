// host/dlaa_aa.cpp (bbgoated): the game's anti-aliasing is held off while DLAA
// runs - the options file keeping the player's choice - and given back when
// DLAA is turned off, fails, or stops resolving frames.
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
    check(dlaa_aa_step(true, true, false) == kDlaaAaHold, "DLAA runs, the game's AA on: held off");
    check(dlaa_aa_step(true, true, true) == kDlaaAaHold, "turned on again from a menu while DLAA runs: held off again");
    check(dlaa_aa_step(true, false, true) == kDlaaAaKeep, "DLAA runs, AA held off: nothing to do");
    check(dlaa_aa_step(true, false, false) == kDlaaAaKeep, "DLAA runs, the player has AA off: nothing to do");
    check(dlaa_aa_step(false, false, true) == kDlaaAaRelease, "DLAA turned off: AA back to the player's setting");
    check(dlaa_aa_step(false, true, false) == kDlaaAaKeep, "DLAA off, AA on: nothing to do");
    check(dlaa_aa_step(false, false, false) == kDlaaAaKeep, "DLAA off, the player has AA off: it stays off");

    // dlaa_active(configured, ngx_ok, failed, idle, enabled)
    check(dlaa_active(true, true, false, false, true), "configured, NGX up, resolving, on: DLAA runs");
    check(!dlaa_active(false, true, false, false, true), "dlaa.enabled = false: not running");
    check(!dlaa_active(true, false, false, false, true), "NGX not up (yet, or failed to start): not running");
    check(!dlaa_active(true, true, true, false, true), "failed for the session after NGX started: not running");
    check(!dlaa_active(true, true, false, true, true), "ten seconds of 3D frames without an evaluation: not running");
    check(!dlaa_active(true, true, false, false, false), "switched off with Ctrl+F1: not running");
    if (!fail) std::printf("dlaa_aa_test: ok\n");
    return fail ? 1 : 0;
}
