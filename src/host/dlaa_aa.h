#pragma once
// bbgoated: DLAA and the game's own anti-aliasing. DLAA resolves the edges
// itself, and the game's post-tonemap AA on top only blurs what it resolved,
// so while DLAA runs the game's AA is held off - live only: the options file
// keeps the player's choice (host_opt_hold), so it is never lost, even when
// bbgoated ends with DLAA on. When DLAA is turned off (Ctrl+F1) the game's AA
// goes back to that choice (host_opt_release). A change the player makes to
// the setting while DLAA runs is their new choice; if it is on, the live
// value is held off again.

#include <cstdint>

enum DlaaAaAction { kDlaaAaKeep, kDlaaAaHold, kDlaaAaRelease };

// What to do with the game's AA, from whether DLAA runs, whether the game's
// AA is on now, and whether it is held off now.
DlaaAaAction dlaa_aa_step(bool dlaa_active, bool game_aa_on, bool held);

// Whether DLAA resolves the frames now, so the game's AA is held off: it is
// configured (dlaa.enabled), NGX is up, it has not failed for the session,
// it is not idle (ten seconds of 3D frames without an evaluation: no anchor,
// or none with its scene and depth) and Ctrl+F1 has not switched it off.
bool dlaa_active(bool configured, bool ngx_ok, bool failed, bool idle, bool enabled);

// Whether DLAA is idle at a flip with 3D frames: more than 600 flips (ten
// seconds) since last_eval_flip, the flip of the last frame it resolved. Never
// before its first, so a DLAA that is still starting is not idle.
bool dlaa_idle(std::uint64_t last_eval_flip, std::uint64_t flip);

// BBHOST_TEST_DLAA_FAIL=<flip> (fail_flip, 0 when unset): DLAA fails for the
// session at that flip, or the first anchor after it, once it has resolved
// frames - so the hold has something to give back when it does.
bool dlaa_test_fail(std::uint64_t fail_flip, std::uint64_t flip, std::uint64_t evaluations);
