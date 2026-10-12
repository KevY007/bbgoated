# Cloud task 4: the DLSS evaluation path from PR #73 - DLAA at the HDR anchor

Phase 2 plan, Task 4 (Steps 1-6 and the commits), with the controller's rulings in
`docs/cloud-tasks/task-4.md` (workspace) and the port map `docs/bbgoated/pr73-port.md` (rulings,
tables A-C, section E). Cloud run of 2026-10-12, session
https://claude.ai/code/session_01Evbd5sWhMNheA93g3mCtge; the commits end with the controller's
`Claude-Session` line, as README rule 5 asks.

## Hand-back

| Repo | Branch | From | Commits |
|---|---|---|---|
| KevY007/bbgoated | `cloud/task-4-dlss-eval` | `bbgoated/main` b4e1d97 | 10506c9 `bbg/upscale: DLSS evaluation path from PR #73 - DLAA at the HDR anchor, camera motion vectors, jitter`; b1279e6 `bbg/upscale: the game's anti-aliasing is held off while PR #73's DLAA runs, as with the fork's`; a3708d9 `bbg/upscale: DLAA stays the default reconstruction; [dlss] mode, [dlaa] enabled = false or BBHOST_DLAA=0 turn it off`; then this report |

- ctest at a3708d9 (Linux, clang 18, upstream's `tools/ci_build.sh`, `-DBBGOATED=ON`):
  `100% tests passed, 0 tests failed out of 63` (40 ran, 23 skipped: game files, or the game's libc
  for libc_native_test). `-DBBGOATED=OFF`: `100% tests passed, 0 tests failed out of 61` (38 ran,
  the same 23 skipped; bbgoated_config_test and redzone_audit_test are ON-only).
- Baseline, b4e1d97 with the same script: `100% tests passed, 0 tests failed out of 60`. New:
  display_settings_test, dlaa_motion_test (on lavapipe), ngx_transfer_sync_test.
- `check-online-untouched.ps1 -Branch cloud/task-4-dlss-eval` (PowerShell 7.4.6): `online paths
  untouched since 566184a (1 upstream cherry-picks allowed)`. `check-no-nvidia-binaries.ps1`
  (workspace main 96ee256, Task 3's): `no NVIDIA runtime, DLSS SDK files or FSR 4 model files
  tracked`.
- Nothing was pushed to `bbgoated/main` or `main`, no tag was touched, and the workspace has no
  commit from this task.

## What was done

1. Read the README, the brief, the plan (header and amendment, Global Constraints, Review Focus,
   Task 4), the port map, task 2's report and dry-run script, and report I sections 2-4 and 6.
2. Fetched upstream master (566184a) and PR #73 (`refs/pull/73/head` = 539d45f, unchanged since
   task 2). Built and tested the base, b4e1d97, as the baseline.
3. R1 step 1 (port map section E), on `cloud/task-4-dlss-eval` from b4e1d97:
   - `git diff ae0b56f~1 ae0b56f --output=<p> -- src/host/render.cpp src/host/gpu.cpp src/host/window.cpp`,
     `git apply -R --index <p>`: applied as it is (render.cpp 9 hunks, gpu.cpp 3, window.cpp 2);
   - `git diff 0717add~1 0717add --output=<t> -- src/main.cpp`, `git apply -R --index <t>`: applied;
   - cmake/bbgoated.cmake: the fork's dlaa.cpp block deleted by hand (6664c3f's dlaa_aa block stays).
4. Step 1: `git checkout 539d45f --` the 13 a2 files of table A; each blob checked equal to 539d45f's
   (`tools/dlaa_cbscan.py` is the same blob as the fork's, so git shows no change for it).
5. Step 5: the a2 hunks with the workspace's `scripts/dev/pr73_hunks.py` (566184a..539d45f) and
   `git apply --3way`. Every subset applied strictly (`git apply --check` passed first), with no
   unmerged file and `git diff --check` clean:

   ```
   src/host/render.cpp [1,2,21,22,24,25,26,28,29,30]: applied (strict)
   src/host/gpu.cpp [6,9,14]: applied (strict)
   src/host/window.cpp [24]: applied (strict)
   src/core/config.h [1]: applied (strict)
   src/core/config.cpp [1,2,3]: applied (strict)
   src/host/settings.h [1,2]: applied (strict)
   src/host/options.cpp [1,15]: applied (strict)
   src/main.cpp [1]: applied (strict)
   tests/config_test.cpp [1]: applied (strict)
   bbhost.example.toml [1]: applied (strict)
   ```

   Then table C's hand edits: render.cpp h25's object-motion params block out; gpu.cpp
   `host/dlaa.h` and `host/fsr_upscaler.h` after `core/portable.h`, and
   `dlaa_add_device_extensions(g.phys, dext);` after the `fifo_latest_ready_ext` block; window.cpp
   `host/dlaa.h` after `host/bindings.h`; config.cpp h2's `BBHOST_FG_BACKEND` block and h3's
   frame-generation and Reflex template lines out; bbhost.example.toml's three frame-generation lines
   out; options.cpp the plan's transitional block after the `BBHOST_RES` block.
6. Step 2: dlaa.cpp edits 1-4 (no `host/reflex.h`; the two frame-generation extension queries only
   when `fg_requested()`; `guides.reflex_frame_id = 0`; a log line when the
   `VK_EXT_buffer_device_address` strip removes it).
7. Steps 3-4: `src/host/fsr_upscaler_stub.cpp` and `cmake/bbgoated-upscale.cmake` as the plan writes
   them, plus `ngx_transfer_sync_test` (ruling 3), and the include line at the end of
   cmake/bbgoated.cmake. Built and tested: 63 of 63. Committed as 10506c9 with the UPSTREAM.md Ported
   row and the SETUP.md bridge sentence.
8. R1 step 3 and R2/R7, test first: E1-E10 on PR #73's dlaa.cpp/dlaa.h, the tick back in main.cpp
   (`git apply --3way <t>`: applied cleanly), the predicate as `reconstruction_running()`. 63 of 63;
   committed as b1279e6 with pr73-port.md's section E and R7 brought up to date.
9. R3/R5, test first: the DLAA seed in `config_load()`, the template and example as comments.
   63 of 63; committed as a3708d9 with pr73-port.md's R3 and table C, and SETUP.md's switch row.
10. Step 6: fresh builds of a3708d9 - `tools/ci_build.sh` with `-DBBGOATED=ON`, and `-DBBGOATED=OFF`
    with the same flags - and the whole suite each time; the two workspace guards; and, beyond the
    brief, a Windows-target syntax check of every changed translation unit (Test results).

## Files changed

- 10506c9 (26 files, +2608/-299): `src/host/dlaa.cpp`, `dlaa.h`, `display_settings.h`,
  `fsr_upscaler.h`, `object_motion.cpp/.h`, `motion_history.h`, `shaders/dlaa_mv.comp/.spv.h`,
  `docs/dlaa.md`, `tests/display_settings_test.cpp`, `tests/dlaa_motion_test.cpp` (PR #73's);
  `src/host/fsr_upscaler_stub.cpp`, `cmake/bbgoated-upscale.cmake` (new, bbgoated's);
  `src/host/render.cpp`, `gpu.cpp`, `main.cpp`, `options.cpp`, `settings.h`, `src/core/config.h`,
  `config.cpp`, `tests/config_test.cpp`, `bbhost.example.toml`, `cmake/bbgoated.cmake`;
  `docs/bbgoated/UPSTREAM.md`, `docs/bbgoated/SETUP.md`. (window.cpp is reverted and re-applied to
  the same bytes, so it is not in the commit.)
- b1279e6 (7 files, +134/-7): `src/host/dlaa.cpp`, `dlaa.h`, `dlaa_aa.cpp`, `dlaa_aa.h`,
  `src/main.cpp`, `tests/dlaa_aa_test.cpp`, `docs/bbgoated/pr73-port.md`.
- a3708d9 (5 files, +47/-8): `src/core/config.cpp`, `tests/config_test.cpp`, `bbhost.example.toml`,
  `docs/bbgoated/SETUP.md`, `docs/bbgoated/pr73-port.md`.
- This report: `docs/cloud-reports/task-4.md`.

Net effect against b4e1d97, as section E of the port map expects:

- window.cpp: byte for byte as before (no diff).
- gpu.cpp: +3 lines, `host/fsr_upscaler.h`, h9 (`shaderStorageImageExtendedFormats`) and h14
  (`fsr_retire_locked`); the fork's three DLAA lines are the same lines PR #73 has.
- render.cpp: +78/-1, PR #73's super-resolution HUD handling and the object-motion scope (the
  `VkRect2D sc` line moved up).
- main.cpp: h1's `display:` log line; the pump-loop tick as it was.
- dlaa.cpp and dlaa.h: `git diff 539d45f` shows only the plan's four edits and the marked
  `bbgoated:` blocks. Every other file of table A is 539d45f's blob (tools/ngx_bridge differs only
  by Task 3's provenance headers).

## Rulings

1. Ruling: three code commits - PR #73's path (R1 steps 1-2 with the plan's Steps 1-5), bbgoated's
   anti-aliasing hold on it (R1 step 3, R2, R7), DLAA as the default (R3, R5) - rather than the plan's
   single Step 10 commit. - The brief asks for one logical commit per step group, and each part
   reviews against a different reference (PR #73 plus the plan; .4's code; the rulings). Each commit
   builds and passes ctest (63 of 63). The cost: at 10506c9 DLAA is off by default and the hold is
   missing, at b1279e6 DLAA is off by default; only the head is meant to be built. - Cost if wrong:
   squash the three when merging.
2. Ruling: 10506c9's transitional block is the plan's text exactly (Off by default there), and R3's
   default comes in a3708d9. - The plan's block is the reference for Step 5; R3 is a separate
   decision. - Cost if wrong: none at the head.
3. Ruling: `ngx_transfer_sync_test` is registered in `cmake/bbgoated-upscale.cmake` (with
   `Vulkan::Vulkan Threads::Threads`, as PR #73's CMakeLists h6 and task 2's dry run), though the
   plan's Step 4 text has only two tests. - Table A, task 2's ruling 12 and Task 3's commit message
   ("ngx_transfer_sync_test joins it with the DLSS evaluation path") all say so. - Cost if wrong:
   six CMake lines.
4. Ruling: `.gitignore` h1 needs nothing: b4e1d97 (Task 3) merged it; the file is PR #73's plus
   bbgoated's five NVIDIA/FSR 4 lines. - Checked with `git diff 539d45f HEAD -- .gitignore`. - Cost
   if wrong: none.
5. Ruling: R3's seed sits in `config_load()` (config.cpp, right after the layers are merged), marked:
   with `config_value("dlss.mode")` empty, `c.dlss_mode` becomes "dlaa", or "off" when `[dlaa]
   enabled` is false, "0" or "off". The port map put it in the transitional block and, from Task 5,
   beside h11's seeds. - One edit instead of two: `config().dlss_mode` carries it to the
   transitional block now and to Task 5's `kDlssMode` seed (h11) and the setup window as PR #73
   wrote them, and config_test (which the brief names) can test it. pr73-port.md's R3 bullet and
   table C now say h11 needs no seed, so Task 5 does not add a second one. - Cost if wrong: move six
   lines into the transitional block and Task 5's h11.
6. Ruling: "unset" is exactly `config_value("dlss.mode").empty()`, so `mode = ""` counts as unset
   (DLAA), and `[dlaa] enabled` reads false for "false", "0" and "off" (the fork's and PR #73's own
   parse). - R3 defines unset that way. - Cost if wrong: one condition.
7. Ruling: the environment stays where the plan put it: `BBHOST_DLSS_MODE` (wins over the config) and
   `BBHOST_DLAA=0` (always Off) in the transitional block, every `rebuild_settings()`; Task 5 replaces
   them with PR #73's h8/h14. `BBHOST_DLAA=1` keeps PR #73's meaning (`config_on()` true whatever the
   mode), so `[dlaa] enabled = false` plus `BBHOST_DLAA=1` runs DLAA while the `display:` line says
   mode 0 - as in PR #73. - The plan's block is verbatim and options.cpp has no unit test. - Cost if
   wrong: a log line.
8. Ruling: R2's "failed for the session (set wherever PR #73's code turns DLAA off for the session)":
   besides `fail_locked` (E5), PR #73's NGX-init failure at the anchor (`g_s.failed = true;` alone)
   also stores `g_dlaa_off`. The port map said it needs none. - NGX was never up there, so
   `reconstruction_running()` was false anyway; nothing changes, but every "off for the session"
   is one signal, as the brief words it. - Cost if wrong: one line.
9. Ruling: E6's idle rule and E7's test-fail rule are `dlaa_idle()` and `dlaa_test_fail()` in
   host/dlaa_aa.h, called from the same places with the same expressions as .4's inline code. - So
   R2's logic is test-driven (dlaa_aa_test); .4 tested those conditions only in game. - Cost if
   wrong: inline them again.
10. Ruling: R7: `gpu::reconstruction_running()` is the predicate (DLAA's inputs only), with
    `dlaa_running()` kept as an alias although nothing else calls it - the port map and Tasks 5-6
    name it. `dlaa_aa_tick()` keeps its name and .4's log lines (DLAA is the only reconstruction in
    Task 4), and the pure `dlaa_active()` keeps R2's name. There is no new boolean logic for R7 in
    Task 4: the predicate's truth table is `dlaa_active()`'s, tested. - Cost if wrong: renames.
11. Ruling: the re-applied blocks keep .4's code and carry `bbgoated:` comments, as task 2's dry run
    wrote them (E2's comment also names the NGX-init case of ruling 8). - README rule 4. - Cost if
    wrong: comment text.
12. Ruling: SETUP.md changes with the code it describes: 10506c9 says the bridge must be version 2
    (the fork's v1 bridge is refused) and points at `docs/dlaa.md`'s "ngx_bridge" section for the
    build; a3708d9 lists the switches that turn DLAA off. `docs/dlaa.md` stays PR #73's verbatim (it
    already reads "on where the DLLs are", which R3 makes true). - A source-build player following
    the old sentence would get `dlaa: ngx_bridge.dll is not version 2; off`. - Cost if wrong: two
    table cells.
13. Ruling: the commit messages carry no measurement: the plan's `Measured: <...>` comes from Step 8,
    which is local, and a message never goes in with angle brackets (Global Constraints). - Cost if
    wrong: the controller records the numbers in `docs/perf/upscalers-2026-10.md` (Step 8) instead.
14. Ruling: commit trailers follow README rule 5 (the controller's `Claude-Session` line only, as on
    every earlier bbgoated commit), not this cloud session's default attribution lines. - The
    project's rule is the user's instruction. - Cost if wrong: trailers.
15. Ruling: to run the workspace guards here, the read-only workspace checkout was moved (detached)
    to origin/main 96ee256, which has Task 3's `check-no-nvidia-binaries.ps1`, and a symlink
    `src/bbgoated -> /home/user/bbgoated` was made under its gitignored `/src/`. PowerShell 7.4.6 came
    from its GitHub release (SHA-256 checked against the release's `hashes.sha256`), as in task 2.
    - The scripts look for the code repo at `<workspace>/src/bbgoated`. - Cost if wrong: none; nothing
    in either repo changed for them.
16. Ruling: the cloud machine also got `mesa-vulkan-drivers` (lavapipe: llvmpipe is its only Vulkan
    device, so dlaa_motion_test, gcn_sampling_test and indirect_clamp_test ran), `ccache`, and Ubuntu's
    mingw-w64 headers for the syntax check. - Cost if wrong: none.

## TDD evidence

R2 (the idle and test-fail rules, `tests/dlaa_aa_test.cpp`). RED, the cases before the functions:

```
$ cmake --build /home/user/ci-work/t4-on --target dlaa_aa_test
FAILED: CMakeFiles/dlaa_aa_test.dir/tests/dlaa_aa_test.cpp.o
../../bbgoated/tests/dlaa_aa_test.cpp:33:12: error: use of undeclared identifier 'dlaa_idle'
...
../../bbgoated/tests/dlaa_aa_test.cpp:39:12: error: use of undeclared identifier 'dlaa_test_fail'
(9 errors)
```

GREEN, with `dlaa_idle()` and `dlaa_test_fail()` in host/dlaa_aa.{h,cpp}:

```
$ cmake --build /home/user/ci-work/t4-on --target dlaa_aa_test && ./dlaa_aa_test
dlaa_aa_test: ok
```

Each new case catches a mutation of its rule (scratch copies, compiled with the test):

```
flip >= last_eval_flip + 600       -> FAIL: 600 flips since the last resolved frame: not idle yet
no last_eval_flip guard            -> FAIL: no frame resolved yet (starting, or never ran): not idle
no evaluations guard               -> FAIL: no frame resolved yet: waits, so the hold has something to release
flip > fail_flip                   -> FAIL: at its flip, with frames resolved: fails
no fail_flip guard                 -> FAIL: BBHOST_TEST_DLAA_FAIL unset: never
```

R3 and R5 (`tests/config_test.cpp`, a marked block after PR #73's object-motion checks, and the
template check). RED, the cases before the seed:

```
$ cmake --build /home/user/ci-work/t4-on --target config_test && ./config_test
FAIL ../../bbgoated/tests/config_test.cpp:108: load("").dlss_mode == "dlaa"
FAIL ../../bbgoated/tests/config_test.cpp:110: load("[dlss]\nmode = \"\"\n").dlss_mode == "dlaa"
FAIL ../../bbgoated/tests/config_test.cpp:111: load("[dlaa]\nenabled = true\n").dlss_mode == "dlaa"
FAIL ../../bbgoated/tests/config_test.cpp:121: c.dlss_mode == "dlaa"
FAIL ../../bbgoated/tests/config_test.cpp:209: !kv.count("dlss.mode")
FAIL ../../bbgoated/tests/config_test.cpp:210: read(t).find("# mode = \"off\"") != std::string::npos
6 failed
```

The other new cases (`[dlaa] enabled = false/"off"/0` -> off, a set mode wins over `[dlaa] enabled`
both ways, `quality` kept, `config_value("dlaa.preset") == "K"`) passed already against PR #73's
"off" default; they guard the seed. GREEN, with the seed and the comment-only template:

```
$ cmake --build /home/user/ci-work/t4-on --target config_test bbgoated_config_test && ./config_test && ./bbgoated_config_test
config_test: ok
bbgoated_config_test: ok
```

Mutations of the seed (scratch copies of config.cpp, built into config_test):

```
seed whatever the files say (no unset test) -> 4 FAIL, e.g. load("[dlss]\nmode = \"off\"\n").dlss_mode == "off"
always "dlaa" ([dlaa] enabled ignored)      -> 3 FAIL, e.g. load("[dlaa]\nenabled = false\n").dlss_mode == "off"
only "false" read as off                    -> 2 FAIL: enabled = "off", enabled = 0
template writes mode = "off" again          -> 2 FAIL: !kv.count("dlss.mode"), the comment check
```

R7: no new boolean logic in Task 4 (ruling 10); its truth table is `dlaa_active()`'s six cases,
which pass as before.

## Test results

| Run | Command | Result |
|---|---|---|
| b4e1d97 (base) | `CI_WORK=/home/user/ci-work CI_APT=0 CI_CMAKE_ARGS="-DBBGOATED=ON ..." bash tools/ci_build.sh` (worktree at b4e1d97) | `100% tests passed, 0 tests failed out of 60` |
| 10506c9 tree | the same flags, `cmake --build` + `ctest` | `100% tests passed, 0 tests failed out of 63` |
| b1279e6 tree | incremental | `100% tests passed, 0 tests failed out of 63` |
| a3708d9 tree | incremental | `100% tests passed, 0 tests failed out of 63` |
| a3708d9, fresh | `CI_WORK=/home/user/ci-work CI_APT=0 CI_CMAKE_ARGS="-DBBGOATED=ON" bash tools/ci_build.sh` | `100% tests passed, 0 tests failed out of 63`, `ci build: /home/user/ci-work/build/bbhost` |
| a3708d9, fresh, OFF | the same flags with `-DBBGOATED=OFF` | `100% tests passed, 0 tests failed out of 61` |

The Step 6 lines the plan expects:

```
display_settings_test: custom sizes, SR presets, native modes and overrides passed
ALL DLAA MOTION VECTOR TESTS PASSED          (llvmpipe; VK_DRIVER_FILES naming no driver gives the SKIP line instead)
All ngx_transfer_sync tests passed
config_test: ok                              (with PR #73's three object_motion checks and the R3/R5 cases; ON and OFF)
bbgoated_config_test: ok
dlaa_aa_test: ok                             (ON and OFF)
```

The 23 skipped (both builds): debug_menu_strings_test, libc_native_test, decomp_insn_eboot,
decomp_event_interpreter, decomp_player_data, decomp_item_lots, decomp_talk_evaluator,
decomp_chalice_ritual, decomp_follow_camera, decomp_sfx_ribbons, at9_test, dream_mirror_layout_test,
esd_test, rebirth_script_test, maiden_events_test, randomizer_files_test, gcn_decode_test,
translation_cache_test, gcn_bindings_test, gcn_translate_test, gcn_lift_test, gcn_tess_lift_test,
gcn_names_test - the same list as the baseline and task 2.

Warnings: the fresh ON build has 343, the baseline 343, the same count in every file (gpu.cpp 123,
render.cpp 94, ...); dlaa.cpp, object_motion.cpp, the stub, dlaa_aa.cpp, config.cpp and options.cpp
add none.

Windows-target syntax check (beyond the brief; upstream's `tools/win_syntax_check.sh` command with
Ubuntu's mingw-w64 libstdc++ 13 and the CI prefix's Vulkan/SDL3 headers, `-DBBHOST_BBGOATED=1`):
0 errors in dlaa.cpp, dlaa_aa.cpp, object_motion.cpp, fsr_upscaler_stub.cpp, gpu.cpp, render.cpp,
window.cpp, options.cpp, config.cpp and main.cpp at a3708d9 (and 0 in the same files at b4e1d97). A
scratch copy with an `#error` beside E3 reported it, so the `_WIN32`-only code (E3, `load_bridge`'s
`LoadLibraryExW`, `init_ngx_locked`) is compiled by this check. It is not the cross build: Step 7
still is.

Checks:

```
$ pwsh -NoProfile -File scripts/check-online-untouched.ps1 -Branch cloud/task-4-dlss-eval
online paths untouched since 566184a (1 upstream cherry-picks allowed)
$ pwsh -NoProfile -File scripts/check-no-nvidia-binaries.ps1
no NVIDIA runtime, DLSS SDK files or FSR 4 model files tracked
$ git diff --stat b4e1d97 HEAD -- src/net src/hle src/decomp src/engine
(nothing)
```

## What needs local verification (Task 4 Steps 7-9; not attempted here)

R3 changes two of the plan's expectations: the default run is the DLAA run, and an Off run needs
`BBHOST_DLSS_MODE=off` (or `BBHOST_DLAA=0`). Step 7, adapted:

```powershell
& $WS\scripts\build.ps1 -CMakeArgs "-DBBGOATED=ON" -Out $WS\out\bbg-a2 -Probe
& $WS\scripts\ngx-bridge.ps1 -Install -Pkg $WS\out\bbg-a2
& $WS\scripts\smoke.ps1 -Exe $WS\out\bbg-a2\bbhost.exe -Name a2-off -Env @{BBHOST_DLSS_MODE='off'}
& $WS\scripts\smoke.ps1 -Exe $WS\out\bbg-a2\bbhost.exe -Name a2-dlaa
Select-String -Path $WS\sandbox\a2-off\logs\run.log -Pattern "display: render", "temporal: NGX is not requested", "added for NGX"
Select-String -Path $WS\sandbox\a2-dlaa\logs\run.log -Pattern "display: render", "dlaa: ngx_bridge.dll loaded", "added for NGX", "VK_EXT_buffer_device_address", "NGX init", "anti-aliasing"
New-Item -ItemType Directory -Force $SP\v1pkg | Out-Null
Copy-Item -Recurse -Force $WS\out\bbg-a2\* $SP\v1pkg
Copy-Item -Force E:\Bloodborne\bbhost-dlaa-v0.2.18-dlaa.1\ngx_bridge.dll $SP\v1pkg\ngx_bridge.dll
& $WS\scripts\smoke.ps1 -Exe $SP\v1pkg\bbhost.exe -Name a2-v1
Select-String -Path $WS\sandbox\a2-v1\logs\run.log -Pattern "is not version 2", "anti-aliasing"
```

Expected:

- All smokes `smoke: ok`.
- a2-off: `display: render 1920x1080 -> output 1920x1080, DLSS mode 0, frame generation 1x` (the
  sandbox's size), `temporal: NGX is not requested for the selected providers`, no `added for NGX`.
- a2-dlaa (the default): `display: ... DLSS mode 1 ...`, `dlaa: ngx_bridge.dll loaded from ...`, the
  instance/device `... added for NGX` lines, then `dlaa: VK_EXT_buffer_device_address dropped: the
  core bufferDeviceAddress feature is on (VUID-VkDeviceCreateInfo-pNext-04748)`. `dlaa: NGX init ok`
  comes at the first anchor of a 3D scene (a smoke to the title may not draw one; Step 8's runs do),
  and after it, if the sandbox's anti-aliasing is on, `dlaa: the game's anti-aliasing is off while
  DLAA runs (...)`.
- a2-v1: `dlaa: ngx_bridge.dll is not version 2; off`, the title reached, and no anti-aliasing line
  (the hold never engages; the game's setting stays as it is).
- Also worth one smoke each (Review Focus 4):
  - a copy without `ngx_bridge.dll`: `dlaa: <dir>/ngx_bridge.dll not found or not loadable (error
    <n>); off`, and no anti-aliasing line;
  - `[dlaa] enabled = false` in the sandbox's `bbgoated.toml`, no `[dlss] mode`: `DLSS mode 0`, NGX
    not requested;
  - `BBHOST_TEST_DLAA_FAIL=600`, the anti-aliasing on, a run past flip 600 in an area: `dlaa:
    BBHOST_TEST_DLAA_FAIL - off for this session ...`, then `dlaa: the game's anti-aliasing is back
    to your setting (DLAA off)`;
  - Ctrl+F1 in a windowed run: `dlaa: off (Ctrl+F1)`, then the same "back to your setting" line.

  A non-NVIDIA GPU and 30 failed evaluations cannot be staged on the RTX 5080; their code paths are
  PR #73's.

Step 8, per the port map's R3 note on Review Focus 1 (with `-B`, `area-check.ps1` gives both builds
`-Env`; .4 reads `BBHOST_DLAA=0` as off, and so does a2):

```powershell
& $WS\scripts\area-check.ps1 -Pkg $WS\out\bbg-p1 -B $WS\out\bbg-a2 -Env @("BBHOST_DLAA=0") -Out $WS\sandbox\a2-off-ab -Areas "hunters-dream,central-yharnam,cathedral-ward,old-yharnam" -Rounds 2
& $WS\scripts\area-check.ps1 -Pkg $WS\out\bbg-p1 -B $WS\out\bbg-a2 -Out $WS\sandbox\a2-default-ab -Areas "hunters-dream,central-yharnam,cathedral-ward,old-yharnam" -Rounds 2
& $WS\scripts\area-check.ps1 -Pkg $WS\out\bbg-a2 -EnvA @("BBHOST_DLSS_MODE=off") -EnvB @("BBHOST_DLSS_MODE=dlaa") -Width 2560 -Height 1440 -Areas "central-yharnam,cathedral-ward,forbidden-woods,nightmare-of-mensis" -Rounds 2 -Out $WS\sandbox\a2-dlaa-1440
```

Expected: Off against Off within the noise floor with no frame differences (Review Focus 1); the
defaults (both DLAA) within noise too - PR #73's DLAA adds per-frame guide bookkeeping and a
third (1x1 fallback) binding to the motion-vector pass over the fork's; the 1440p A/B gives DLAA's
cost per place, with `dlaa: NGX init ok`, `dlaa: ready at 2560x1440, scene format 97, output copied
back, preset 0`, `ViewProj check ok`, `dlaa: 300 frames evaluated` and no `evaluate failed`.

Step 9 (validation pass with DLAA on) as the plan writes it; `BBHOST_DLSS_MODE=dlaa` is the default
now but harmless to keep. `VUID-VkDeviceCreateInfo-pNext-04748` would mean the BDA strip failed; a
`StorageImageExtendedFormats` message would mean gpu.cpp h9 is missing (it is in).

## Open concerns

- The Off path is not quite .4's with DLAA off. With mode Off, PR #73's anchor still finds the
  scene, depth and camera every frame before it returns (`if (!config_on() && !fg_requested())`):
  CPU only - the camera-signature scan of main-depth draws' vertex constant buffers until one
  matches, an 864-byte copy, a `dlaa: scene ...` line when the scene changes, `dlaa: camera found`
  once. .4's fork code stopped after its first anchor (`g_s.failed` once the bridge was not wanted).
  No GPU work, no jitter, no extension either way. If the Off-vs-Off A/B shows it, a marked early
  return at the top of `dlaa_anchor_locked` when neither DLAA nor frame generation is wanted is the
  fix.
- gpu.cpp h9 turns on the core feature `shaderStorageImageExtendedFormats` on every device that has
  it, on the default path too: a feature, not an extension (Review Focus 1's "no added extensions"
  holds), needed by the motion-vector pass's RG16F storage image. The A/B checks it costs nothing.
- With a v2 bridge present on a non-NVIDIA GPU, NGX's required instance extensions are still added
  (the bridge loads before the vendor check at device creation), as in .4.
- The transitional block's environment handling (`BBHOST_DLSS_MODE`, `BBHOST_DLAA=0`) is the plan's
  text and has no unit test (options.cpp has none); Task 5 replaces it with PR #73's h8/h14.
- pr73-port.md now says where R3's seed lives (config_load, ruling 5) and that h11 needs none; the
  plan's own text (workspace) is not edited and still describes the seed in options.cpp and an Off
  default for Step 8.
- `docs/dlaa.md` (PR #73's, verbatim) still asks the player to turn the game's anti-aliasing off;
  bbgoated holds it off itself (SETUP.md says so).
