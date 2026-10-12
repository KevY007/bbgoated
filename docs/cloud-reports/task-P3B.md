# Task P3B: Phase 3 wave-1 switches, second half (Tasks 8, 9, 12, 14, 15)

Cloud run of 2026-10-12 (session https://claude.ai/code/session_01Eq9DQSvHocwLFwb5SdKUzS; the commits carry the controller's `Claude-Session` line, as README rule 5 asks).

| Repo | Branch | Base | Head before this report |
|---|---|---|---|
| KevY007/bbgoated | `cloud/p3-b-2` (not `cloud/p3-b`: see the next section) | be5ef07 `bbg/perf: switch core ...` (P3A's Task 3, on `cloud/p3-a`; parent ebb97dc = `cloud/task-R-rebase`) | 1c7c04e (this report's commit goes on top) |
| KevY007/bbgoated-workspace | not changed | | |

## Another run on `cloud/p3-b`

`cloud/p3-b` on origin is another run of this same task (author KevY007, commits from 12:43 UTC, its report 840cd21 at 13:10), pushed task by task while this run worked; this run's poll watched only `cloud/p3-a`, and its push was rejected as non-fast-forward. Its branch was not touched (no force push); this run's work is on **`cloud/p3-b-2`**. Both start at be5ef07 with the same five subjects and mostly the same choices. The differences that matter, for the controller to pick or merge:

| | `cloud/p3-b` (other run, 840cd21) | `cloud/p3-b-2` (this run) |
|---|---|---|
| Task 12, the occlusion rule (review item 1) | `g_iv.queries < g_iv.cap`, the same fix | the same |
| Task 12, a draw that may not skip while its pipeline's variant is on a worker (review item 2) | builds the **fallback** in place (the plan's code), and drops the draw when the fallback binds differently | builds the no-fallback variant in place under its own key (ruling 5) |
| Task 12, that check on a draw lacking a constant-buffer binding | skipped too (its report says so) | not skipped: it needs the fallback anyway (ruling 5) |
| Task 14, a window across a once-watched hole on Linux (review item 3) | **missed write**: `ww_hole_check` built against its `write_watch.cpp` prints `FAIL 45 five` | stamped, Linux only (ruling 11): `ww_hole_check: ok` |
| Task 9, a copy the live share refuses | still counted by the frames and ping-pong rules (3 refusals in 60 flips send a page to host memory for good under `SHADOW_HOT=frames`) | undone (ruling 2) |
| Task 9, ping-pong and retry bookkeeping | an entry for every GPU-written page | only pages the rules have seen copied (ruling 3) |
| Task 9, audit | host binds; also the upload-buffer failure, as `no-mirror` | host and mirror binds (ruling 4); the upload-buffer failure not counted |
| Task 15, the switch's value | `p[0] == '1'` | set and not 0/off/false/no, and a bounds check (ruling 12) |
| Tests | also commits tests of the real fault path (`bbg_fault_window_test` includes `core/write_watch.cpp`) and of the guard with stand-ins (`bbg_frame_pacing_test` includes `frame_pacing.cpp`) | pure-core tests; the fault path checked with scratch programs, not committed |
| Task 12, `PassHistory` prune | at most once a flip | as the plan (at every note past 4096 passes) |
| Trial merge with `cloud/p3-a` | clean, ON 70 of 70 (at 973d6fb) | clean, ON 70 of 70 (at ff0f44f, P3A's report included) |

Recommendation: do not ship `cloud/p3-b`'s Task 12 and Task 14 as they are (review items 2 and 3). Either take `cloud/p3-b-2` and port from `cloud/p3-b` its committed fault-path and guard tests (adding `ww_hole_check`'s case to the window test), its once-a-flip prune and its upload-buffer audit; or take `cloud/p3-b` and port rulings 2, 3, 5, 11 and 12 from here.

## Result

- Five commits on be5ef07, one per task, in the plan's order, with the plan's subjects:

  | Task | Commit | Switch | Upstream functions touched |
  |---|---|---|---|
  | 8 | bd6ff44 | `BBHOST_BBG_FRAMES_AHEAD=1..3` | video.cpp `hle_video_finish_flip` (one line + include) |
  | 9 | fa1ccd8 | `BBHOST_BBG_SHADOW_HOT/_RETRY_S/_PINGPONG/_FLOOR_MB/_AUDIT` | buffer_shadow.cpp `mirror_room`, `go_dynamic`, `shadow_locate_locked`, `shadow_written_locked` |
  | 12 | 4a272fc | `BBHOST_BBG_ASYNC_SKIP=1` | render.cpp `draw_impl` (the lean-variant block, after it, before `begin_pass`) + a helper above it; occlusion.cpp new `occlusion_interval_open_locked`; gpu_internal.h its declaration |
  | 14 | 637a65e | `BBHOST_BBG_FAULT_WINDOW_KB=16..1024` | write_watch.cpp `write_watch_on_fault` (after the look-ahead, and its failed-range path) + `write_watch_set_window_pages`; write_watch.h the declaration |
  | 15 | 1c7c04e | `BBHOST_BBG_PCORES=1` | host_clock.cpp `host_thread_set_class` (Windows branch, first statement) + `pcores_for_this_thread` above it |

