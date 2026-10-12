// See dlaa_aa.h.
#include "host/dlaa_aa.h"

DlaaAaAction dlaa_aa_step(bool dlaa_active, bool game_aa_on, bool held) {
    if (dlaa_active) return game_aa_on ? kDlaaAaHold : kDlaaAaKeep;
    return held ? kDlaaAaRelease : kDlaaAaKeep;
}
