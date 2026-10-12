# Task R: bbgoated rebased onto upstream master 566184a

Cloud run of 2026-10-12 (session https://claude.ai/code/session_01CwcqehwvDGj2RGaEtsj4Cx; the commits carry the controller's `Claude-Session` line, as README rule 5 asks).

| Repo | Branch | Head before this report | Base |
|---|---|---|---|
| KevY007/bbgoated | `cloud/task-R-rebase` | 9cc8dc0 (the report commit goes on top) | upstream/master 566184a |
| KevY007/bbgoated-workspace | `cloud/task-R-rebase` | 1bf6bc7 | main 2ddc62f |

## Result

- The stack is on 566184a: the three open-PR picks at their new heads, then all 26 kept commits in their original order, then `bbg/docs: rebased onto upstream master 566184a`. 30 commits on top of 566184a.
- No conflicts. Every commit's change is the same as before: 29 of 29 replayed commits have the same `git patch-id --stable` as the commits they replace, and the old tip to new tip diff (a00921d to a85a77e, the stack before the docs commit) is exactly upstream's own change from 63c1cea plus the dropped picks to 566184a, file by file (20 files, the added and removed lines identical).
- Linux CI (upstream's `tools/ci_build.sh`, unmodified, `CI_CMAKE_ARGS="-DBBGOATED=ON"`): `100% tests passed, 0 tests failed out of 60` (25 skipped for missing game files). Also checked: upstream 566184a by itself (54 of 54), the branch with BBGOATED=OFF (58 of 58), bbgoated's Python tool tests (8 tests: 7 OK, 1 skipped on Linux), and the compiler warnings (343, the same set as upstream's).
- `check-online-untouched.ps1` (workspace) reads its base from the ledger's Base row whatever upstream ref it names, so it is 566184a on this branch: `online paths untouched since 566184a (1 upstream cherry-picks allowed)`, run with PowerShell 7.4.6 here. A second commit adds PR #87's `src/engine/chara_id.cpp` to the protected paths (ruling 7).

## Facts checked before the rebase

- `upstream/master` = 566184a74f07; 63c1cea is its ancestor; `git rev-list --count upstream/master..upstream/release/next-steps` = 0.
- 63c1cea..566184a lists 144 commits, but the tree changes in 25 files only: master's first-parent history has its own root (a3847b6, the public v0.2.x line) and joined next-steps' history at the PR #116 merge (02b882c); that line's code was already in next-steps. What 566184a adds over 63c1cea: PR #79 and PR #92 (bbgoated had both), PR #118 (libc errno, stdio, wide and float paths, `host_thread_set_name`, five new libc tests) and PR #87 (`src/engine/chara_id.cpp`, the prologue hook at 0x2043ee0, installed from `hle_patch_guest`).
- Dropped picks are in master: 8ea60f6, 69580d6, 1822748 and 9c71024 are ancestors of 566184a, and `src/host/launcher.cpp`, `src/hle/libc_ctype.h`, `src/hle/libc_random.h` and their two tests are byte-identical between 681720a (the old stack with all four picks applied) and 566184a. The second PR #79 pick was 5db407e (`git log --grep="cherry picked from commit 1822748"`).
- New PR heads are one commit each on 566184a: PR #74 82c58b4, PR #70 94db371, PR #117 0b0ea89 (parents 566184a). PR #73 is 539d45f, four commits on 566184a (adb8f38, 85d81a9, a3a7fa7, 539d45f); FIocker's `render/dlss-sr-framegen-pipeline` is the same 539d45f (`git ls-remote`). PR #122 is 1ae05f9, one commit on 566184a. PR #100 has not moved (7817a4231f8c); the DLAA fork's `dlaa` has not moved (96cd17d4a4fb).
- `origin/bbgoated/main` was still a00921d when I pushed.

## PR #74, #70, #117: range-diff of old pick against new head

`git range-diff <old>^! <new>^!` for each: the content did not change. The only difference is the commit message, where the old pick carried its `(cherry picked from commit ...)` line and the PR head carries none. The patch-ids agree (PR #74 77193faf, PR #70 e2ec624d, PR #117 47910a15). PR #74 and PR #117's diffs are byte-identical; PR #70's differs only in blob indices and the CMakeLists.txt hunk's position (`@@ -560,6 +560,13 @@ if(UNIX)` became `@@ -596,6 +596,13 @@ if(UNIX)`, because PR #118's test registrations, 35 lines, and PR #87's source line sit above it).

## Commit map

Old = `63c1cea..a00921d`; new = `566184a..cloud/task-R-rebase`. "Picked from" is the commit's `cherry picked from` line.

| Old | New | Picked from | Subject |
|---|---|---|---|
| cfc81e0 | dropped | 8ea60f6 | libc: fix ctype table (PR #92, in master) |
| 0bbf4b6 | dropped | 69580d6 | libc: fix rand implementation (PR #92, in master) |
| 5db407e | dropped | 1822748 | Changed setup scaling logic (PR #79, in master) |
| 681720a | dropped | 9c71024 | Reverted some changes that broke x11, but don't impact wayland (PR #79, in master) |
| 43039cf | 31edecf | 82c58b4 | net: use Windows native certificate trust for portable curl builds (PR #74) |
| ba04e60 | 00d3cd8 | 94db371 | fix: disable chromatic dispersion on Windows and preserve guest flags (PR #70) |
| 9377e61 | f171529 | 0b0ea89 | fix: capture dispatch records only inside the coverage trace window (PR #117) |
| 689dacc | accaad6 | 689dacc | bbg/docs: upstream ledger |
| 3fd38d4 | 5951ad2 | 3fd38d4 | bbg/tools: area check runs on Windows (prebuilt probe, pipe reader thread, --windowed) |
| dd9f8d7 | 03c69c6 | dd9f8d7 | bbg/docs: ledger - base composition |
| 729ea17 | 4137d00 | 729ea17 | bbg/feat: bbgoated config isolation - bbgoated.toml layer, own data folder seeded from bbhost's saves |
| 0445c83 | 12cfd67 | 0445c83 | bbg/fix: sprint fix above 90 fps and Intel tone-map fix (cpu_vendor manifests) |
| d121f9f | a7bfdd3 | d121f9f | bbg/perf: light-grid tile basis switch (render or window) |
| 191232b | 3e71e8a | 191232b | bbg/tools: red-zone audit (BBHOST_REDZONE_AUDIT=1, observe-only) |
| 71b26a5 | 79ec3e3 | 71b26a5 | bbg/docs: README, setup guide and build guide |
| 2fa7afe | 5cb1a02 | 2fa7afe | bbg/docs: repository links; ledger picked SHAs after the author-email rewrite |
| 9bbbd0a | 7315833 | 9bbbd0a | bbg/docs: ledger - release v0.2.18-bbgoated.1 |
| 19490dc | 717de60 | 19490dc | bbg/fix: drop sprint-fix.toml - the base's frame-rate code already owns that site |
| 973eae2 | 402f553 | 973eae2 | bbg/fix: keep upstream's file modes |
| 58c2c5d | 9460110 | 58c2c5d | bbg/fix: the shared options file keeps the keys bbgoated does not know |
| 72e5270 | 26009a0 | 72e5270 | bbg/docs: ledger - release v0.2.18-bbgoated.2 |
| 81ff1f7 | 30d7a92 | 81ff1f7 | bbg/feat: bbgoated is fully separate - its own config, options, plugins and data |
| 5a80558 | 86f9fcc | 65932a4 | bbg/perf: a vertex range staged once in a recording serves its later draws (stage-share pick; its own line kept) |
| 480d92d | ae0b56f | ed58faa | render: optional NVIDIA DLAA on the HDR scene, before YEBIS (DLAA fork; its own line kept) |
| c0ebd6c | 8e42461 | fe06cb5 | dlaa: track 16 depth targets, so the main depth survives busy frames (DLAA fork; its own line kept) |
| db6d8a0 | 6664c3f | db6d8a0 | bbg/upscale: DLAA holds the game's anti-aliasing off; no pre-core BDA extension |
| 8f823fa | abe5e6f | 8f823fa | bbg/upscale: DLAA holds the game's anti-aliasing live only; the file keeps the player's |
| 8fa7f88 | e4f5fee | 8fa7f88 | bbg/fix: next-steps' per-draw vertex copies off by default (BBHOST_VERTEX_STAGE=1) |
| 52020fe | 0717add | 52020fe | bbg/upscale: DLAA's anti-aliasing hold runs in the pump loop, with or without a window |
| 767104d | f382cfe | 767104d | bbg/fix: bbgoated never runs on bbhost's data folder; the first-start copy cannot reopen saves |
| 3982626 | 6edcb6b | 3982626 | bbg/fix: a staged vertex range is shared only from the same bytes of the same source |
| 13fd88a | 414027f | 13fd88a | bbg/fix: the game's anti-aliasing comes back when DLAA fails or stops resolving frames |
| a00921d | a85a77e | a00921d | bbg/docs: ledger - release v0.2.18-bbgoated.4 (supersedes .3) |
| - | 9cc8dc0 | - | bbg/docs: rebased onto upstream master 566184a |

Every replayed commit has exactly one `cherry picked from` line. Authors are unchanged (FIocker, Youcef, rehab and KevY007 as before, all bbgoated ones on the 8596387+KevY007 noreply address); the committer is KevY007.

## Conflicts and their resolution

None. `git cherry-pick` stopped nowhere; its only automatic merge was CMakeLists.txt in 729ea17's pick (4137d00): the one bbgoated line, `include(cmake/bbgoated.cmake OPTIONAL)` with its comment, stays the last line of the file, after PR #118's new libc test registrations and PR #87's `chara_id.cpp`. Mode changes match too: 402f553 makes the same five 100644 to 100755 changes 973eae2 made, and `git diff --summary 566184a cloud/task-R-rebase` shows no mode change.

Semantic checks for the code that arrived under the stack:
- PR #118 moved upstream's thread naming to `host_thread_set_name`; no bbgoated code calls `pthread_setname_np` (only upstream's `core/portable.cpp` and `hle/pthread.cpp` do).
- PR #87's hook (0x2043ee0) against bbgoated's manifest: `tools/test_patch_overlap.py` passes, but it reads only eight-digit `0x0XXXXXXX` addresses and chara_id.cpp writes `0x2043ee0`, so I also ran the same check over 6- to 8-digit addresses (an ad-hoc copy, not committed): bbgoated's one manifest, intel-tonemap.toml (`gate` at 0x25843e5), is clear of every engine address. Its only hit is upstream's own: fov-uncap.toml's `clamp` at 0x183af4e, which src/engine/camera.cpp also patches. Neither file differs between 63c1cea and this branch, and the same hit is there on 566184a alone.

## Files changed

Code repo, on top of the replayed stack:
- `docs/bbgoated/UPSTREAM.md` (9cc8dc0): Base table (upstream/master 566184a74f071166299bb656c4af479e5bbe2771, 2026-10-12); Tracked refs (`upstream/master` at 566184a, `upstream/release/next-steps` merged into master at 63c1cea, `upstream-pr/73` at 539d45f with the four commits, new `upstream-pr/122` at 1ae05f9, plus ruling 4's rows); Taken table (the PR #92 and #79 rows removed; PR #74, #70, #117 rows now 82c58b4 to 31edecf, 94db371 to 00d3cd8, 0b0ea89 to f171529, each saying which old upstream SHA it replaces). The rest of the file is unchanged.
- `docs/cloud-reports/task-R.md`: this report.

Workspace repo (`scripts/check-online-untouched.ps1`):
- a853b54 `check-online-untouched: base is upstream master 566184a`: the default base comes from the ledger's Base row for any `upstream/...` ref (regex `^\|\s*upstream/[^|]*\|\s*([0-9a-f]{40})\s*\|`; before, `upstream/release/next-steps` only), and the header comment says so. The allow list is unchanged: it still comes from column 1 of the Taken table, so the PR #74 row (82c58b4) allows the TLS pick 31edecf.
- 1bf6bc7 `check-online-untouched: chara-id's game hook (PR #87) is an online path`: `src/engine/chara_id.cpp` joins the protected paths.

## Rulings

1. Ruling: the kept bbgoated commits were picked with `-x`, as the brief's command reads; the three that already carried a line (stage-share 5a80558, DLAA ed58faa and fe06cb5) were picked without `-x`, which keeps their one line. - The brief writes `git cherry-pick -x <sha>` for the kept stack, and the line maps each new commit to its old one. - Cost if wrong: the bbgoated commits' messages now end with `(cherry picked from commit <old sha>)` after their `Claude-Session` line (as 5a80558's already did); a message-only reword removes it, and the trees stay the same.
2. Ruling: committer KevY007 <8596387+KevY007@users.noreply.github.com> on every commit; authors kept. - README rule 5's identity, and cherry-pick keeps the author. - Cost if wrong: none to the code.
3. Ruling: order as the brief gives it, the three PR picks first and then the kept stack, so 689dacc and 3fd38d4 (before the PR picks in the old stack) now follow them. - The brief's step 2 order. - Cost if wrong: none; no commit touches the others' files, and every patch-id is unchanged.
4. Ruling: in UPSTREAM.md I also updated the rows the rebase made wrong, besides the ones the brief names: `upstream-pr/117` (head 0b0ea89; the old 021112f is gone from the PR, so the sync script would count master's whole history from it), `upstream-pr/118` (merged, in the base), `upstream-pr/116` (merged), `fiocker/render/dlss-sr-framegen-pipeline` (539d45f, checked equal to PR #73's head) and the `dlaa/dlaa` row's picked SHAs (480d92d and c0ebd6c became ae0b56f and 8e42461). - The ledger names picked SHAs so they can be found, and the sync script reads every row. - Cost if wrong: a doc edit.
5. Ruling: the Base table holds one row, the new base; the next-steps row was replaced, not kept beside it. - The check script takes the first base row it finds; the old base stays in the next-steps tracked row and the Releases table. - Cost if wrong: a doc edit.
6. Ruling: `check-online-untouched.ps1` gets its default base from the branch's own ledger, which now says 566184a, rather than a hard-coded SHA. - The script already worked that way; only its regex refused the new row. Old branches and tags (v0.2.18-bbgoated.4) still check against their own base, 63c1cea, and the next rebase needs only the ledger edit. - Cost if wrong: a one-line default (`[string]$Base = "566184a74f071166299bb656c4af479e5bbe2771"`).
7. Ruling: `src/engine/chara_id.cpp` is protected, in its own workspace commit (1bf6bc7). - PR #87 is upstream's online code, it arrived with the new base, and the list already names the other engine-side online hooks' .cpp files (playlog, sos_signs, five_players, maiden_events, summon_invite). Before this commit, a non-pick edit to it passed the check (below). - Cost if wrong: drop 1bf6bc7.
8. Ruling: the CI script ran unmodified, with `CI_APT=0` after I installed its exact apt list, and with the three sources cloned by git into `$CI_WORK/src` under the names its `fetch()` globs look for (`Vulkan-Headers-1.4.350`, `SDL-release-3.4.16`, `ffmpeg-7.1.1`). `CI_WORK=/home/user/ci`, outside the repo. - The proxy blocks the tarballs; nothing in the repo changed. - Cost if wrong: none.
9. Ruling: checks beyond the brief: the CI script on 566184a alone first (to tell any failure of ours from upstream's), a BBGOATED=OFF build and ctest of the branch, bbgoated's Python tool tests, a warning comparison and the wider overlap scan. - They are cheap here (about two minutes a build). - Cost if wrong: none.

## TDD evidence

The code repo change is a rebase and a docs edit, with no new logic and no test to write. The workspace script's change has logic, so it was run before and after with PowerShell 7.4.6 (the Linux build from PowerShell's GitHub release, SHA-256 checked), with `src/bbgoated` linked to the code repo (`/src/` is ignored by the workspace's .gitignore). Throwaway local branches built the negative cases and were deleted afterwards (never pushed).

RED, the script as on main (2ddc62f), on the rebased branch:
```
$ pwsh -NoProfile -File scripts/check-online-untouched.ps1 -Branch cloud/task-R-rebase
Exception: /home/user/bbgoated-workspace/scripts/check-online-untouched.ps1:14
  14 | … ot $Base) { throw "no base SHA in the ledger's Base table; give -Base …
     | no base SHA in the ledger's Base table; give -Base
exit=1
```
GREEN, after a853b54:
```
$ pwsh -NoProfile -File scripts/check-online-untouched.ps1 -Branch cloud/task-R-rebase
online paths untouched since 566184a (1 upstream cherry-picks allowed)
exit=0
$ pwsh -NoProfile -File scripts/check-online-untouched.ps1 -Branch bbgoated/main
online paths untouched since 63c1cea (1 upstream cherry-picks allowed)
exit=0
```
The PR #74 allowance comes from the Taken row (a branch whose ledger lacks the 82c58b4 row), and a non-pick edit is still caught (an edit to `src/net/http.cpp`):
```
online paths CHANGED since 566184a:
  31edecf net: use Windows native certificate trust for portable curl builds  (picked 82c58b4, not in the ledger's Taken table)
exit=1
online paths CHANGED since 566184a:
  ded2fca tmp: a non-pick edit on an online path  (not an upstream cherry-pick)
exit=1
```
chara_id.cpp, an edit to it: RED at a853b54 `online paths untouched since 566184a (1 upstream cherry-picks allowed)`, exit 0; GREEN at 1bf6bc7:
```
online paths CHANGED since 566184a:
  82568c5 tmp: a non-pick edit to chara_id.cpp  (not an upstream cherry-pick)
exit=1
```
Why the base had to move: with `-Base 63c1cea` on the new branch the script lists 23 of upstream master's own commits (master's separate history down to a3847b6, the merge commits, PR #87 and PR #118's thread naming) as `not an upstream cherry-pick`, exit 1.

## Test results

Commands (tools/ci_build.sh as in 566184a, unmodified):
```
cd /home/user/ci/src   # CI_WORK=/home/user/ci
git clone --depth 1 --branch v1.4.350 https://github.com/KhronosGroup/Vulkan-Headers Vulkan-Headers-1.4.350
git clone --depth 1 --branch release-3.4.16 https://github.com/libsdl-org/SDL SDL-release-3.4.16
git clone --depth 1 --branch n7.1.1 https://github.com/FFmpeg/FFmpeg ffmpeg-7.1.1
sudo apt-get install -y -q --no-install-recommends <the script's package list>
# 1. upstream alone, in a worktree at 566184a (builds the prefix)
CI_APT=0 CI_WORK=/home/user/ci bash tools/ci_build.sh
# 2. this branch, with the prefix cached and the first build tree moved aside
CI_APT=0 CI_WORK=/home/user/ci CI_CMAKE_ARGS="-DBBGOATED=ON" bash tools/ci_build.sh
# 3. this branch with BBGOATED=OFF: the script's cmake line without CI_CMAKE_ARGS, own build tree, then ctest
```

| Run | Configure line | ctest |
|---|---|---|
| branch, BBGOATED=ON (the CI run) | `-- bbhost: bbgoated build (its own config and data folders)` | `100% tests passed, 0 tests failed out of 60`, Total Test time 12.55 s, 25 skipped |
| 566184a alone | no bbgoated line | `100% tests passed, 0 tests failed out of 54`, 25 skipped |
| branch, BBGOATED=OFF | no bbgoated line | `100% tests passed, 0 tests failed out of 58`, 25 skipped |

- The ON run has upstream's 54 plus chromatic_stub_test (PR #70), bbgoated_config_test, patch_manifest_cpu_test, stage_share_test, redzone_audit_test and dlaa_aa_test; OFF lacks bbgoated_config_test and redzone_audit_test, which cmake/bbgoated.cmake registers only with BBGOATED.
- The 25 skips are the same in all three: tests that need game files (decomp_*, gcn_*, esd_test and others), and PR #118's new libc_native_test, which skips without `BBHOST_LIBC_ELF` (the decrypted guest libc.elf).
- Python: `python3 -I tools/test_patch_overlap.py -v` gives `Ran 2 tests ... OK`; `python3 -I tools/test_area_check_win.py -v` gives `Ran 6 tests ... OK (skipped=1)` (the Windows-paths case).
- Warnings: 343 in each build, and the same file and message set in the 566184a and branch builds: the stack adds none.

Time per phase (4 CPUs):

| Phase | Time |
|---|---|
| apt-get install of the script's list (most of it preinstalled) | 13 s |
| the three git clones (depth 1) | 10 s |
| Vulkan headers (configure and install) | 2 s |
| SDL3 (configure 19 s, build 12 s) | 31 s |
| ffmpeg (configure 10 s, build 30 s, install) | 41 s |
| bbhost configure | 1 s |
| bbhost build, branch (378 steps) / 566184a (356 steps) | 124 s / 123 s |
| ctest | 12.6 s |
| whole script, branch with the prefix cached | 2 min 18 s |
| whole script, 566184a with the prefix built | 3 min 30 s |
| BBGOATED=OFF configure, build and ctest | 2 min 20 s |

## What needs local verification (not attempted here)

1. Windows build: `scripts\build.ps1 -Ref cloud/task-R-rebase -CMakeArgs "-DBBGOATED=ON" -Probe` after `git -C src\bbgoated fetch origin cloud/task-R-rebase:cloud/task-R-rebase`, or the brief's `scripts\build.ps1 -CMakeArgs "-DBBGOATED=ON" -Probe` with the branch checked out in src\bbgoated. New for the mingw build: PR #118's Windows errno translation and portable thread naming, and PR #87's `chara_id.cpp`.
2. Smoke boot: `scripts\smoke.ps1 -Exe <pkg>\bbgoated.exe`. With the 1.09 eboot the log must not say `chara id: off`. After an online sign-in it says `chara id: <id>` once, or `chara id: this character has none from the server (0x8000000000000000); asking for one` for a character the server never gave an id.
3. Isolation: `scripts\iso-check.ps1 -Exe <pkg>\bbgoated.exe`.
4. Frame rate: `scripts\check-framerate.ps1 -Exe <pkg>\bbgoated.exe -Fps 60`, then `-Fps 90` and `-Fps 120`: all 23 per-frame redirects installed, nothing left out or refused.
5. Online paths: with the workspace on `cloud/task-R-rebase`, `scripts\check-online-untouched.ps1 -Branch cloud/task-R-rebase` should print `online paths untouched since 566184a (1 upstream cherry-picks allowed)`. After bbgoated/main moves, the defaults print the same.
6. Area-check A/B against v0.2.18-bbgoated.4 (the old base) at the four places: `scripts\area-check.ps1 -Pkg <new pkg> -B <v0.2.18-bbgoated.4 pkg> -Out <dir> -Areas <four places>`. The new runtime code is PR #118's libc fixes and PR #87's login hook, so frame times should match within noise.
7. Optional: PR #118's native libc ABI test, which skipped here, with the game's libc in WSL: `BBHOST_LIBC_ELF=<decrypted libc.elf> ctest -R libc_native_test`.

Moving bbgoated/main to this branch is not a fast-forward: a00921d is not an ancestor of the new stack, so the push needs `--force-with-lease=bbgoated/main:a00921d`. The old stack stays reachable through the release tags up to 13fd88a (v0.2.18-bbgoated.4), but a00921d (the .4 ledger commit, which a85a77e's `cherry picked from` line names) is reachable only from bbgoated/main today. If it should stay reachable, push a ref to it before the update.

## Open concerns

- `tools/test_patch_overlap.py` misses engine addresses written with seven digits (`0x2043ee0`, `0x183af4e`). Widening it would at once flag upstream's own fov-uncap.toml and camera.cpp pair, so it needs an allowance for that pair; I left it as it is.
- `.github/README.md` still says "Based on bbhost's next release (`release/next-steps`)". This is still true in substance (master 566184a is next-steps merged plus the libc and chara-id fixes, and bbhost's next release ships it), so I did not change it. The wording could say master.
- The workspace's Phase 2 plan expects `online paths untouched since 63c1cea (7 upstream cherry-picks allowed)`; the check now prints `since 566184a (1 upstream cherry-picks allowed)` (only the PR #74 pick touches the protected paths, as on the old stack, where it also printed 1).
- The upstream fetch also copied upstream's v0.2.12 to v0.2.18 tags into this cloud clone; no tag was pushed or changed anywhere.
