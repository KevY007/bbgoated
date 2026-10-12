#pragma once
// bbgoated: DLAA and the game's own anti-aliasing. DLAA resolves the edges
// itself, and the game's post-tonemap AA on top only blurs what it resolved,
// so while DLAA runs the game's AA is held off. When DLAA is turned off
// (Ctrl+F1) the game's AA comes back - if the player had it on, including when
// they turned it on in the menu while DLAA held it off.

enum DlaaAaAction { kDlaaAaKeep = -1, kDlaaAaTurnOff = 0, kDlaaAaTurnOn = 1 };

struct DlaaAaState {
    bool held = false;            // DLAA runs and holds the game's AA off
    bool restore = false;         // the player wants it on once DLAA stops
};

// Called with the current DLAA and AA states; says what to do with the game's AA.
DlaaAaAction dlaa_aa_step(DlaaAaState* s, bool dlaa_active, bool game_aa_on);
