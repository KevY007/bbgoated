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
| `dlaa/dlaa` | 96cd17d4a4fb | 2026-10-12 | YoucefNabil DLAA fork (superseded by PR #73's DLAA path) |
| `bbport/master` | 3c57da4198ad | 2026-10-12 | 0.51; techniques reviewed (report E) |
| `supermedo/windows` | ec8340a9f9d8 | 2026-10-12 | windows-v1.6; reviewed (report F) |

## Taken early (cherry-picks, `git cherry-pick -x`)

Rows: upstream SHA | picked SHA | source | reason. Column 1 is plain text on purpose (the sync script only reads backticked refs).

| Upstream SHA | Picked SHA | Source | Reason |
|---|---|---|---|

## Releases

| Tag | Base | Notes |
|---|---|---|

## Rebase procedure (new upstream release or next-steps update)

1. `scripts\sync-upstream.ps1 -Fetch` and read what is new per ref.
2. `git switch -c bbgoated/next <new base>` then `git rebase --onto bbgoated/next <old base SHA> bbgoated/main` on a copy branch; rerere replays earlier resolutions. Cherry-picks upstream merged are dropped by the rebase (same patch id).
3. Build (`scripts\build.ps1 -CMakeArgs "-DBBGOATED=ON"`), smoke, `scripts\check-online-untouched.ps1`, area-check A/B against the previous bbgoated build.
4. Fast-forward `bbgoated/main`, tag `v<upstream tag>-bbgoated.<n>`, update the tables above.

Online or decomp changes upstream are rebased promptly: co-op peers run them.
