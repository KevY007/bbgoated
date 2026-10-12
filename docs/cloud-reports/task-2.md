# Cloud task 2: the PR #73 port map, the hunk tool and the conflict ledger

Phase 2 plan, Task 2, with the controller's rulings R1-R9 (`docs/cloud-tasks/task-2.md` in the
workspace). Cloud run of 2026-10-12, session https://claude.ai/code/session_011TAMzL4n6DtUftdoptrxwa;
the commits end with the controller's `Claude-Session` line, as README rule 5 asks.

## Hand-back

| Repo | Branch | From | Commits |
|---|---|---|---|
| KevY007/bbgoated | `cloud/task-2-port-map` | `cloud/task-R-rebase` ebb97dc (9cc8dc0 + task R's report) | 48f5145 `bbg/docs: PR #73 port map - what is taken, left out and resolved`; then this report with `task-2-dryrun.py` |
| KevY007/bbgoated-workspace | `cloud/task-2-port-map` | `main` 07bd5c3 | 2da0e18 `pr73_hunks.py: PR #73's hunks of one file as a patch for git apply --3way, with its test` |

- Code repo ctest (Linux, upstream's `tools/ci_build.sh`, `-DBBGOATED=ON`) on 48f5145:
  `100% tests passed, 0 tests failed out of 60` (37 ran, 23 skipped: they need the game files or
  the game's libc). The code repo change is documentation only.
- Workspace tests: `test_pr73_hunks.py` 11 OK; all four `scripts/tests/test_*.py` files OK (28 tests).
- `check-online-untouched.ps1` (task R's version, PowerShell 7.4.6): `online paths untouched since
  566184a (1 upstream cherry-picks allowed)`. No NVIDIA runtime, DLSS SDK or FSR 4 model file is tracked.

## What was done

1. Read the brief, the plan (header and amendment, Global Constraints, Review Focus, Task 2 in
   full, Tasks 3-7) and reports I and D.
2. Fetched upstream (`upstream/*`), PR #73 (`upstream-pr/73` = 539d45f), the head report I reviewed
   (3bfcb65, fetched from FIocker/bbhost by its SHA) and the DLAA fork (`dlaa` = 96cd17d), to see
   what FIocker's rebase changed and what the fork's code on bbgoated is.
3. The hunk tool, test first: `scripts/dev/pr73_hunks.py`, `scripts/tests/test_pr73_hunks.py`
   (workspace). Defaults 566184a / 539d45f (R9).
4. Every hunk of PR #73 classified: 59 added files, 32 modified files with 173 hunks, one rename
   (appendix A has the list). Table A/B of `docs/bbgoated/pr73-port.md`, per commit (a1)-(d), with
   frame generation (85d81a9's rows included), Reflex and the presenter/frame-loop hunks left out
   and a3a7fa7's FSR retirement patch taken with FSR.
5. R1: the fork's DLAA hunks and bbgoated's DLAA edits on the rebased branch, located per commit
   and hunk; the revert, port and re-apply sequence, with each re-applied edit's landing spot in
   PR #73's `dlaa.cpp` (section E of the map).
6. Waited for `cloud/task-R-rebase`: it was not there at the start (checks at 11:41 and 11:47 UTC)
   and was at the next check, about 11:55 UTC (9cc8dc0); task R added its report on top later
   (ebb97dc), which changes no code. The dry runs ran against 9cc8dc0 and again, with the committed
   script, against this branch: the same results. Near the end of the run `bbgoated/main` itself
   moved to dde2fa6 (task R's branch plus four area-check commits: `tools/area_check.py`,
   `src/hle/system.cpp`, `video.cpp`, `modules.h` and their tests); this branch merges into it
   without a conflict (`git merge-tree`), and both dry runs print the same lines against it.
7. Dry runs: each subset alone (throwaway index), the whole sequence in a scratch worktree, and the
   trees after (a2) and after (d) built and tested on Linux.
8. Docs: `docs/bbgoated/pr73-port.md`, UPSTREAM.md's "Ported (not cherry-picked)" table, this
   report and `docs/cloud-reports/task-2-dryrun.py`.

## Files changed

- Code repo: `docs/bbgoated/pr73-port.md` (new), `docs/bbgoated/UPSTREAM.md` (the "Ported (not
  cherry-picked)" table after "Taken early", and a line on the DLAA fork's picks giving way),
  `docs/cloud-reports/task-2.md` (this report), `docs/cloud-reports/task-2-dryrun.py` (the dry runs).
- Workspace: `scripts/dev/pr73_hunks.py`, `scripts/tests/test_pr73_hunks.py` (both new).

## What the map says, in short

- PR #73's added files are byte for byte the reviewed 3bfcb65's except `cmake/fsr.cmake` (+18:
  applies the new `fsr-frame-retirement.patch`), `display_settings.h` (85d81a9 removed
  `experimental_mfg_enabled`) and files that are not taken. Line numbers quoted against 3bfcb65's
  `dlaa.cpp` in report I and the plan still hold.
- The modified files have 3bfcb65's hunk counts except `options.cpp` (17 -> 16; old h12-h17 are
  h11-h16). `src/net/session.cpp` and `src/hle/video.cpp` are no longer touched (master's 59da4a6
  made the same change).
- The plan's hand resolutions C1-C5 are gone: FIocker resolved `translate.h` and `render.cpp`
  h16 against next-steps as the plan proposed, and the other three hunks' contexts now hold
  next-steps' lines. Every (b), (c) and (d) subset applies strictly to the rebased bbgoated/main.
- What collides is the DLAA fork's code: render.cpp, gpu.cpp and window.cpp's (a2) subsets until R1's
  revert (then they apply strictly), and .gitignore h1, which merges with `--3way` into exactly PR #73's
  file. After revert, port and re-apply, window.cpp is byte for byte unchanged, gpu.cpp gains three
  lines, and every line bbgoated added to the fork's dlaa.cpp/dlaa.h is in PR #73's files.
- With the plan's Task 4 scaffolding (stub, `cmake/bbgoated-upscale.cmake`), the tree after (a2)
  builds and passes ctest 63/63; after (b), (c) and (d), with the Task 5/7 test registrations, 66/66.
  The new tests ran: `display_settings_test`, `dlaa_motion_test` (on lavapipe), `ngx_transfer_sync_test`,
  `presentation_coordinates_test`, `motion_history_test`, `object_motion_shader_test`.

## Rulings

1. Ruling: R2, R3, R5 and R7 are the four clauses of the brief's "R2/R3/R5/R7" line, in that order
   (R2 the edits survive, R3 the DLAA default, R5 `[dlaa] enabled`/preset, R7 the AA hold for every
   reconstruction). - The brief lists four rulings and four clauses without labels. - Cost if
   wrong: relabel four bullets in pr73-port.md.
2. Ruling: the tool goes beyond the plan's listing: it spells out git's diff defaults (`-U3`,
   `--inter-hunk-context=0`, myers, indent heuristic, a/ b/ prefixes, no colour, no external diff, no
   textconv, no renames, full blob ids, `diff.suppressBlankEmpty=false`), moves each chosen hunk's
   new-side start by the left-out hunks' line deltas, writes the file's bytes (CRLF and non-UTF-8 kept;
   the plan's version decoded as UTF-8) and refuses reversed ranges, hunk 0, empty selections and
   missing refs. Its interface and `--list` format are the plan's; `--repo` still defaults to
   `<workspace>/src/bbgoated`. - A user's `diff.noprefix`, `diff.context`, `color.ui` or `diff.external`
   would renumber the hunks or break the patch (the RED run shows it), and git apply searches from the
   new-side start first. - Cost if wrong: none to the map; drop the extra flags.
3. Ruling: .gitignore is not in step 1's revert. - h1 with `--3way` merges into PR #73's file
   exactly (checked: blob 8a35fa4 both ways); the brief's list names the five files that need it. -
   Cost if wrong: one more `git apply -R` line.
4. Ruling: in cmake/bbgoated.cmake only the fork's dlaa.cpp block is reverted (by hand: 6664c3f's
   block follows it, so `git apply -R` does not match); 6664c3f's dlaa_aa block stays where it is, and
   dlaa.cpp is registered from (a2) on by `cmake/bbgoated-upscale.cmake` with the plan's `IN_LIST`
   guard. - dlaa_aa.* is bbgoated's own and conflicts with nothing. - Cost if wrong: move six lines.
5. Ruling: main.cpp's pump-loop tick (0717add) is reverted in step 1 and put back unchanged in step 3,
   as the brief's list names main.cpp; it never conflicts with PR #73's h1 either way. - Cost if
   wrong: none (the same lines end where they were).
6. Ruling: the re-applied DLAA edits keep their code byte for byte and carry `// bbgoated:` in their
   comments; the dlaa.h comment on `dlaa_running()` now also says "not failed or idle", which 13fd88a
   added to its logic without updating the comment. 6664c3f's BDA strip is not re-applied: PR #73
   strips the extension itself (`dlaa.cpp:933-938`) and the plan's edit 4 logs it. - README rule 4
   asks for marked edits in upstream code. - Cost if wrong: comment text.
7. Ruling: R3 has a consequence the plan did not have: PR #73 refuses live resolution changes with any
   reconstruction mode, DLAA included (`res_live()` h6, `apply(kResolution)` h7), so with DLAA the
   default every player would lose them, on any GPU (the mode is DLAA in the settings even where
   `dlaa.cpp` turns itself off). The map tells Task 5 to narrow both checks to the modes that change
   the render size, DLSS super resolution (`dlss_mode >= 2`) and FSR (`upscaler_backend != 0`), as a
   marked edit. - DLAA recreates its feature when the scene size changes (`ensure_locked`), as the
   fork's did, and bbgoated .4 has live resolution with DLAA on. - Cost if wrong: Task 5 keeps PR
   #73's checks, and resolution changes need a restart for everyone.
8. Ruling: R3 also needs the first-start template (`config.cpp` h3) and `bbhost.example.toml` (h1) to
   carry `mode = "off"` as a comment, and the "unset" test to use `config_value("dlss.mode")` (empty
   when no config layer sets it). - With R5's "[dlss] mode wins", a template that writes `mode = "off"`
   would turn DLAA off for every new player. - Cost if wrong: two template lines.
9. Ruling: R7's design for Tasks 5-6: `dlaa_running()`'s "provider started" input becomes NGX for
   DLSS and the FSR provider for FSR, `g_sr_render_failed` with FSR selected counts as failed (DLSS
   falls back to native DLAA), and the idle clock is fed where a frame was reconstructed (DLAA's
   evaluate at the anchor; `dlss_sr_upscale_hud_locked` after a successful SR or FSR evaluation).
   - In super-resolution modes PR #73's anchor records guides only, so feeding the clock there would
   hold the game's AA off over a failed FSR. - Cost if wrong: a design note; nothing is implemented.
10. Ruling: a3a7fa7 goes with (c) as `cmake/fsr.cmake` (verbatim, 539d45f) plus
    `cmake/fsr-frame-retirement.patch`; its `frame_generation.cpp` and `fsr_provider_check.cpp` parts
    are left out with those files. The patch changes FireBurn's FG module and adds a diagnostic count
    to the 3.1.5 bridge, so for upscaling it is inert. - The controller's ruling, and fsr.cmake refuses
    a provider without the patch applied. - Cost if wrong: none.
11. Ruling: 539d45f's `docs/upstream-rebase-validation-2026-10-12.md` and its CSV are left out with
    the other validation records. - They record FG and Reflex pacing runs on FIocker's machine. -
    Cost if wrong: two doc files.
12. Ruling: `ngx_transfer_sync_test` (a mock of NGX's barriers, no GPU) is registered with (a1) in
    `cmake/bbgoated-upscale.cmake`, under `if(TARGET config_test)`; PR #73's CMakeLists h6 registers
    it for every build. - It builds and passes on Linux (checked). - Cost if wrong: four CMake lines.
13. Ruling: the dry runs built with the plan's Task 4 Steps 3-4 files (fsr_upscaler_stub.cpp,
    `cmake/bbgoated-upscale.cmake` and its include line) and Task 5/7 test registrations, in the scratch
    worktree only. Nothing of it is committed: those files are Tasks 4-7's. - Without them the trees
    do not link. - Cost if wrong: none.
14. Ruling: section E's commands write patches with `git diff <c>~1 <c> --output=<patch>` rather than
    `git show | git apply`. - A PowerShell 5.1 pipe re-encodes text (the plan's own warning about `>`).
    - Cost if wrong: none; both forms produce the same patch (checked).
15. Ruling: the workspace branch was rebased onto main as main moved during the run (6bb335e: Phase 2
    Task 1 tooling and the P3A/P3B briefs; then 07bd5c3: task R's check-script commits and the
    vvl-summary update) and force-pushed with a lease on my own commit; the code branch was moved onto
    task R's ebb97dc before its first commit. - Both are my branches; the workspace one fast-forwards
    main. - Cost if wrong: none.
16. Ruling: the cloud machine got `mesa-vulkan-drivers` (lavapipe) besides upstream's CI package list,
    so the Vulkan unit tests ran on the CPU (`dlaa_motion_test`, `gcn_sampling_test`,
    `indirect_clamp_test`); task R's machine skipped 25 tests, this one 23. PowerShell 7.4.6 was
    downloaded from PowerShell's GitHub release (SHA-256 checked against its `hashes.sha256`) to run
    the online check. - The plan expects `dlaa_motion_test` on lavapipe, not skipped. - Cost if wrong:
    none; nothing in either repo changed for them.
17. Ruling: the code branch stays on `cloud/task-R-rebase` (ebb97dc), as the brief says, not on the
    newer `bbgoated/main` dde2fa6. - It merges cleanly, and the dry runs print the same lines against
    dde2fa6: its four new commits touch no file of the port's subsets (PR #73's `system.cpp` hunks
    are Reflex, left out). - Cost if wrong: a rebase of two docs commits.

## TDD evidence (the hunk tool)

RED 1, the test before the tool existed:

```
$ python3 -I scripts/tests/test_pr73_hunks.py
FileNotFoundError: [Errno 2] No such file or directory: '/home/user/bbgoated-workspace/scripts/dev/pr73_hunks.py'
exit=1
```

RED 2, the plan's Task 2 Step 1 tool (verbatim but for the two default SHAs) under the new test:

```
$ python3 -I scripts/tests/test_pr73_hunks.py
ERROR: test_an_empty_selection_is_refused (__main__.RepoTest...)
ERROR: test_crlf_and_non_utf8_bytes_reach_the_patch (__main__.RepoTest...)
ERROR: test_list_numbers_hunks_as_plain_git_diff_does (__main__.RepoTest...)
ERROR: test_patch_applies_three_way_without_the_left_out_hunk (__main__.RepoTest...)
    subprocess.CalledProcessError: Command '['git', '-C', '/tmp/.../repo', 'diff', ...]' returned non-zero exit status 128.
    (the scratch repo's diff.external; run by hand, git says "fatal: external diff died, stopping at <file>")
FAIL: test_bad_ranges_are_errors - AssertionError: ValueError not raised : 5-3
FAIL: test_new_side_start_follows_the_hunks_left_out
    AssertionError: '\n@@ -20,2 +20,3 @@\n p\n+q\n r\n' not found in '...@@ -20,2 +21,3 @@\n p\n+q\n r\n'
Ran 11 tests in 0.272s
FAILED (failures=2, errors=4)
```

GREEN:

```
$ python3 -I scripts/tests/test_pr73_hunks.py -v
... 11 tests ... ok
Ran 11 tests in 0.200s
OK
```

Each pinned diff option is covered (a copy of the tool with one option removed, the same tests):

```
without "--no-color"                         -> FAILED (failures=3)
without "--no-ext-diff"                      -> FAILED (failures=3)
without "-U3"                                -> FAILED (failures=1, errors=1)
without "--src-prefix=a/", "--dst-prefix=b/" -> FAILED (failures=1, errors=1)
without "--full-index"                       -> OK   (robustness only: abbreviated ids still resolve)
without the new-side renumbering             -> FAILED (failures=2)
```

On the code repo, `--list` gives the plan's expected lines:

```
$ python3 -I scripts/dev/pr73_hunks.py --repo /home/user/bbgoated --file src/host/window.cpp --list | sed -n '1p;$p'
h1   @@ -10,10 +10,14  +4 -0  +#include "host/dlaa.h"
h35  @@ -2351,20 +2797,24  +12 -8  +        PresentFlip captured_flip = flip;
$ ... --file src/net/session.cpp --list        -> "src/net/session.cpp: no change between 566184a and 539d45f", exit 1
$ ... --file src/host/window.cpp --hunks 36 ... -> "error: src/host/window.cpp: no hunk [36] (the diff has 35)", exit 2
```

## Dry-run evidence

Commands (the script is `docs/cloud-reports/task-2-dryrun.py`; `/home/user/bbg-dry` is a scratch
`git worktree add --detach`):

```
python3 -I docs/cloud-reports/task-2-dryrun.py /home/user/bbg-dry --base cloud/task-2-port-map \
    --tool /home/user/bbgoated-workspace/scripts/dev/pr73_hunks.py --scratch <dir> --strict-only
python3 -I docs/cloud-reports/task-2-dryrun.py /home/user/bbg-dry --base cloud/task-2-port-map \
    --tool /home/user/bbgoated-workspace/scripts/dev/pr73_hunks.py --scratch <dir>
```

```
strict, each subset alone against cloud/task-2-port-map
  a1  .gitignore                           1                              CONFLICT: error: patch failed: .gitignore:21
  a2  bbhost.example.toml                  1                              applies
  a2  src/core/config.h                    1                              applies
  a2  src/core/config.cpp                  1-3                            applies
  a2  src/host/settings.h                  1,2                            applies
  a2  src/host/options.cpp                 1,15                           applies
  a2  src/main.cpp                         1                              applies
  a2  tests/config_test.cpp                1                              applies
  a2  src/host/gpu.cpp                     6,9,14                         CONFLICT: error: patch failed: src/host/gpu.cpp:1657
  a2  src/host/render.cpp                  1,2,21,22,24,25,26,28,29,30    CONFLICT: error: patch failed: src/host/render.cpp:7
  a2  src/host/window.cpp                  24                             CONFLICT: error: patch failed: src/host/window.cpp:1658
  b   src/host/window.cpp                  2,3,4,25,26,27,28,29,33,34     applies
  b   src/host/window.h                    1                              applies
  b   src/host/overlay.cpp                 1-3                            applies
  b   src/host/overlay.h                   1,2                            applies
  b   src/engine/live_resolution.cpp       1-4                            applies
  b   src/host/options.cpp                 2,3,4,5,6,7,8,11,12,13,14,16   applies
  b   src/host/options.h                   1                              applies
  b   src/host/launcher.cpp                1-6                            applies
  c   src/host/gpu.cpp                     8                              applies
  c   tools/win_package.sh                 1                              applies
  d   src/gcn/translate.cpp                1-10                           applies
  d   src/gcn/translate.h                  1,2                            applies
  d   src/host/translation_cache.cpp       1                              applies
  d   tests/translation_cache_test.cpp     1                              applies
  d   src/host/gpu_internal.h              1                              applies
  d   src/host/gpu.cpp                     13                             applies
  d   src/host/render.cpp                  3-20,23,27                     applies
strict, a1/a2 after reverting ae0b56f's render.cpp, gpu.cpp and window.cpp hunks: ok
  a1  .gitignore                           1                              CONFLICT: error: patch failed: .gitignore:21
  a2  bbhost.example.toml                  1                              applies
  a2  src/core/config.h                    1                              applies
  a2  src/core/config.cpp                  1-3                            applies
  a2  src/host/settings.h                  1,2                            applies
  a2  src/host/options.cpp                 1,15                           applies
  a2  src/main.cpp                         1                              applies
  a2  tests/config_test.cpp                1                              applies
  a2  src/host/gpu.cpp                     6,9,14                         applies
  a2  src/host/render.cpp                  1,2,21,22,24,25,26,28,29,30    applies
  a2  src/host/window.cpp                  24                             applies
```

The sequence (the base is this branch at 48f5145, the same code as 9cc8dc0; the earlier run on
9cc8dc0 printed the same lines, but for the commit SHAs):

```
base cloud/task-2-port-map = 48f5145
R1.1 revert (fork pick ae0b56f, pump-loop tick 0717add)
  R1  git diff ae0b56f~1 ae0b56f -- src/host/render.cpp src/host/gpu.cpp src/host/window.cpp, git apply -R: ok
  R1  git diff 0717add~1 0717add -- src/main.cpp, git apply -R: ok
  R1  cmake/bbgoated.cmake                 edit: delete the fork's dlaa.cpp block (cmake/bbgoated-upscale.cmake registers it)
  == after R1.1 revert: a5a0da1, unmerged none, conflict markers none, whitespace errors none
a1
  a1  from 539d45f: tools/ngx_bridge
  a1  .gitignore                           1                              applies with --3way only
  == after a1: 51be976, unmerged none, conflict markers none, whitespace errors none
a2
  a2  from 539d45f: src/host/dlaa.cpp src/host/dlaa.h src/host/display_settings.h src/host/fsr_upscaler.h src/host/object_motion.cpp src/host/object_motion.h src/host/motion_history.h src/host/shaders/dlaa_mv.comp src/host/shaders/dlaa_mv.spv.h tools/dlaa_cbscan.py docs/dlaa.md tests/display_settings_test.cpp tests/dlaa_motion_test.cpp
  a2  src/host/render.cpp                  1,2,21,22,24,25,26,28,29,30    applies
  a2  src/host/gpu.cpp                     6,9,14                         applies
  a2  src/host/window.cpp                  24                             applies
  a2  src/core/config.h                    1                              applies
  a2  src/core/config.cpp                  1-3                            applies
  a2  src/host/settings.h                  1,2                            applies
  a2  src/host/options.cpp                 1,15                           applies
  a2  src/main.cpp                         1                              applies
  a2  tests/config_test.cpp                1                              applies
  a2  bbhost.example.toml                  1                              applies
  a2  src/host/render.cpp                  edit: h25's object-motion params block out until (d)
  a2  src/host/gpu.cpp                     edit: h1's two includes (not reflex.h)
  a2  src/host/gpu.cpp                     edit: h7's first line (not the derivatives block)
  a2  src/host/window.cpp                  edit: h1's dlaa.h include
  a2  src/core/config.cpp                  edit: h2's BBHOST_FG_BACKEND block out
  a2  src/core/config.cpp                  edit: h3's frame-generation and Reflex template lines out
  a2  bbhost.example.toml                  edit: frame_generation_backend line out
  a2  bbhost.example.toml                  edit: frame_generation lines out
  a2  src/host/dlaa.cpp                    edit: edit 1: no reflex.h
  a2  src/host/dlaa.cpp                    edit: edit 2a: FG instance extensions only when requested
  a2  src/host/dlaa.cpp                    edit: edit 2b: FG device extensions only when requested
  a2  src/host/dlaa.cpp                    edit: edit 3: no Reflex frame id
  a2  src/host/dlaa.cpp                    edit: edit 4: the BDA strip says so
  == after a2 (PR #73's DLAA path): ae74f8d, unmerged none, conflict markers none, whitespace errors none
R1.3 re-apply
  R1  src/host/dlaa.cpp                    edit: E1 includes
  R1  src/host/dlaa.cpp                    edit: E2 the three atomics
  R1  src/host/dlaa.cpp                    edit: E3 NGX's result published
  R1  src/host/dlaa.cpp                    edit: E4 State::last_eval_flip
  R1  src/host/dlaa.cpp                    edit: E5 fail_locked publishes the failure
  R1  src/host/dlaa.cpp                    edit: E6 idle after 600 flips
  R1  src/host/dlaa.cpp                    edit: E7 BBHOST_TEST_DLAA_FAIL
  R1  src/host/dlaa.cpp                    edit: E8 the idle clock restarts
  R1  src/host/dlaa.cpp                    edit: E9 dlaa_running and dlaa_aa_tick appended
  R1  src/host/dlaa.h                      edit: E10 declarations
  R1  the tick patch, git apply --3way: ok
  == after R1.3 re-apply: 3b38bd2, unmerged none, conflict markers none, whitespace errors none
a2 scaffolding (plan Task 4 Steps 3-4)
  scaffold: src/host/fsr_upscaler_stub.cpp, cmake/bbgoated-upscale.cmake, include line in cmake/bbgoated.cmake
  == after a2 scaffolding: 047bb34, unmerged none, conflict markers none, whitespace errors none
b
  b   from 539d45f: src/host/presentation_coordinates.h tests/presentation_coordinates_test.cpp
  b   src/host/window.cpp                  2,3,4,25,26,27,28,29,33,34     applies
  b   src/host/window.h                    1                              applies
  b   src/host/overlay.cpp                 1-3                            applies
  b   src/host/overlay.h                   1,2                            applies
  b   src/engine/live_resolution.cpp       1-4                            applies
  b   src/host/options.cpp                 2,3,4,5,6,7,8,11,12,13,14,16   applies
  b   src/host/options.h                   1                              applies
  b   src/host/launcher.cpp                1-6                            applies
  b   src/host/window.cpp                  edit: h1's presentation_coordinates.h
  b   src/host/window.cpp                  edit: the SR display size under vk_present's lock (h15's intent)
  b   src/host/window.cpp                  edit: h35's removal of host_present's g_display store
  == after b: 66c803e, unmerged none, conflict markers none, whitespace errors none
c
  c   from 539d45f: cmake/fsr.cmake cmake/fsr-clear-order.patch cmake/fsr-debug-view.patch cmake/fsr-frame-retirement.patch LICENSE-FSR.txt src/host/fsr_provider.cpp src/host/fsr_upscaler.cpp tools/win/Install-FSR4.ps1 tools/win/Install-FSR4.bat docs/fsr.md
  c   src/host/gpu.cpp                     8                              applies
  c   tools/win_package.sh                 1                              applies
  == after c: 2e56bd1, unmerged none, conflict markers none, whitespace errors yes:
cmake/fsr-debug-view.patch:8: trailing whitespace.
+
cmake/fsr-debug-view.patch:19: trailing whitespace.
+
cmake/fsr-debug-view.patch:50: trailing whitespace.
+
cmake/fsr-frame-retirement.patch:6: trailing whitespace.
+
cmake/fsr-frame-retirement.patch:41: trailing whitespace.
+
cmake/fsr-frame-retirement.patch:60: trailing whitespace.
+
cmake/fsr-frame-retirement.patch:69: trailing whitespace.
+

d
  d   from 539d45f: tests/motion_history_test.cpp tests/object_motion_shader_test.cpp
  d   src/gcn/translate.cpp                1-10                           applies
  d   src/gcn/translate.h                  1,2                            applies
  d   src/host/translation_cache.cpp       1                              applies
  d   tests/translation_cache_test.cpp     1                              applies
  d   src/host/gpu_internal.h              1                              applies
  d   src/host/gpu.cpp                     13                             applies
  d   src/host/render.cpp                  3-20,23,27                     applies
  d   src/host/render.cpp                  edit: h25's object-motion params block back
  == after d: 4d9aef2, unmerged none, conflict markers none, whitespace errors none
```

The (c) whitespace report lists the blank context lines of PR #73's `cmake/fsr-debug-view.patch`
(lines 8, 19, 50) and `cmake/fsr-frame-retirement.patch` (6, 41, 60, 69): a single space, as in
PR #73. They stay as they are.

Builds of the scratch trees, with upstream's CI flags on the same prefix (clang 18, lld,
`-DBBGOATED=ON`, RelWithDebInfo):

| Tree | Built | ctest |
|---|---|---|
| after (a2), step 3 and the Task 4 scaffolding (928e83d on 9cc8dc0) | bbhost and all tests | `100% tests passed, 0 tests failed out of 63` (3 new: display_settings_test, dlaa_motion_test, ngx_transfer_sync_test) |
| after (b), (c), (d) and the Task 5/7 test registrations (dc6b7db) | bbhost and all tests, incrementally | `100% tests passed, 0 tests failed out of 66` (3 more: presentation_coordinates_test, motion_history_test, object_motion_shader_test) |

```
dlaa_motion_test:              ALL DLAA MOTION VECTOR TESTS PASSED   (lavapipe)
display_settings_test:         custom sizes, SR presets, native modes and overrides passed
ngx_transfer_sync_test:        All ngx_transfer_sync tests passed
presentation_coordinates_test: SR output, menu rows, letterboxing, DPI and resize passed
motion_history_test:           ALL MOTION HISTORY TESTS PASSED!
object_motion_shader_test:     test_game_shaders: skipped (no dump available) / object_motion_shader_test ALL PASSED
```

Warnings: none from dlaa.cpp, object_motion.cpp, window.cpp, options.cpp or launcher.cpp (clang
`-fsyntax-only` on each with the build's flags: 0). gpu.cpp 128 against 123 on the base and
render.cpp 95 against 94, all `-Wmissing-field-initializers` on Vulkan structs initialised with their
`sType` only, as upstream's own. The final run's trees after the (a2) scaffolding and after (d) differ
from the built ones (928e83d, and dc6b7db less its test registrations) in docs only (`git diff --stat`).

## Test results

| Run | Command | Result |
|---|---|---|
| bbgoated/main a00921d (before the rebase), baseline | `CI_WORK=/home/user/ci-work CI_APT=0 CI_CMAKE_ARGS="-DBBGOATED=ON" bash tools/ci_build.sh` | `100% tests passed, 0 tests failed out of 55` |
| cloud/task-R-rebase 9cc8dc0 | the same | `100% tests passed, 0 tests failed out of 60` |
| this branch 48f5145 (before the report commit; the report adds docs only) | the same | `100% tests passed, 0 tests failed out of 60`, 37 ran, 23 skipped |
| scratch tree after (a2) | `cmake -G Ninja ... -DBBGOATED=ON && cmake --build && ctest` | `100% tests passed, 0 tests failed out of 63` |
| scratch tree after (d) | the same, incremental | `100% tests passed, 0 tests failed out of 66` |
| workspace (on main 07bd5c3) | `python3 -I scripts/tests/test_<x>.py` for each of the four | OK, OK, OK, OK (2 + 11 + 3 + 12 tests) |

The 23 skipped: debug_menu_strings_test, libc_native_test, decomp_insn_eboot,
decomp_event_interpreter, decomp_player_data, decomp_item_lots, decomp_talk_evaluator,
decomp_chalice_ritual, decomp_follow_camera, decomp_sfx_ribbons, at9_test, dream_mirror_layout_test,
esd_test, rebirth_script_test, maiden_events_test, randomizer_files_test, gcn_decode_test,
translation_cache_test, gcn_bindings_test, gcn_translate_test, gcn_lift_test, gcn_tess_lift_test,
gcn_names_test (game files, or the game's libc for libc_native_test).

Checks:

```
$ pwsh -NoProfile -File scripts/check-online-untouched.ps1 -Branch cloud/task-2-port-map   (workspace at task R's 1bf6bc7)
online paths untouched since 566184a (1 upstream cherry-picks allowed)
$ git ls-files | grep -iE '(^|/)(nvngx[^/]*\.dll|ngx_bridge\.dll|nvsdk_ngx[^/]*\.(lib|h)|fsr4_model_v07[^/]*\.(spv|bin|json))$'
(nothing)
```

## What needs local verification

This task changed documentation in the code repo, so there is no Windows build or in-game check for
it. Locally:

1. The tool on Windows: `python -I $WS\scripts\tests\test_pr73_hunks.py` (Git for Windows, any
   `core.autocrlf`): `Ran 11 tests ... OK`. Then
   `python -I $WS\scripts\dev\pr73_hunks.py --file src/host/window.cpp --list`: 35 lines, the first
   `h1   @@ -10,10 +10,14  +4 -0  +#include "host/dlaa.h"`.
2. The plan's Task 2 Step 4 dry run with the new numbers, once `bbgoated/main` is the rebased branch
   (or with `origin/cloud/task-2-port-map` in `read-tree`):

   ```powershell
   $repo = "$WS\src\bbgoated"
   $env:GIT_INDEX_FILE = "$SP\dryrun.idx"
   git -C $repo read-tree bbgoated/main
   $sets = [ordered]@{
     ".gitignore|a1" = "1"
     "bbhost.example.toml|a2" = "1"; "src/core/config.h|a2" = "1"; "src/core/config.cpp|a2" = "1-3"; "src/host/settings.h|a2" = "1,2"
     "src/host/options.cpp|a2" = "1,15"; "src/main.cpp|a2" = "1"; "tests/config_test.cpp|a2" = "1"
     "src/host/gpu.cpp|a2" = "6,9,14"; "src/host/render.cpp|a2" = "1,2,21,22,24,25,26,28,29,30"; "src/host/window.cpp|a2" = "24"
     "src/host/window.cpp|b" = "2,3,4,25,26,27,28,29,33,34"; "src/host/window.h|b" = "1"; "src/host/overlay.cpp|b" = "1-3"
     "src/host/overlay.h|b" = "1,2"; "src/engine/live_resolution.cpp|b" = "1-4"; "src/host/options.cpp|b" = "2,3,4,5,6,7,8,11,12,13,14,16"
     "src/host/options.h|b" = "1"; "src/host/launcher.cpp|b" = "1-6"; "src/host/gpu.cpp|c" = "8"; "tools/win_package.sh|c" = "1"
     "src/gcn/translate.cpp|d" = "1-10"; "src/gcn/translate.h|d" = "1,2"; "src/host/translation_cache.cpp|d" = "1"
     "tests/translation_cache_test.cpp|d" = "1"; "src/host/gpu_internal.h|d" = "1"; "src/host/gpu.cpp|d" = "13"; "src/host/render.cpp|d" = "3-20,23,27"
   }
   foreach ($k in $sets.Keys) {
     $file = $k.Split('|')[0]
     python -I $WS\scripts\dev\pr73_hunks.py --file $file --hunks $sets[$k] --out $SP\dry.patch | Out-Null
     git -C $repo apply --cached --check $SP\dry.patch 2>$null
     "{0,-40} {1}" -f $k, $(if ($LASTEXITCODE -eq 0) { "applies" } else { "CONFLICT" })
   }
   Remove-Item Env:GIT_INDEX_FILE; Remove-Item $SP\dryrun.idx
   ```

   Expected: every line `applies`, except `.gitignore|a1`, `src/host/gpu.cpp|a2`,
   `src/host/render.cpp|a2` and `src/host/window.cpp|a2`: `CONFLICT`, the DLAA fork's lines
   (section E's step 1 removes them; .gitignore merges with `--3way`).
3. Or the whole replay, with Windows' Python and Git (a worktree made by Windows' git does not work
   from WSL): `git -C $WS\src\bbgoated worktree add --detach $SP\dry bbgoated/main`, then
   `python -I $WS\src\bbgoated\docs\cloud-reports\task-2-dryrun.py $SP\dry --base bbgoated/main`
   (`--tool` defaults to the workspace's `scripts\dev\pr73_hunks.py`; the script refuses the main
   checkout). Expected: the sequence log above, every step `unmerged none, conflict markers none`.
   Afterwards `git -C $WS\src\bbgoated worktree remove --force $SP\dry`.
4. `scripts\check-online-untouched.ps1 -Branch cloud/task-2-port-map` with task R's workspace
   version (a853b54, 1bf6bc7): `online paths untouched since 566184a (1 upstream cherry-picks allowed)`.

## Open concerns

- Rulings 7 and 8 (R3's live-resolution consequence and the template line) change PR #73's code in
  Tasks 4-5 beyond the plan; please confirm them.
- The plan's text still has the dropped `bbgoated_options` machinery (Task 5 Steps 4-7 and 10,
  Review Focus 2, Global Constraints "Config") and, for Task 4 Step 8, an Off default; pr73-port.md
  records R3/R4, the plan itself is not edited here.
- PR #73's `dlaa.h` says `[dlaa] enabled` defaults to true when the bridge is present, but its
  `config_on()` never reads it; with R5's seed the behaviour is right, and the file's comment stays
  PR #73's (it is taken verbatim).
- A pre-commit `git diff --check` would stop Task 6's commit on the FSR patches' blank context
  lines; they must stay as they are.
- Not verified here: the FSR provider build (Task 6's CMake switch; FireBurn's sources were not
  fetched) and anything Windows (the cross build, the MSVC bridge, the game). Instrumented
  object-motion draws still run translated, not lifted, shaders (FIocker's resolution of the old
  C2): Task 7 measures it.

## Appendix A - PR #73's hunks (`pr73_hunks.py --list`, 566184a..539d45f)

<details>
<summary>173 hunks in 32 files</summary>

```
.gitignore
    h1   @@ -21,8 +21,15  +7 -0  +/build-dev/
CMakeLists.txt
    h1   @@ -108,6 +108,7  +1 -0  +  src/host/reflex.cpp
    h2   @@ -226,6 +227,10  +4 -0  +  if(MINGW)
    h3   @@ -260,11 +265,16  +6 -1  +  target_compile_definitions(bbhost PRIVATE BBHOST_HAVE_VULKAN=1)
    h4   @@ -294,7 +304,7  +1 -1  -  # https://github.com/ocornut/imgui, third_party/imgui/VERSION) on SDL3's 2D renderer.
    h5   @@ -308,7 +318,9  +3 -1  -  set_source_files_properties(src/host/gpu_untile.cpp src/host/depth_copy.cpp src/host/in
    h6   @@ -596,6 +608,51  +45 -0  +# Motion regressions are portable, including the native Windows renderer build.
bbhost.example.toml
    h1   @@ -77,6 +77,24  +18 -0  +[dlss]
src/core/config.cpp
    h1   @@ -157,6 +157,21  +15 -0  +    str("dlss.mode", &c->dlss_mode);
    h2   @@ -733,6 +748,14  +8 -0  +    if (const char* e = std::getenv("BBHOST_UPSCALER")) {
    h3   @@ -959,6 +982,26  +20 -0  +           "\n[dlss]\n"
src/core/config.h
    h1   @@ -44,6 +44,18  +12 -0  +    // [dlss]: output dimensions are independent of the game's render size.
src/engine/frame_rate.cpp
    h1   @@ -1,3 +1,4  +1 -0  +#include "host/reflex.h"
    h2   @@ -1292,6 +1293,18  +12 -0  +    auto* f = reinterpret_cast<SprjFlipper*>(static_cast<std::uintptr_t>(saved[5]));
    h3   @@ -1302,14 +1315,7  +0 -7  -    auto* f = reinterpret_cast<SprjFlipper*>(static_cast<std::uintptr_t>(saved[5]));
src/engine/live_resolution.cpp
    h1   @@ -12,6 +12,7  +1 -0  +#include "host/settings.h"
    h2   @@ -1088,6 +1089,10  +4 -0  +    const HostSettings& display = host_startup_settings();
    h3   @@ -1128,7 +1133,7  +1 -1  -    const std::uint64_t live_px = std::max(start_px, std::min(static_cast<std::uint64_t>(
    h4   @@ -1142,8 +1147,8  +2 -2  -    g_display[0] = std::max<std::uint32_t>(max_w, static_cast<std::uint32_t>(w));
src/gcn/translate.cpp
    h1   @@ -115,6 +115,7  +1 -0  +    Id p_psb_v4f = 0;  // PhysicalStorageBuffer pointer to v4f
    h2   @@ -194,6 +195,10  +4 -0  +    // TranslateOptions::object_motion
    h3   @@ -263,7 +268,7  +1 -1  -    Id p_uni_u32 = 0;
    h4   @@ -340,6 +345,7  +1 -0  +        p_psb_v4f = m.type_pointer(spv::ScPhysicalStorageBuffer, t_v4f);
    h5   @@ -1238,6 +1244,15  +9 -0  +    Id in_raw_location(std::uint32_t location) {
    h6   @@ -1258,6 +1273,122  +116 -0  +    void emit_vertex_motion() {
    h7   @@ -3676,6 +3807,16  +10 -0  +        if (opt.object_motion) {
    h8   @@ -3695,14 +3836,27  +14 -1  +        if (opt.object_motion) {
    h9   @@ -4212,6 +4366,9  +3 -0  +        if (opt.stage == Stage::Vertex && opt.object_motion) {
    h10  @@ -4223,6 +4380,9  +3 -0  +        if (opt.stage == Stage::Pixel && opt.object_motion) {
src/gcn/translate.h
    h1   @@ -203,6 +203,8  +2 -0  +    bool object_motion = false;
    h2   @@ -474,8 +476,20  +13 -1  +    // TranslateOptions::object_motion (offset 512, std140)
src/hle/gnm.cpp
    h1   @@ -1,3 +1,4  +1 -0  +#include "host/reflex.h"
    h2   @@ -366,6 +367,7  +1 -0  +    host::reflex::on_simulation_end();
src/hle/gnm_exec.cpp
    h1   @@ -1,3 +1,4  +1 -0  +#include "host/reflex.h"
    h2   @@ -2099,6 +2100,7  +1 -0  +    std::uint64_t reflex_frame_id = 0;  // immutable simulation ID captured on the submit
    h3   @@ -2214,6 +2216,7  +1 -0  +        host::reflex::set_cp_thread_frame_id(job.reflex_frame_id);
    h4   @@ -2292,6 +2295,7  +1 -0  +        host::reflex::set_cp_thread_frame_id(0);
    h5   @@ -2372,7 +2376,10  +3 -0  +    const auto frame_id = host::reflex::current_sim_frame_id();
    h6   @@ -2393,6 +2400,7  +1 -0  +        host::reflex::set_cp_thread_frame_id(saved_frame);
    h7   @@ -2426,6 +2434,7  +1 -0  +    q->jobs.back().reflex_frame_id = frame_id;
src/hle/system.cpp
    h1   @@ -1,3 +1,4  +1 -0  +#include "host/reflex.h"
    h2   @@ -1546,6 +1547,7  +1 -0  +    host::reflex::on_input_sampled();
src/host/gpu.cpp
    h1   @@ -1,7 +1,10  +3 -0  +#include "host/reflex.h"
    h2   @@ -117,6 +120,7  +1 -0  +    std::uint64_t reflex_frame_id = 0;
    h3   @@ -157,7 +161,13  +6 -0  +        VkLatencySubmissionPresentIdNV attribution{};
    h4   @@ -529,7 +539,7  +1 -1  -                       VkFence fence, std::uint32_t items, std::uint64_t serial, std::uin
    h5   @@ -540,6 +550,7  +1 -0  +    job.reflex_frame_id = reflex_frame_id;
    h6   @@ -1657,6 +1668,7  +1 -0  +    dlaa_add_instance_extensions(iext);
    h7   @@ -1931,10 +1943,36  +26 -0  +    dlaa_add_device_extensions(g.phys, dext);
    h8   @@ -2062,6 +2100,27  +21 -0  +    // Temporal FSR shaders use FP16/INT8 storage and, for the optional FSR 4
    h9   @@ -2124,6 +2183,7  +1 -0  +    f2.features.shaderStorageImageExtendedFormats = supported.shaderStorageImageExtendedF
    h10  @@ -2202,6 +2262,12  +6 -0  +    VkPhysicalDevicePresentIdFeaturesKHR reflex_present_id{};
    h11  @@ -2213,6 +2279,7  +1 -0  +    host::reflex::init(g.instance, g.phys, g.device, reflex_enabled);
    h12  @@ -2543,7 +2610,17  +10 -0  +    const auto id = host::reflex::cp_thread_frame_id();
    h13  @@ -2570,6 +2647,7  +1 -0  +    static_assert(sizeof(gcn::StageParams) <= kParamsStride);
    h14  @@ -4237,6 +4315,7  +1 -0  +    fsr_retire_locked(g.completed_submits);
    h15  @@ -5222,6 +5301,7  +1 -0  +        es.reflex_frame_id = g.recording_reflex_id;
    h16  @@ -5234,12 +5314,17  +5 -0  +        job.reflex_frame_id = g.recording_reflex_id;
    h17  @@ -5420,14 +5505,14  +2 -2  -std::uint64_t host_gpu_submit_presenter(void* cmd, void* wait, std::uint32_t wait_stage,
    h18  @@ -5436,6 +5521,7  +1 -0  +    job.reflex_frame_id = reflex_frame_id;
src/host/gpu.h
    h1   @@ -505,7 +505,7  +1 -1  -std::uint64_t host_gpu_submit_presenter(void* cmd, void* wait, std::uint32_t wait_stage,
src/host/gpu_internal.h
    h1   @@ -80,7 +80,7  +1 -1  -constexpr std::uint32_t kParamsStride = 512;  // bytes per StageParams slot in the ring (
    h2   @@ -408,6 +408,7  +1 -0  +    std::uint64_t recording_reflex_id = 0;
    h3   @@ -1361,7 +1362,7  +1 -1  -                       VkFence fence, std::uint32_t items, std::uint64_t serial, std::uin
    h4   @@ -1405,7 +1406,7  +1 -1  -    std::uint64_t foreign_submit(VkCommandBuffer cmd, VkSemaphore wait, VkPipelineStageFl
src/host/gpu_stub.cpp
    h1   @@ -7,7 +7,7  +1 -1  -std::uint64_t host_gpu_submit_presenter(void*, void*, std::uint32_t, void*, void*) { retu
src/host/launcher.cpp
    h1   @@ -5,6 +5,7  +1 -0  +#include "host/display_settings.h"
    h2   @@ -187,10 +188,27  +18 -1  +    int custom_width = 1920, custom_height = 1080;
    h3   @@ -288,6 +306,10  +4 -0  +        if (resolution == 14 && !host::valid_resolution(custom_width, custom_height)) {
    h4   @@ -301,6 +323,13  +7 -0  +        e.push_back({"dlss", "mode", toml_string(host::DlssModes[dlss_mode])});
    h5   @@ -324,7 +353,14  +8 -1  -        host_opt_set_resolution_index(resolution);
    h6   @@ -633,7 +669,68  +62 -1  -        ImGui::Combo("Render resolution", &resolution, kResolutions, static_cast<int>(siz
src/host/options.cpp
    h1   @@ -1,6 +1,7  +1 -0  +#include "host/display_settings.h"
    h2   @@ -89,9 +90,20  +11 -0  +    kObjectMotion,
    h3   @@ -222,11 +234,11  +4 -4  -    {"resolution", "Resolution", {"1280x720", "1600x900", "1920x1080", "2560x1440", "3200
    h4   @@ -249,6 +261,18  +12 -0  +    {"object_motion", "Object motion vectors", {"Off", "On"}, 0,
    h5   @@ -273,6 +297,12  +6 -0  +    {Row::Option, nullptr, kUpscalerBackend},
    h6   @@ -351,7 +381,8  +2 -1  -    return live_resolution_available() && std::sscanf(g_set[kResolution].values[static_ca
    h7   @@ -630,6 +661,11  +5 -0  +            // The hooks may already accept requests before the GX device is
    h8   @@ -712,19 +748,29  +19 -9  +    h.upscaler_backend = index_of(kUpscalerBackend);
    h9   @@ -776,7 +822,10  +4 -1  -    const int n = static_cast<int>(s.values.size());
    h10  @@ -786,6 +835,15  +9 -0  +    if (!from_file) {
    h11  @@ -841,8 +899,18  +12 -2  -    // video.fps_cap is the frame rate's default: 30, 60, 90, 120 or 0
    h12  @@ -853,14 +921,28  +16 -2  -            if (value == g_set[id].values[k]) g_set[id].index = static_cast<int>(k);
    h13  @@ -916,6 +998,9  +3 -0  +            if (key == "resolution") seed(kResolution, val);
    h14  @@ -992,6 +1077,29  +23 -0  +    if (const char* e = std::getenv("BBHOST_UPSCALER"); e && e[0])
    h15  @@ -1354,6 +1462,11  +5 -0  +const HostSettings& host_startup_settings() {
    h16  @@ -1407,6 +1520,19  +13 -0  +bool host_opt_display_resolution(int* w, int* h) {
src/host/options.h
    h1   @@ -43,6 +43,10  +4 -0  +// The selected native/output size, before a DLSS preset lowers the input size.
src/host/overlay.cpp
    h1   @@ -31,6 +31,8  +2 -0  +float g_build_w = 0.0f, g_build_h = 0.0f;
    h2   @@ -89,14 +91,18  +5 -1  -void host_overlay_reset() {
    h3   @@ -434,6 +440,12  +6 -0  +        // A queued overlay retains its own coordinate space when a loading,
src/host/overlay.h
    h1   @@ -6,7 +6,7  +1 -1  -// Coordinates are the game's display-buffer pixels, the same space
    h2   @@ -17,7 +17,7  +1 -1  -void host_overlay_reset();
src/host/recorder.cpp
    h1   @@ -405,12 +405,12  +2 -2  -            stream_submit_job(r->cmds, r->ncmds, r->wait, r->wait_stage, VK_NULL_HANDLE,
    h2   @@ -976,7 +976,7  +1 -1  -std::uint64_t Rec::foreign_submit(VkCommandBuffer cmd, VkSemaphore wait, VkPipelineStageF
    h3   @@ -987,6 +987,7  +1 -0  +    r->reflex_frame_id = reflex_frame_id;
src/host/render.cpp
    h1   @@ -7,6 +7,8  +2 -0  +#include "host/dlaa.h"
    h2   @@ -270,6 +272,7  +1 -0  +    std::int8_t dlaa_anchor = -1;  // 1 for DLAA's anchor (dlaa_is_anchor), 0 for the res
    h3   @@ -1991,6 +1994,8  +2 -0  +    bool object_motion = false;
    h4   @@ -2077,6 +2082,7  +1 -0  +        std::uint32_t object_motion, motion_location;
    h5   @@ -2084,6 +2090,8  +2 -0  +    k.object_motion = s.object_motion;
    h6   @@ -2325,7 +2333,16  +9 -0  +    if (s.object_motion) last = 7;
    h7   @@ -3265,6 +3282,8  +2 -0  +    bool object_motion = false;
    h8   @@ -3272,6 +3291,7  +1 -0  +    if (in.object_motion) key = fnv1a(&in.motion_location, sizeof(in.motion_location), ke
    h9   @@ -3299,6 +3319,8  +2 -0  +    po.object_motion = in.object_motion;
    h10  @@ -3425,6 +3447,8  +2 -0  +    bool object_motion = false;
    h11  @@ -3438,6 +3462,7  +1 -0  +    if (in.object_motion) key = fnv1a(&in.motion_location, sizeof(in.motion_location), ke
    h12  @@ -3642,6 +3667,7  +1 -0  +    if (in.object_motion) return;  // draw-specific variants stay in the translation cach
    h13  @@ -3665,6 +3691,7  +1 -0  +    if (in.object_motion) return;
    h14  @@ -3768,6 +3795,8  +2 -0  +    vo.object_motion = in.object_motion;
    h15  @@ -4108,8 +4137,8  +2 -2  -                                  &ps_dims, &ps_sampler_modes, &ps_input_map, clip_discar
    h16  @@ -4218,13 +4247,13  +2 -2  -                              s.tess_window};
    h17  @@ -5778,6 +5807,7  +1 -0  +    if (in.object_motion) return;
    h18  @@ -5826,6 +5856,7  +1 -0  +    if (in.object_motion) return;
    h19  @@ -6582,7 +6613,17  +10 -0  +    if (t_object_motion_draw) last = 7;
    h20  @@ -6636,6 +6677,9  +3 -0  +    const VkImageView motion = t_object_motion_draw && s.depth
    h21  @@ -9047,6 +9091,8  +2 -0  +        if (g_dlaa_cb_capturing.load(std::memory_order_relaxed) && !b.pointer) dlaa_cb_no
    h22  @@ -12287,7 +12333,24  +17 -0  +struct SrHudState {
    h23  @@ -12906,6 +12969,126  +120 -0  +    // A visible instance ordinal is not a persistent object identity. Culling
    h24  @@ -12957,6 +13140,47  +41 -0  +    // DLAA: at the anchor (YEBIS's first colour write), before its pass, the
    h25  @@ -13704,11 +13928,24  +13 -0  +    t_dlaa_main_draw = dlaa_main_draw_locked(s.depth ? s.depth->base : 0, s.depth_control
    h26  @@ -13836,7 +14073,9  +2 -0  +        t_dlaa_cb_stage = static_cast<int>(st);  // F12's constant-buffer capture (dlaa.c
    h27  @@ -14474,6 +14713,7  +1 -0  +    if (s.object_motion) object_motion_attach_locked(VK_NULL_HANDLE, s.depth->width, s.de
    h28  @@ -14654,16 +14894,54  +39 -1  +    // DLAA's jitter: the scene's geometry shifted by a sub-pixel amount (dlaa.cpp).
    h29  @@ -15090,6 +15368,8  +2 -0  +        t_dlaa_main_draw = false;
    h30  @@ -15247,6 +15527,7  +1 -0  +    gpu::dlaa_request_cb_capture(g_capture_dir);
src/host/settings.h
    h1   @@ -23,6 +23,13  +7 -0  +    int output_width = 1920, output_height = 1080; // reconstructed/native display size
    h2   @@ -74,3 +81,6  +3 -0  +// Immutable after the game starts. Restart-required temporal settings cannot
src/host/stream_ops.h
    h1   @@ -69,6 +69,7  +1 -0  +    std::uint64_t reflex_frame_id; // simulation ID captured at recording start
    h2   @@ -78,6 +79,7  +1 -0  +    std::uint64_t reflex_frame_id;
src/host/translation_cache.cpp
    h1   @@ -116,6 +116,8  +2 -0  +    h.flag(o.object_motion);
src/host/window.cpp
    h1   @@ -10,10 +10,14  +4 -0  +#include "host/dlaa.h"
    h2   @@ -58,18 +62,11  +3 -10  -struct FitRect {
    h3   @@ -130,6 +127,14  +8 -0  +void mouse_remap_locked() {
    h4   @@ -137,18 +142,7  +1 -12  -    if (ww <= 0 || wh <= 0 || !g_display_w || !g_display_h) {
    h5   @@ -277,6 +271,7  +1 -0  +    std::uint64_t last_reflex_present_id = 0;
    h6   @@ -300,6 +295,7  +1 -0  +    std::int64_t present_block_ns = 0;
    h7   @@ -517,6 +513,11  +5 -0  +    VkSwapchainLatencyCreateInfoNV latency{VK_STRUCTURE_TYPE_SWAPCHAIN_LATENCY_CREATE_INF
    h8   @@ -529,9 +530,12  +3 -0  +        host::reflex::on_swapchain_destroying(old);
    h9   @@ -621,6 +625,7  +1 -0  +    if (g_vk.present_queue) host::reflex::on_present_queue_created(g_vk.present_queue);
    h10  @@ -659,6 +664,7  +1 -0  +    host::fg_get().init(g_vk.instance, g_vk.phys, g_vk.device, g_vk.family, g_vk.queue);
    h11  @@ -714,29 +720,27  +16 -18  -void vk_present(int buffer_index, std::uint64_t display_va, unsigned display_w, unsigned
    h12  @@ -764,11 +768,15  +6 -2  -    VkResult r = vkAcquireNextImageKHR(g_vk.device, g_vk.swapchain, 100000000ull, acquire
    h13  @@ -787,14 +795,14  +3 -3  -            return;
    h14  @@ -818,18 +826,33  +25 -10  -    if (present_flush) {
    h15  @@ -842,11 +865,24  +15 -2  +    const unsigned eff_w = display_w ? display_w : source.width;
    h16  @@ -857,27 +893,27  +11 -11  -    std::uint32_t src_x = 0, src_y = 0, src_w = display_w, src_h = display_h;
    h17  @@ -888,19 +924,35  +23 -7  -        const bool have = display_va && host_gpu_display_image(display_va, &image, &forma
    h18  @@ -960,8 +1012,20  +14 -2  -        if (!display_va || !host_gpu_blit_display(g_vk.cmd, display_va, g_vk.images[idx],
    h19  @@ -1016,20 +1080,43  +25 -2  +    const VkSemaphore waits[2] = {source.ready, acquire_sem};
    h20  @@ -1037,11 +1124,20  +11 -2  -    if (g_vk.gpu_wait) {
    h21  @@ -1065,17 +1161,49  +33 -1  +    // Synthetic presents share a real frame's identity for out-of-band
    h22  @@ -1091,12 +1219,12  +2 -2  -    work_us += us(t_acquired, t_submitted) + us(t_gpu_done, t_presented);
    h23  @@ -1141,6 +1269,237  +231 -0  +    return true;
    h24  @@ -1658,6 +2017,12  +6 -0  +                // Ctrl+F1-F4: DLAA's keys (host/dlaa.h); Ctrl+F5-F8 the same,
    h25  @@ -1860,7 +2225,15  +9 -1  -    host_overlay_reset();
    h26  @@ -1892,11 +2265,7  +1 -5  -        float dw = 0.0f;
    h27  @@ -1923,12 +2292,7  +1 -6  -            float dw = 0.0f, dh = 0.0f;
    h28  @@ -1976,12 +2340,7  +1 -6  -            float dw = 0.0f, dh = 0.0f;
    h29  @@ -2015,13 +2374,7  +1 -7  -        float dw = 0.0f, dh = 0.0f;
    h30  @@ -2141,12 +2494,105  +93 -0  +    int fg_slot = -1;
    h31  @@ -2155,12 +2601,21  +13 -4  -            g_present_cv.wait(lk, [] { return g_present_quit || g_present_next.has; });
    h32  @@ -2201,10 +2656,13  +3 -0  +    if (g_fg_thread.joinable()) g_fg_thread.join();
    h33  @@ -2257,19 +2715,9  +3 -13  -    float px = 0.0f, py = 0.0f;
    h34  @@ -2277,10 +2725,8  +2 -4  -    const float s = std::min(static_cast<float>(bw) / 1920.0f, static_cast<float>(bh) / 1
    h35  @@ -2351,20 +2797,24  +12 -8  +        PresentFlip captured_flip = flip;
src/host/window.h
    h1   @@ -71,9 +71,8  +2 -3  -    // Where the pointer is in the game's own display buffer, in pixels - the
    h2   @@ -177,6 +176,8  +2 -0  +    std::uint64_t reflex_frame_id = 0;  // CP job simulation ID, retained by the presente
src/main.cpp
    h1   @@ -968,6 +968,12  +6 -0  +    {
tests/config_test.cpp
    h1   @@ -98,6 +98,10  +4 -0  +    CHECK(!load("[dlss]\nmode = \"quality\"\n").dlss_object_motion);
tests/translation_cache_test.cpp
    h1   @@ -123,6 +123,17  +11 -0  +    // These key checks also run without a game dump in CI. Motion-enabled
tools/win_package.sh
    h1   @@ -28,6 +28,10  +4 -0  +cp LICENSE-FSR.txt "$dir/LICENSE-FSR.txt"
```

</details>
