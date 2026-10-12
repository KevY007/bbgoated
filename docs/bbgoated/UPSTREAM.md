# bbgoated upstream ledger

bbgoated is droogie/bbhost plus a linear patch stack (`bbg/<topic>:` commits) on branch `bbgoated/main`. This file records what the stack is based on, what was taken early from upstream, and what has been reviewed in every project we track. `scripts/sync-upstream.ps1` in the workspace reads the tracked-refs table.

## Base

| Base ref | SHA | Taken on |
|---|---|---|
| upstream/master (v0.2.18 + PR #92, #79, #118, next-steps as PR #116, and PR #87) | 566184a74f071166299bb656c4af479e5bbe2771 | 2026-10-12 |

## Tracked refs (last reviewed)

| Ref | Last reviewed | Date | Notes |
|---|---|---|---|
| `upstream/master` | 566184a74f07 | 2026-10-12 | the base: v0.2.18, PR #79 (Wayland setup), PR #92 (libc rand/ctype), next-steps (PR #116), PR #118 (libc errno, stdio, wide and float paths), PR #87 (online: chara-id) |
| `upstream/release/next-steps` | 63c1cea36749 | 2026-10-12 | merged into master (PR #116) at 63c1cea; the base before 566184a |
| `upstream-pr/73` | 539d45f41e16 | 2026-10-12 | FIocker DLSS/FSR/FG/Reflex, rebased onto 566184a: adb8f38 temporal upscaling/FG/MV/Reflex squash, 85d81a9 DLSS 3x/4x menu rows, a3a7fa7 FSR descriptor retirement, 539d45f docs; Phase 2 parts bin (report I reviewed 3bfcb65) |
| `upstream-pr/100` | 7817a4231f8c | 2026-10-12 | Raikzer descriptor reuse / state suppression; Phase 3 candidate |
| `upstream-pr/116` | 63c1cea36749 | 2026-10-12 | = next-steps; merged into master |
| `upstream-pr/117` | 0b0ea897469b | 2026-10-12 | dispatch-trace leak fix; taken (its head rebased onto 566184a, the same change as 021112f) |
| `upstream-pr/118` | 72672ad4ae1a | 2026-10-12 | libc errno, stdio, wide and float fixes; merged into master, in the base |
| `upstream-pr/122` | 1ae05f919ee7 | 2026-10-12 | kyo pc settings perf - Phase 3 review |
| `fiocker/render/dlss-sr-framegen-pipeline` | 539d45f41e16 | 2026-10-12 | = PR #73 head |
| `dlaa/dlaa` | 96cd17d4a4fb | 2026-10-12 | YoucefNabil DLAA fork: its DLAA is taken (ed58faa, fe06cb5 picked as ae0b56f, 8e42461), with the PR #73 BDA fix and the anti-aliasing hold on top |
| `bbport/master` | 3c57da4198ad | 2026-10-12 | 0.51; techniques reviewed (report E) |
| `supermedo/windows` | ec8340a9f9d8 | 2026-10-12 | windows-v1.6; reviewed (report F) |

## Taken early (cherry-picks, `git cherry-pick -x`)

Rows: upstream SHA | picked SHA | source | reason. Column 1 is plain text on purpose (the sync script only reads backticked refs).

| Upstream SHA | Picked SHA | Source | Reason |
|---|---|---|---|
| 82c58b4 | 31edecf | upstream PR #74 (open) | TLS setup shared by sceHttp and account calls, Windows native CA trust for non-Schannel curl; no URL, header or UA change (online path: allowed); the PR's head rebased onto 566184a, the same change as 6bab81d |
| 94db371 | 00d3cd8 | upstream PR #70 (open) | chromatic aberration toggle works on Windows (the user's log: "cannot place chromatic aberration's stub"); the PR's head rebased onto 566184a, the same change as e57427b |
| 0b0ea89 | f171529 | upstream PR #117 (open) | dispatch-trace records only inside the coverage window (~14 MiB/min leak in v0.2.18); the PR's head rebased onto 566184a, the same change as 021112f |

## Ported (not cherry-picked)

Pieces of unmerged work re-applied as bbgoated commits (`bbg/upscale:`), with the record of what was taken in `docs/bbgoated/pr73-port.md`. Column 1 is plain text, so `check-online-untouched.ps1` and `sync-upstream.ps1` do not read these rows.

| Source | bbgoated commit | What | Left out |
|---|---|---|---|
| upstream-pr/73 at 539d45f | bbg/upscale: ngx_bridge ABI v2 from PR #73 (this commit) | tools/ngx_bridge (ABI v2) | none |
| upstream-pr/73 at 539d45f | bbg/upscale: DLSS evaluation path from PR #73 - DLAA at the HDR anchor, camera motion vectors, jitter | (a2): dlaa.cpp/.h, display_settings.h, fsr_upscaler.h, object_motion.*, motion_history.h, shaders/dlaa_mv.*, tools/dlaa_cbscan.py, docs/dlaa.md, display_settings_test, dlaa_motion_test; render.cpp, gpu.cpp, window.cpp, config.h/.cpp, settings.h, options.cpp, main.cpp, config_test and bbhost.example.toml hunks (pr73-port.md tables A-C); ngx_transfer_sync_test registered | Reflex (`host/reflex.h`, its frame ID); the FG backend switch and template lines; render.cpp h25's object-motion params (d); gpu.cpp h7's derivatives block. Frame generation's NGX extension queries run only when it is requested |
| upstream-pr/73 at 539d45f (on 566184a) | (filled by Tasks 5-7) | | frame generation (85d81a9's rows included), Reflex, presenter/frame-loop timing (pr73-port.md table B) |

The DLAA fork's picks above (ae0b56f, 8e42461, in the tracked refs) gave way to PR #73's versions of the same files in (a1) and (a2): pr73-port.md, section E.

## Upstream behaviour bbgoated changes by default

| Upstream commit | What it does | Found | bbgoated |
|---|---|---|---|
| c715cc2 (next-steps) | `BBHOST_VERTEX_STAGE`: a vertex record the buffer shadow keeps in host memory is copied into device memory for each draw | Windows, RTX 5080: ~35 MiB of per-draw copies a frame where a ring's pages went dynamic; the command processor waited on GPU fences for most of each frame, 37-46 fps at the Astral Clocktower, Mensis, Lumenwood Garden and the Fishing Hamlet in about half the sessions (workspace `docs/perf/baseline-2026-10-12.md`, cells I-L) | off by default (`BBHOST_VERTEX_STAGE=1` turns it on; a range copied earlier in the same recording is shared then, `BBHOST_VERTEX_STAGE_SHARE`) |
| c313d5d (next-steps) | GDS counters on the GPU's timeline (`BBHOST_GDS_COUNTERS`) | ~3 ms more GPU a frame at the Astral Clocktower (11.3 against 8.4 ms with it off); no effect on the stalls above | kept on: it fixes a GPU hang (a particle system's garbage counter) |

## Releases

| Tag | Base | Notes |
|---|---|---|
| v0.2.18-bbgoated.4 | next-steps 63c1cea | .3 plus the fixes from its release review: bbgoated never runs on bbhost's data folder (refused however it is named or spelled; the import also drops `[ paths ]`-style spellings), the first-start copy closes its saves guard as soon as the saves are in (a failed cache copy cannot merge bbhost's saves into later progress), an empty saves folder or a missing source no longer stops the copy for good, the game's anti-aliasing comes back when DLAA fails or idles, staged vertex copies are shared only from the same source; docs: no shared save folder, one shared sign-in |
| v0.2.18-bbgoated.3 (superseded by .4 the same day) | next-steps 63c1cea | fully separate from bbhost (own config, options, plugins, data; first start copies bbhost's, caches included); NVIDIA DLAA from the DLAA fork with the game's AA held off while it runs (the file keeps the player's choice); the next-steps vertex-staging stall fixed (per-draw vertex copies off by default: 60 fps in 12 of 12 sessions at the four places against 37-46 in about half) |
| v0.2.18-bbgoated.2 | next-steps 63c1cea | replaces .1: sprint-fix.toml dropped (the base owns that site), the shared options file keeps unknown keys, upstream file modes kept; checked at 60/90/120 fps that all 23 per-frame redirects install |
| v0.2.18-bbgoated.1 | next-steps 63c1cea | WITHDRAWN: its sprint-fix.toml wrote over 0x1914c24, a site the base's frame-rate code owns, which then refused all 23 per-frame redirects at 60/90/120 fps (fixed in .2). First bbgoated build: next-steps base, PRs #92/#79/#74/#70/#117, sprint and Intel tone-map fixes, config isolation (BBGOATED=ON), light-grid basis switch, red-zone audit; no upscalers yet |

## Rebase procedure (new upstream release or next-steps update)

1. `scripts\sync-upstream.ps1 -Fetch` and read what is new per ref.
2. `git switch -c bbgoated/next <new base>` then `git rebase --onto bbgoated/next <old base SHA> bbgoated/main` on a copy branch; rerere replays earlier resolutions. Cherry-picks upstream merged are dropped by the rebase (same patch id).
3. Build (`scripts\build.ps1 -CMakeArgs "-DBBGOATED=ON"`), smoke, `scripts\check-online-untouched.ps1`, area-check A/B against the previous bbgoated build.
4. Fast-forward `bbgoated/main`, tag `v<upstream tag>-bbgoated.<n>`, update the tables above.

Online or decomp changes upstream are rebased promptly: co-op peers run them.
