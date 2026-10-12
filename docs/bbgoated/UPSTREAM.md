# bbgoated upstream ledger

bbgoated is droogie/bbhost plus a linear patch stack (`bbg/<topic>:` commits) on branch `bbgoated/main`. This file records what the stack is based on, what was taken early from upstream, and what has been reviewed in every project we track. `scripts/sync-upstream.ps1` in the workspace reads the tracked-refs table.

## Base

| Base ref | SHA | Taken on |
|---|---|---|
| upstream/release/next-steps (draft PR #116) | 63c1cea36749246b05633667df76616b931b0caf | 2026-10-12 |

## Tracked refs (last reviewed)

| Ref | Last reviewed | Date | Notes |
|---|---|---|---|
| `upstream/master` | 43d7e958eac7 | 2026-10-12 | v0.2.18 + PR #79 (Wayland setup), PR #92 (libc rand/ctype) |
| `upstream/release/next-steps` | 63c1cea36749 | 2026-10-12 | the base |
| `upstream-pr/73` | 3bfcb657eae8 | 2026-10-12 | FIocker DLSS/FSR/FG/Reflex; Phase 2 parts bin (report I) |
| `upstream-pr/100` | 7817a4231f8c | 2026-10-12 | Raikzer descriptor reuse / state suppression; Phase 3 candidate |
| `upstream-pr/116` | 63c1cea36749 | 2026-10-12 | = next-steps |
| `upstream-pr/117` | 021112fd48bb | 2026-10-12 | dispatch-trace leak fix; taken |
| `upstream-pr/118` | 72672ad4ae1a | 2026-10-12 | libc formatted stream errors; review when merged |
| `fiocker/render/dlss-sr-framegen-pipeline` | 3bfcb657eae8 | 2026-10-12 | = PR #73 head |
| `dlaa/dlaa` | 96cd17d4a4fb | 2026-10-12 | YoucefNabil DLAA fork: its DLAA is taken (ed58faa, fe06cb5 picked as 480d92d, c0ebd6c), with the PR #73 BDA fix and the anti-aliasing hold on top |
| `bbport/master` | 3c57da4198ad | 2026-10-12 | 0.51; techniques reviewed (report E) |
| `supermedo/windows` | ec8340a9f9d8 | 2026-10-12 | windows-v1.6; reviewed (report F) |

## Taken early (cherry-picks, `git cherry-pick -x`)

Rows: upstream SHA | picked SHA | source | reason. Column 1 is plain text on purpose (the sync script only reads backticked refs).

| Upstream SHA | Picked SHA | Source | Reason |
|---|---|---|---|
| 8ea60f6 | cfc81e0 | upstream/master (PR #92) | libc ctype table; merged upstream 2026-10-11 |
| 69580d6 | 0bbf4b6 | upstream/master (PR #92) | libc rand: the guest's 30-bit LCG on Windows (AI rolls, Make Contact); merged upstream |
| 1822748 | 5db407e | upstream/master (PR #79) | setup window scaling (Wayland); merged upstream |
| 9c71024 | 681720a | upstream/master (PR #79) | setup window scaling follow-up; merged upstream |
| 6bab81d | 43039cf | upstream PR #74 (open) | TLS setup shared by sceHttp and account calls, Windows native CA trust for non-Schannel curl; no URL, header or UA change (online path: allowed) |
| e57427b | ba04e60 | upstream PR #70 (open) | chromatic aberration toggle works on Windows (the user's log: "cannot place chromatic aberration's stub") |
| 021112f | 9377e61 | upstream PR #117 (open) | dispatch-trace records only inside the coverage window (~14 MiB/min leak in v0.2.18) |

## Upstream behaviour bbgoated changes by default

| Upstream commit | What it does | Found | bbgoated |
|---|---|---|---|
| c715cc2 (next-steps) | `BBHOST_VERTEX_STAGE`: a vertex record the buffer shadow keeps in host memory is copied into device memory for each draw | Windows, RTX 5080: ~35 MiB of per-draw copies a frame where a ring's pages went dynamic; the command processor waited on GPU fences for most of each frame, 37-46 fps at the Astral Clocktower, Mensis, Lumenwood Garden and the Fishing Hamlet in about half the sessions (workspace `docs/perf/baseline-2026-10-12.md`, cells I-L) | off by default (`BBHOST_VERTEX_STAGE=1` turns it on; a range copied earlier in the same recording is shared then, `BBHOST_VERTEX_STAGE_SHARE`) |
| c313d5d (next-steps) | GDS counters on the GPU's timeline (`BBHOST_GDS_COUNTERS`) | ~3 ms more GPU a frame at the Astral Clocktower (11.3 against 8.4 ms with it off); no effect on the stalls above | kept on: it fixes a GPU hang (a particle system's garbage counter) |

## Releases

| Tag | Base | Notes |
|---|---|---|
| v0.2.18-bbgoated.3 | next-steps 63c1cea | fully separate from bbhost (own config, options, plugins, data; first start copies bbhost's, caches included); NVIDIA DLAA from the DLAA fork with the game's AA held off while it runs (the file keeps the player's choice); the next-steps vertex-staging stall fixed (per-draw vertex copies off by default: 60 fps in 12 of 12 sessions at the four places against 37-46 in about half) |
| v0.2.18-bbgoated.2 | next-steps 63c1cea | replaces .1: sprint-fix.toml dropped (the base owns that site), the shared options file keeps unknown keys, upstream file modes kept; checked at 60/90/120 fps that all 23 per-frame redirects install |
| v0.2.18-bbgoated.1 | next-steps 63c1cea | WITHDRAWN: its sprint-fix.toml wrote over 0x1914c24, a site the base's frame-rate code owns, which then refused all 23 per-frame redirects at 60/90/120 fps (fixed in .2). First bbgoated build: next-steps base, PRs #92/#79/#74/#70/#117, sprint and Intel tone-map fixes, config isolation (BBGOATED=ON), light-grid basis switch, red-zone audit; no upscalers yet |

## Rebase procedure (new upstream release or next-steps update)

1. `scripts\sync-upstream.ps1 -Fetch` and read what is new per ref.
2. `git switch -c bbgoated/next <new base>` then `git rebase --onto bbgoated/next <old base SHA> bbgoated/main` on a copy branch; rerere replays earlier resolutions. Cherry-picks upstream merged are dropped by the rebase (same patch id).
3. Build (`scripts\build.ps1 -CMakeArgs "-DBBGOATED=ON"`), smoke, `scripts\check-online-untouched.ps1`, area-check A/B against the previous bbgoated build.
4. Fast-forward `bbgoated/main`, tag `v<upstream tag>-bbgoated.<n>`, update the tables above.

Online or decomp changes upstream are rebased promptly: co-op peers run them.