- Every switch is unset by default and then the hooked code takes upstream's path; with every `BBHOST_BBG_*` unset none of this code logs anything (checked on the Linux binaries, below).
- An independent read-only review of the five commits (a separate agent, against Review Focus 3 and 4, switch-off identity and the handler rules) found three real bugs and three smaller issues: the three bugs and two of the smaller issues are fixed in their task's commit (section "Review" below; rulings 5, 6 and 11, then 2 and 12), and the third, a diagnostic gap, is under "Open concerns".
- Linux builds with `-DBBGOATED=ON` and `-DBBGOATED=OFF` (upstream `tools/ci_build.sh`'s flags, clang 18): `100% tests passed, 0 tests failed out of 66` (ON) and `out of 64` (OFF), 25 skipped for missing game files in each. The five new tests pass in both.
- `scripts/check-online-untouched.ps1 -Branch cloud/p3-b-2` (PowerShell 7.4.6 here, the code repo linked at `src/bbgoated` for the run): `online paths untouched since 566184a (1 upstream cherry-picks allowed)`.
- `git diff --summary ebb97dc..cloud/p3-b-2 | grep -c "mode change"`: 0 (and 0 in each commit).
- No `CMakeLists.txt` or `cmake/bbgoated.cmake` edit: every new source sits where Task 3's globs look (`src/host/perf/core/*.h`, `src/host/perf/*.cpp`, `src/host/perf/vk/*.cpp`, `tests/bbg_*_test.cpp`).

## Base

The poll for `cloud/p3-a` (`git ls-remote` every 2 minutes) found `bbg/perf: switch core (BBHOST_BBG_* environment, BBHOST_BBG_PERF and [perf], perf hooks)` at be5ef07 about 14 minutes in; this run's task branch (local `cloud/p3-b`, pushed as `cloud/p3-b-2`) was created from that commit, not from P3A's later ones. Its API is the plan's (`perf_value`, `perf_on`, `perf_uint`, `perf_register_hook`, `perf_report_due`, `perf_init`); its globs are unconditional, so the perf sources build in ON and OFF builds alike and the hooks in upstream files need no `#if`. One difference from the plan's text: the `bbg perf:` line lists the late upstream switches only when a bbgoated switch is set too.

While P3A's commit was pending I built the plan's Task 3 text on a local throwaway branch (never pushed) and developed Tasks 8 and 9 test-first on it. On be5ef07 both were replayed the same way: the test alone (RED), then the commit (GREEN). Tasks 12, 14 and 15 were written directly on the task branch. After the review, the five commits were replayed once more onto be5ef07 with each fix folded into its task's commit (the branch had not been pushed). The RED/GREEN lines below are all from the task branch on be5ef07 (pushed as `cloud/p3-b-2`).

## Rulings

1. **Task 8: only work handed over before flip N is waited for.** `FrameNeeds::push` returns flip N-k's need only when it is below flip N's own; equal means no submission between them, so it may count the recording still open, which only the command processor (the waiting thread) would hand over. With the default `BBHOST_SUBMIT_EVERY_JOB` every job submits before its `done()` runs, so this never triggers; with `BBHOST_SUBMIT_EVERY_JOB=0` the plan's code could wait 250 ms for its own unsubmitted work and count it as gave-up. Review Focus 4 asks for exactly this. Cost if wrong: a flip with no submission since flip N-k is not bounded (it has nothing of its own to run ahead with). New test case `handed_over_only` (fails against the plan's `push`: `FAIL: handed_over_only: no submission since flip N-1`).
2. **Task 9: a copy the live share refuses is taken off the rules' counts.** Upstream takes such a copy off its own count (`if (pg.window_uploads) --pg.window_uploads;`, "Not a copy toward 'keeps being rewritten' either"). The plan's policy state kept it, so under `SHADOW_HOT=frames` a page refused three times (the refusal is retried every 250 ms, ~15 flips) within 60 flips would go to host memory for good as "rewritten". `shadow_policy_copy_locked` keeps the page's state from before its decision and whether the decision made the page's entry; `shadow_policy_copy_refused_locked` (one line at the refusal, beside upstream's own decrement) puts the state back, or removes the entry it made (review item 4: a restored empty entry made a never-copied page look copied to the ping-pong rule). Cost if wrong: none for the default; one more hooked line.
3. **Task 9: ping-pong marks only pages the rules have seen copied.** A GPU write over a page never copied is not the start of a bounce ("re-copied after a GPU write"), and `shadow_written_locked` no longer adds a map entry for every GPU-written page. Likewise the retry looks pages up without adding them. Cost if wrong: a page's first copy after a GPU write is not counted as a bounce.
4. **Task 9: the audit counts mirror binds too and reports from every bind.** The plan's line came only from a host-memory bind, so a title window with none after flip 16 would print nothing and `sc-t09-audit`'s expected line could be missing. The line is now `bbg shadow audit: host binds/flip by reason: not-mirrorable <n> no-mirror <n> dynamic <n> gpu-written <n> no-room <n> no-import <n> unwatchable <n> rewritten <n>; mirror binds/flip <n>; top ranges: 0x<va>=<n>/flip(<reason>) ...` (the `; mirror binds/flip <n>` part is new; the rest is the plan's). Two more audit lines in `shadow_locate_locked`, at its two mirror-bind returns. Cost if wrong: Task 22's `SWITCHES.md` documents the extra field.
5. **Task 12: a draw of a pipeline whose variant is on a worker is either skipped or builds the variant in place - never the fallback.** The plan's second skip covered only the skip; a draw of that pipeline the rules do not let skip (indirect, 12 or fewer vertices, a query draw, another pass, the 45 flips spent) went on to translate and compile the *fallback* on the command processor - a pipeline upstream never builds for a draw whose constant buffers all bind (upstream with `BBHOST_LEAN_FIRST` builds the no-fallback variant in place) - and when the fallback binds differently it is dropped (`render: ... one draw skipped`), which could lose exactly a draw the rules protect (review item 2). Now such a draw builds the no-fallback variant in place under a key of its own (`fnv1a("cb-lean-cp", 10, key)`: the worker's entry is the worker's until handed over), takes it as the pipeline's variant (`apply_lean_builds_locked` replaces it with the worker's once handed over), and binds it if it fits, as upstream's lean block does; one that does not fit falls through to upstream's path. The hook also needs the draw to be lean-eligible (`g.cb_lean && any_buffers && all_bound && !pl.tess_native`, the lean block's own condition): a draw lacking a constant-buffer binding binds the fallback whatever the worker builds, as upstream. Cost if wrong: such a draw costs the in-place compile upstream would have paid at the pipeline's first draw, and the worker's compile of the same variant is wasted.
6. **Task 12: "inside an occlusion-query interval" is "the next draw would get a query".** bbhost's interval (`g_iv.open`) is set at the first dump of the session and never closed, so the plan's `g_on && g_iv.open` protected every draw once the game had dumped once - ~180 dumps a frame in the world - and the switch never skipped where it is meant to (review item 1; the title, under 300 draws, hid it). `occlusion_interval_open_locked()` is now `g_on && g_iv.open && g_iv.queries < g_iv.cap`: the draws after a dump, up to its cap, which are the ones that get queries (`occlusion_draw_locked`). Any other draw is unmeasured whether it is drawn or skipped, and an unmeasured interval reads visible, never occluded (occlusion.cpp's design note). Cost if wrong: none for the default.
7. **Task 12: "stores" also counts the geometry stage and GDS counters** (`m.gds_counters`, ds_append/ds_consume through GDS), beside the plan's store buffers, untraced stores, storage images and written buffers. A draw with any of them is never skipped (Review Focus 3). Cost if wrong: none (fewer skips).
8. **Task 12: the `bbg async skip:` line also reports from the pass notes** (every drawn draw while the switch is on), so it appears at flip 16 even when nothing compiles (a warm cache), as Step 6's switch check expects. Cost if wrong: none.
9. **Task 12: `PassHistory::note` ignores a flip older than the entry's** (`f <= e.at`), where the plan's shift by `f - e.at` would underflow. Flips only move on, so this changes nothing in practice.
10. **Task 14: `bbg::window_ahead` is `no_stack_protector, always_inline`**, like the fault handler's own helpers (`entry_of`, `protect_pages`): the handler runs on guest threads, whose FS is the guest's. The rest is the plan's (one relaxed atomic load in the handler, the window set from a perf hook).
11. **Task 14: on Linux, with the window on, a failed range call stamps the pages it covered.** An entry never goes back to 0 (an unmapped watched range is forgotten, which stamps it), so the window reaches over a hole a watched mapping left; Linux's `mprotect` then makes the mappings before the hole writable and fails, the handler falls back to the faulting page alone, and those pages stay writable with clean entries: their later writes are never seen (review item 3; reproduced, below). Now, when the window is on and the range call fails, the pages it covered count as written too (an extra upload for any that stayed protected). Only where `protect_pages` is `mprotect`: `VirtualProtect` (Windows, and wine, which checks the range against one view first) changes nothing when it fails, and stamping there would only invalidate the next view's watched pages whenever a window straddles a view boundary (the reviewer's re-check; `#if !defined(_WIN32)`). Upstream's own look-ahead has the same gap over at most 16 pages behind a streaming run; with the switch unset the handler is upstream's, so that is left as it is (see "Open concerns"). Cost if wrong: extra uploads after a failed range call.
12. **Task 15: the value reads as every bbgoated switch's** (set, and not `0`, `off`, `false` or `no`: `bbg::perf_on`'s rule and the Global Constraints'), through a pure `bbg::pcores_wanted` in `pcores_core.h`, rather than the plan's `p[0] == '1'`: `src/core` stays free of the perf glue, and `BBHOST_BBG_PCORES=on` (which the `bbg perf:` line would list) no longer does nothing (review item 6). The CPU-set walk also stops at an entry that would end past the buffer. Cost if wrong: none.
13. **Tests extended beyond the plan's** (all in the plan's style): Task 8 `handed_over_only`; Task 12 `indirect_never`, `tessellated_never`, the 12/13-vertex boundary, a pass not drawn every frame, a budget that stays expired and per-pipeline budgets; Task 14 the 16 KiB floor, exactly 1 MiB and a 256-page window; Task 15 three efficiency classes and the switch's values.
14. **Commit trailers**: README rule 5's `Claude-Session: .../session_01QamtaLwymAJnp4iySL9WYK` line only, author and committer KevY007, as the earlier cloud runs did; this run's own session is named at the top of this report.
15. **The branch is `cloud/p3-b-2`**, because another run of this task holds `cloud/p3-b` (section "Another run on `cloud/p3-b`"): never force-pushed over another session's branch; the hand-back protocol allows another name when it is said. Cost if wrong: the controller fetches one more branch.

## Review

A separate agent read the five commits against the plan (Review Focus 3 and 4, switch-off identity, the handler rules), read-only, then checked the fixes the same way: fixes 1, 2, 4 and 6 correct; fix 3 closed the Linux missed write but stamped pages on Windows after a `VirtualProtect` failure that changes nothing, so the stamping is now Linux-only (ruling 11).

| # | Finding | Severity | Done |
|---|---|---|---|
| 1 | Task 12: `g_iv.open` is never cleared, so the occlusion rule protected every draw after the session's first dump and the skip never fired in the world | real (switch inert) | ruling 6 |
| 2 | Task 12: a draw of a skipped pipeline that may not be skipped compiled the fallback on the command processor (a pipeline upstream never builds there) and could be dropped when the fallback binds differently | real | ruling 5 |
| 3 | Task 14: a window across a once-watched hole left pages writable and unstamped on Linux: missed writes | real | ruling 11 (Linux only, after the re-check) |
| 4 | Task 9: a refused first copy left an empty policy entry, so the page's first real copy counted as a bounce | minor | ruling 2 |
| 5 | Task 9: the `upload_cmd_locked` failure return to host memory has no audit reason | diagnostic only | not changed (open concerns) |
| 6 | Task 15: `on`/`true` did nothing while the `bbg perf:` line listed them; the CPU-set walk had no end check | minor | ruling 12 |
| 7 | Task 14 x Task 9: window stamps count as rewrites in the buffer shadow | not a bug | open concerns |

It found nothing wrong with switch-off identity (every hook returns on a cached static or flag; `shadow_locate_locked` matches upstream exactly with the shadow switches unset), with Task 8's deadlock freedom (below), with Task 9's retry, floor and keys, or with Task 12's early returns (both sit where upstream's own `draw_failed` returns are; the published packet's descriptor writes apply with no draw; `g_recorded`, hazards, copy versions and the query come after them).

## Per task

### Task 8: run-ahead bounded by GPU completion (`BBHOST_BBG_FRAMES_AHEAD`), bd6ff44

- Files: `src/host/perf/core/frame_pacing_core.h` (`FrameNeeds`), `src/host/perf/frame_pacing.{h,cpp}` (`FramesAheadGuard`, every build: it uses only `host_gpu_*` calls `gpu_stub.cpp` also has), `tests/bbg_frame_pacing_test.cpp`; `src/hle/video.cpp`.
- Hook: `hle_video_finish_flip`, after `const std::uint64_t gpu_need = g_flip_after_gpu ? work : 0;`: `bbg::FramesAheadGuard frames_ahead(work, recorded);`. The destructor waits, after the flip was queued and shown, on every return path.
- Deadlock review (Review Focus 4): the caller is the job's `done` callback on the command-processor thread, run after the job's `host_gpu_submit` with no lock held (`gnm_exec.cpp` job loop; `hle_gnm_submit_flip`'s lambda; `hle_gnm_flip_done` passes `recorded=false`, so its guard does nothing); `g_vo_mu` is released before the destructor runs; the wait polls `host_gpu_submissions_completed()` (takes `g.mu` briefly, `vkGetFenceStatus` does not block, ~0 with no GPU or after a loss) every 100 us without holding `g.mu` or its own mutex, and gives up after 250 ms; with ruling 1 it targets only handed-over submissions, which bb-submit and the recorder finish on their own threads.
- Off: the constructor returns at `!ahead()`, the destructor at `!target_`.
- Log: `bbg frames ahead (<k>): waited <ms> ms over <n> flips (<n> waits), longest <ms> ms, <n> gave up` at flip 16 and every 300 flips (gave up is cumulative).
- RED (`cmake --build build-ON --target bbg_frame_pacing_test` with only the test added): `tests/bbg_frame_pacing_test.cpp:3:10: fatal error: 'host/perf/core/frame_pacing_core.h' file not found`. GREEN: `bbg_frame_pacing_test: ok`. Behavioural RED of ruling 1: the test against the plan's `push` prints `FAIL: handed_over_only: no submission since flip N-1` and exits 1.
- Local:
  ```powershell
  & $WS\scripts\switch-check.ps1 -Exe $WS\out\p3-b\bbhost.exe -Name sc-t08-k1 -Env @{BBHOST_BBG_FRAMES_AHEAD="1"} -Expect @('bbg frames ahead \(1\): waited', 'exit: flip \d+ reached')
  & $WS\scripts\switch-check.ps1 -Exe $WS\out\p3-b\bbhost.exe -Name sc-t08-k2-shallow -Env @{BBHOST_BBG_FRAMES_AHEAD="2"; BBHOST_GPU_SLOTS="8"; BBHOST_CP_BACKLOG="4"} -Expect @('bbg frames ahead \(2\)', '8 command buffers in flight at most')
  ```
  Expected: both pass, the title reaches flip 18, `0 gave up` (a non-zero count at the title means a wait never saw its submission complete: stop and debug).

### Task 9: bbport's memory-module rules in the buffer shadow, and the host-bind audit (`BBHOST_BBG_SHADOW_*`), fa1ccd8

- Files: `src/host/perf/core/shadow_policy_core.h` (`PagePolicy`, `frames_rule_copy`, `pingpong_copy`, `retry_due`), `src/host/perf/vk/shadow_policy.{h,cpp}`, `tests/bbg_shadow_policy_test.cpp`; `src/host/buffer_shadow.cpp`.
- Hooks in `buffer_shadow.cpp`: `mirror_room` (the floor, after the cap check); `go_dynamic` (last statement: when, and whether for being rewritten); `shadow_locate_locked`: the audit at each of its 12 returns to host memory (reasons as the plan lists them) and at its two mirror binds (ruling 4), the retry block before the `any_set(m->dyn, ...) || any_set(m->gpu, ...)` return, the pre-arm walk's `kMaxUploads` break under upstream's rule only, the hot rule in the copy loop (the plan's `rewritten` block), the refused-copy undo at the live-share refusal (ruling 2); `shadow_written_locked` (the ping-pong note in `if (by_gpu)`).
- Off: `shadow_policy_any()` is false, so `rewritten = ++pg.window_uploads > kMaxUploads` exactly as before; the retry, the `go_dynamic` note, the ping-pong note and the undo are skipped; the floor is 0; the audit lines test one cached bool.
- Logs: the audit line (ruling 4) and `bbg shadow policy: hot=<rule> retried <n> bounced-out <n> (pages tracked <n>)`, each at flip 16 and every 300 flips; the policy line only once a page is copied under a rule.
- RED: `tests/bbg_shadow_policy_test.cpp:3:10: fatal error: 'host/perf/core/shadow_policy_core.h' file not found`. GREEN: `bbg_shadow_policy_test: ok`.
- Local:
  ```powershell
  $pkg = "$WS\out\p3-b"; $base = "$WS\docs\perf\validation-base-2026-10-12.txt"
  & $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t09-audit -Env @{BBHOST_BBG_SHADOW_AUDIT="1"} -Expect @('bbg shadow audit: host binds/flip by reason:')
  & $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t09-frames -F12 -Env @{BBHOST_BBG_SHADOW_HOT="frames"; BBHOST_BBG_SHADOW_PINGPONG="2"; BBHOST_BBG_SHADOW_RETRY_S="10"} -Expect @('exit: flip \d+ reached') -Validate -Baseline $base
  & $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t09-never -Env @{BBHOST_BBG_SHADOW_HOT="never"; BBHOST_BBG_SHADOW_FLOOR_MB="2048"} -Expect @('exit: flip \d+ reached')
  ```
  Expected: all pass; the audit line at flip 16 (with its `mirror binds/flip` field); the policy line may be absent at the title. World recipe and A/B: the plan's Task 9 Step 6 and A/B.

### Task 12: background pipeline builds with bbport's skip rules (`BBHOST_BBG_ASYNC_SKIP`), 4a272fc

- Files: `src/host/perf/core/async_skip_core.h` (`SkipDraw`, `PassHistory`, `skip_eligible`, `SkipBudget`), `src/host/perf/vk/async_skip.{h,cpp}`, `tests/bbg_async_skip_test.cpp`; `src/host/render.cpp`, `src/host/occlusion.cpp`, `src/host/gpu_internal.h`.
- Hooks: `async_skip_draw_of` just above `static bool draw_impl(const GpuDraw& d) {` (the definition); in `draw_impl` the lean block's middle branch (`else if (g_async_pipelines && !g_dump_spirv && !g_token_user_data_check && async_skip_on() && async_skip_draw_locked(...))`: queue the variant on a worker, `return true`), the block after the lean block (skip, or build the variant in place: ruling 5), and `async_skip_note_pass_locked(pass_key(s))` before `begin_pass(s, pass_key(s), sample_hazard);`. `occlusion_interval_open_locked()` (ruling 6) after `occlusion_draw_locked`, declared after its declaration in `gpu_internal.h`.
- Side effects (Review Focus 3): never skipped are draws that would get an occlusion query (ruling 6), with 12 or fewer vertices or indices (`GpuDraw::index_count`, the packet's count), indirect, tessellated (`t_tess`), or whose VS, PS or GS stores (store buffers, untraced stores, storage images, written buffers, GDS counters: ruling 7); also loading screens and flips after one with under 300 draws, and passes not drawn in each of the last 8 flips. A pipeline is skipped for 45 flips at most from its first skip (`SkipBudget`, keyed by the pipeline's key); then its draws build the variant in place (ruling 5). Both early returns come before `begin_pass` and the draw capture: the draw's descriptor writes went into sets that are never bound (`DrawCmds::take_sets` writes sets, it pushes nothing), the packet publishes on scope exit, and pending depth clears were already recorded earlier in `draw_impl`, as on upstream's own early returns (`one draw skipped`, `fully scissored out`).
- Off: `async_skip_on()` is false, so the lean block takes its upstream branches and the two added blocks do nothing.
- Log: `bbg async skip: <n> draws/flip skipped while their pipeline compiled, <n> built in place` at flip 16 and every 300 flips (ruling 8).
- RED: `tests/bbg_async_skip_test.cpp:3:10: fatal error: 'host/perf/core/async_skip_core.h' file not found`. GREEN: `bbg_async_skip_test: ok` (cases `pass_every_frame`, `occlusion_never`, `fullscreen_never`, `budget_expires`, plus ruling 13's). Rulings 5 and 6 are in `render.cpp` and `occlusion.cpp` glue, which has no unit test: they need the cold-cache world check.
- Local (cold, then the switch check):
  ```powershell
  & $WS\scripts\smoke.ps1 -Exe $WS\out\p3-b\bbhost.exe -Name sc-t12-cold -Env @{BBHOST_BBG_ASYNC_SKIP="1"} -Seconds 420
  Select-String -Path $WS\sandbox\sc-t12-cold\logs\run.log -Pattern 'bbg async skip:'
  & $WS\scripts\switch-check.ps1 -Exe $WS\out\p3-b\bbhost.exe -Name sc-t12 -F12 -Env @{BBHOST_BBG_ASYNC_SKIP="1"} -Expect @('bbg async skip:') -Validate -Baseline $WS\docs\perf\validation-base-2026-10-12.txt
  ```
  Expected: `smoke: ok` and the line present; 0 skipped at the title (under 300 draws a frame: the rule working); the switch check passes. World check on a cold cache: the plan's Task 12 Step 7; there, also read the exit report's `render: fallback pipelines laid out from their no-fallback stages <n>; translated later for a draw <n>` (B should not exceed A) and look for `one draw skipped` lines (none new in B).

### Task 14: write-watch fault window (`BBHOST_BBG_FAULT_WINDOW_KB`), 637a65e

- Files: `src/host/perf/core/fault_window_core.h` (`window_ahead`, `window_pages_for_kb`), `src/host/perf/fault_window.cpp` (the perf hook), `tests/bbg_fault_window_test.cpp`; `src/core/write_watch.{h,cpp}`.
- Hooks: `write_watch_on_fault`, after the `if (g_ahead_on) { ... }` look-ahead and before `protect_pages`: the armed pages to the window's end go too; in the failed-range path, on Linux with the window on, the pages the range covered are stamped (ruling 11); `g_window_pages` (a constant-initialised `std::atomic<std::uint32_t>`) beside `g_ahead_on`; `write_watch_set_window_pages` before `write_watch_stats`. The handler reads one relaxed atomic and calls only inlined helpers (ruling 10): no lock, no thread-local, no switch.
- Off: `g_window_pages` is 0, the block is skipped and the failed-range path is upstream's. In BBGOATED=OFF builds `perf_init` (and so this hook) never runs, so the window stays 0 whatever the environment says.
- Log: `bbg: a write-watch fault releases the armed pages to the end of its <n> KiB window (BBHOST_BBG_FAULT_WINDOW_KB)`, once, from `perf_init`.
- RED: `tests/bbg_fault_window_test.cpp:2:10: fatal error: 'host/perf/core/fault_window_core.h' file not found`. GREEN: `bbg_fault_window_test: ok`. `write_watch_test` (upstream, the real fault path) still passes.
- Also through the real SIGSEGV path on Linux, with scratch programs built like `write_watch_test` (not committed; `clang++ -std=c++20 -O2 -I src <check>.cpp src/core/write_watch.cpp`):
  - `ww_window_check`: window 0 releases the faulting page only; a 16-page window releases from the fault to the window's end, writes inside it fault no more, the next window faults again; a never-armed page stops the release. `ww_window_check: ok`.
  - `ww_hole_check` (ruling 11): 16 watched pages, pages 8-9 unmapped and forgotten, the rest armed again, a 16-page window, a write to page 2 then to page 5. Before the fix: `FAIL 45 five` (page 5's write unseen); after: `ww_hole_check: ok`.
- Local:
  ```powershell
  & $WS\scripts\switch-check.ps1 -Exe $WS\out\p3-b\bbhost.exe -Name sc-t14 -Env @{BBHOST_BBG_FAULT_WINDOW_KB="64"} -Expect @('bbg: a write-watch fault releases the armed pages to the end of its 64 KiB window')
  ```
  Expected: it passes; compare the exit report's write-watch fault count (and its look-ahead pages) with `sc-base`'s.

### Task 15: host GPU-feeding threads on performance cores (`BBHOST_BBG_PCORES`), 1c7c04e

- Files: `src/host/perf/core/pcores_core.h` (`pcores_wanted`, `CpuSet`, `performance_cpu_sets`), `tests/bbg_pcores_test.cpp`; `src/core/host_clock.cpp`.
- Hook: `host_thread_set_class`'s Windows branch, first statement: `if (bbg::pcores_wanted(std::getenv("BBHOST_BBG_PCORES"))) pcores_for_this_thread(who);` (ruling 12); `pcores_for_this_thread` in its own `#if defined(_WIN32)` block above it. Upstream PR #118's `host_thread_set_name` calls beside the callers (`gpu.cpp` bb-submit, `recorder.cpp` bb-record, `window.cpp` bb-present, `video.cpp` bb-vblank, `gnm_exec.cpp` the command processors) are untouched.
- Off: the test is false and nothing else runs. Linux: the hook is inside the Windows branch.
- RED: `tests/bbg_pcores_test.cpp:2:10: fatal error: 'host/perf/core/pcores_core.h' file not found`; then, for ruling 12's cases, `tests/bbg_pcores_test.cpp:27:17: error: no member named 'pcores_wanted' in namespace 'bbg'`. GREEN: `bbg_pcores_test: ok`.
- The Windows branch compiles with clang (`--target=x86_64-w64-mingw32`) and with x86_64-w64-mingw32-g++ 13 (Ubuntu's mingw-w64 11.0.1 headers have `SYSTEM_CPU_SET_INFORMATION`, `CpuSetInformation` and `EfficiencyClass`), `-Wall -Wextra`, no diagnostics. Arch's mingw-w64 is newer, so the local cross build should agree.
- Local:
  ```powershell
  & $WS\scripts\switch-check.ps1 -Exe $WS\out\p3-b\bbhost.exe -Name sc-t15 -Env @{BBHOST_BBG_PCORES="1"} -Expect @('timing: bb-submit runs on the 8 performance-core CPU sets of 20', 'timing: bb-record runs on the 8 performance-core')
  ```
  Expected: it passes; the 265KF reports 8 of 20.

## Tests and checks (commands and summary lines)

- Prefix: upstream `tools/ci_build.sh`'s prefix steps from git clones (Vulkan-Headers v1.4.350, SDL release-3.4.16, FFmpeg n7.1.1), the script itself untouched; bbhost configured as the script does (`-G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_PREFIX_PATH=<prefix> -DVulkan_INCLUDE_DIR=<prefix>/include -fuse-ld=lld`) plus ccache and `-DBBGOATED=ON|OFF`; Ubuntu 24.04.5, clang 18.1.3, cmake 3.28.3, ninja 1.11.1, 4 CPUs.
- Base before any change (ebb97dc, ON): `100% tests passed, 0 tests failed out of 60`.
- `cloud/p3-b-2` ON (`cmake --build build-ON && ctest`): `100% tests passed, 0 tests failed out of 66` (25 skipped).
- `cloud/p3-b-2` OFF: `100% tests passed, 0 tests failed out of 64` (25 skipped; `bbgoated_config_test` and `redzone_audit_test` are ON-only).
- The six `bbg_*` tests also built with GCC 13 under `-fsanitize=address,undefined -fno-sanitize-recover=all`: all print `ok`.
- Each task commit builds on its own (ON, a separate local clone, the commits in order): bd6ff44, fa1ccd8, 4a272fc, 637a65e and 1c7c04e each `build ok`, with `ctest -R 'bbg_|write_watch'` at 100% (3, 4, 5, 6 and 7 tests).
- Warnings: none on any added line (ON and OFF build logs); the Windows-target syntax check of every new `.cpp` and every hooked file (`clang++ --target=x86_64-w64-mingw32 -std=c++20 -fsyntax-only -Wall -Wextra -DBBHOST_BBGOATED=1`) shows only upstream's own three warnings (`video.cpp:598`, `render.cpp:7131`, `render.cpp:7132`).
- Switch-off identity on the Linux binaries, run in a scratch sandbox (empty config, `BBHOST_NP_SIGNED_OUT=1`, headless) without game files, so they stop after the configuration with "does not say where the game is" (exit 2), after `perf_init`: with no `BBHOST_BBG_*` set, no `[bbhost] bbg` line (ON and OFF); with `BBHOST_BBG_FAULT_WINDOW_KB=64 BBHOST_BBG_PERF=FRAMES_AHEAD=0,nosuch`, the ON binary:
  ```
  [bbhost] bbg perf: no preset named "nosuch"
  [bbhost] bbg perf: BBHOST_BBG_FRAMES_AHEAD=0 BBHOST_BBG_FAULT_WINDOW_KB=64
  [bbhost] bbg: a write-watch fault releases the armed pages to the end of its 64 KiB window (BBHOST_BBG_FAULT_WINDOW_KB)
  ```
  and with `[perf]` `fault_window_kb = 100` and `pcores = "1"` in the `--config` file: `bbg perf: BBHOST_BBG_FAULT_WINDOW_KB=100 BBHOST_BBG_PCORES=1`, then `... to the end of its 128 KiB window ...`. The OFF binary logs no `bbg` line in either case (no `perf_init` there).

## What still needs local verification

1. Merge with `cloud/p3-a` (P3A's Tasks 4, 5, 6 touch other functions). A trial merge of `cloud/p3-b-2` with `cloud/p3-a` at ff0f44f (throwaway local branch, not pushed) is clean and the merged tree passes ON `100% tests passed, 0 tests failed out of 70`. Task 6 and Task 12 both touch `draw_impl` near `begin_pass`: Task 6 the `if (g_lazy_barriers) {` line, Task 12 one line just above `begin_pass` (under the lazy-barrier block) and, ~290 lines higher, the lean block's middle branch and the block after it, plus the helper above `draw_impl`. A conflict there keeps both.
2. The Windows build: `powershell -NoProfile -File $WS\scripts\build.ps1 -CMakeArgs "-DBBGOATED=ON" -Out $WS\out\p3-b` (the commands above use `$WS\out\p3-b`; any out directory works), then `powershell -NoProfile -File $WS\scripts\check-online-untouched.ps1 -Branch <the merged branch>` and the WSL ctest (`scripts/linux-test.sh -DBBGOATED=ON`).
3. Switch-off identity in game: a smoke with no switch set, whose log has zero `bbg ` lines (Review Focus 6; Task 24 step 1 later).
4. Each task's GPU steps above (switch checks, the validation runs with `-Validate`, the F12 checks), then the plan's world recipes and A/B passes when the PC is free. Task 12's rulings 5 and 6 can only be seen working there (the title has under 300 draws a frame): in the cold-cache world check, B's `bbg async skip:` line should show skips, and no draw should go missing beyond the first seconds of a place.

## Open concerns

- **On Windows a window that straddles a view boundary falls back to the faulting page alone** (`VirtualProtect` fails across views, as upstream's look-ahead does: `across two mappings (or views): this page alone`). With direct-memory views 64 KiB-aligned, windows above 64 KiB lose part of their effect at view edges; the fault-count A/B shows how much.
- **Upstream's write-watch look-ahead has the gap ruling 11 closes for the window**: with `BBHOST_WATCH_AHEAD` on (the default) and the window off, a look-ahead over a once-watched hole (at most 16 pages, behind a run written a moment ago) can leave pages writable and clean on Linux after `mprotect` fails part-way. Not changed here (switch-off identity); worth reporting upstream.
- **Task 8's 250 ms bounds the polling loop, not wall time**: one `host_gpu_submissions_completed()` call can wait for `g.mu` while another thread holds it (an in-place compile on another queue, a fence wait). Nothing that holds `g.mu` waits on queue 0's command processor, and upstream already takes `g.mu` twice on that thread, so there is no new lock order.
- **Task 14 x Task 9**: every fault stamps up to the window's end, and the buffer shadow counts a stamped page's next copy toward "keeps being rewritten" (more than 4 copies in 8 s sends a page to host memory for good), so a large window next to CPU-written rings may send static vertex pages to host memory. Read the shadow's dynamic count (and `SHADOW_AUDIT`'s `dynamic`/`rewritten`) beside the fault window's A/B.
- **Task 12, the in-place build of ruling 5 makes the worker's compile of the same variant wasted work** (CPU on a worker thread); the command processor pays what upstream pays at the pipeline's first draw, no more.
- **Task 12's skipped draws are not counted in `g.draws`** (bumped at the end of `draw_impl`), which the 300-draw rule reads; in the world thousands of draws a frame keep it far above 300, and the line's skip count shows how many there were.
- **Task 9, two diagnostic gaps**: the copy loop's `upload_cmd_locked` failure (a Vulkan command-buffer allocation failure) returns to host memory with no audit reason, and a copy that fails there still counts toward the frames rule, as it does toward upstream's window.
- **Task 8's line is due only from a flip that had a target** (a recorded flip with a GPU need and a submission since flip N-k); at the title every flip has one, so it appears at flip 16 as Step 5 expects.
- In BBGOATED=OFF builds the switch core's `perf_init` never runs (P3A's `main.cpp` hook is inside `#if defined(BBHOST_BBGOATED)`), so presets, `[perf]` and Task 14's hook do not apply there; the four lazily read switches (Tasks 8, 9, 12, 15) still work from the environment. Phase 3's checks and A/B use ON builds.
