// See dlaa_aa.h.
#include "host/dlaa_aa.h"

DlaaAaAction dlaa_aa_step(DlaaAaState* s, bool dlaa_active, bool game_aa_on) {
    if (dlaa_active) {
        s->held = true;
        if (!game_aa_on) return kDlaaAaKeep;
        s->restore = true;  // on now: the player wants it once DLAA stops
        return kDlaaAaTurnOff;
    }
    if (!s->held) return kDlaaAaKeep;
    s->held = false;
    if (!s->restore) return kDlaaAaKeep;
    s->restore = false;
    return kDlaaAaTurnOn;
}
