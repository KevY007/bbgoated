# Task P3B report - Phase 3 wave-1 switches, second half (Tasks 8, 9, 12, 14, 15)

Cloud run of 2026-10-12, branch `cloud/p3-b` of `KevY007/bbgoated`. Brief: `docs/cloud-tasks/task-P3B.md` (workspace); specification: the Phase 3 plan's Tasks 8, 9, 12, 14 and 15 (`docs/superpowers/plans/2026-10-12-bbgoated-phase3-perf-switches.md`), with the controller's rulings on top.

## Summary

- `cloud/p3-b` starts at **be5ef07**, P3A's `bbg/perf: switch core ...` commit (ebb97dc + Task 3), not at P3A's later commits. Five task commits follow, one per task, with the plan's subjects, then this report.
- Every switch is off by default. With every `BBHOST_BBG_*` unset each hook is one cached test that falls through to upstream's code, and no `bbg ` line is logged. A no-game-files run of the built binary showed zero `bbg` lines when unset (see Verification).
- Linux, fresh builds of the final code head: BBGOATED=ON through the unmodified `tools/ci_build.sh`: `100% tests passed, 0 tests failed out of 66` (25 skipped for want of game files). BBGOATED=OFF: `100% tests passed, 0 tests failed out of 64`. Warnings: 343 in each, the same file, message and count set as ebb97dc's.
- Each task's test went RED for the right reason and then GREEN; evidence is in each task's section. Review Focus 3 (`bbg_async_skip_test`: `occlusion_never`, `fullscreen_never`, `pass_every_frame`, `budget_expires`, plus indirect, tessellated and stores) and 4 (`bbg_frame_pacing_test`, including the guard's 250 ms give-up) are covered.
- `check-online-untouched.ps1 -Branch cloud/p3-b` (PowerShell 7.4.6 here): `online paths untouched since 566184a (1 upstream cherry-picks allowed)`. No file under a protected path changed; `CMakeLists.txt` and `cmake/bbgoated.cmake` were not edited (Task 3's globs pick the new files up); `git diff --summary be5ef07..cloud/p3-b` shows no mode change.
- A trial merge with P3A's branch as it stood at the end of this run (`cloud/p3-a` = 973d6fb, Tasks 3-6) is clean, and the merged tree builds with BBGOATED=ON and passes `100% tests passed, 0 tests failed out of 70` (343 warnings). It was done on a throwaway local branch, deleted afterwards and never pushed.

## Commits

| SHA | Subject |
|---|---|
| be5ef07 | (base, P3A) bbg/perf: switch core (BBHOST_BBG_* environment, BBHOST_BBG_PERF and [perf], perf hooks) |
| 3c2c5b9 | bbg/perf: the command processor bounded to k frames ahead of GPU completion (BBHOST_BBG_FRAMES_AHEAD) |
| 1fa1785 | bbg/perf: buffer shadow hot rules from bbport's memory module, retry, ping-pong, floor and a host-bind audit (BBHOST_BBG_SHADOW_*) |
| ca18add | bbg/perf: draws whose pipeline compiles on a worker are skipped under bbport's rules (BBHOST_BBG_ASYNC_SKIP=1) |
| 4afe1b7 | bbg/perf: write-watch faults release the armed pages to the end of a fixed window (BBHOST_BBG_FAULT_WINDOW_KB) |
| ad56cdf | bbg/perf: the host's GPU-feeding threads on performance cores through CPU sets (BBHOST_BBG_PCORES=1) |
| (last) | bbg/docs: task P3B report |

Author `KevY007 <8596387+KevY007@users.noreply.github.com>`; every message ends with the README's `Claude-Session:` line.

## Base and timeline

- The brief's poll (`git ls-remote origin refs/heads/cloud/p3-a`, then the branch's log for a `bbg/perf: switch core` subject, every 2 minutes in a Monitor loop) started at 12:21 UTC. The branch appeared at be5ef07 and the commit was found at about 12:35 UTC, well inside the 90 minutes, so Task 3 was not redone here.
- be5ef07's parent is ebb97dc (`cloud/task-R-rebase`). Every anchor was re-located on it by function name and quoted line; all were there as the plan quotes them (`hle_video_finish_flip`'s `gpu_need` line, `shadow_locate_locked`'s returns, `go_dynamic`, `mirror_room`, `shadow_written_locked`, `draw_impl`'s lean-variant block and `begin_pass(s, pass_key(s), sample_hazard);`, `occlusion_draw_locked`, `write_watch_on_fault`'s look-ahead, `host_thread_set_class`).
- Upstream's `host_thread_set_name` (PR #118) is in `core/portable.cpp`; the thread starts call it and then `host_thread_set_class`. Task 15 changes only `host_thread_set_class`'s body (and adds a helper above it), so both stay as they are.

## Task 8: run-ahead bounded by GPU completion (`BBHOST_BBG_FRAMES_AHEAD`)

Files:
- New: `src/host/perf/core/frame_pacing_core.h` (`bbg::FrameNeeds`), `src/host/perf/frame_pacing.h`, `src/host/perf/frame_pacing.cpp` (`bbg::FramesAheadGuard`, every build), `tests/bbg_frame_pacing_test.cpp`.
- Hook: `src/hle/video.cpp` `hle_video_finish_flip`, one line after `const std::uint64_t gpu_need = g_flip_after_gpu ? work : 0;` (`bbg::FramesAheadGuard frames_ahead(work, recorded);`), plus the include.

Behaviour: with k = 1-3, the guard records the flip's `host_gpu_work_needs()` and, in its destructor (after the flip was queued and shown), polls `host_gpu_submissions_completed()` every 100 us until flip N-k's need has run, giving up after 250 ms. "bbg frames ahead (k): waited <ms> ms over <n> flips (<n> waits), longest <ms> ms, <n> gave up" at flip 16 and every 300 flips. Off: the constructor returns at `!ahead()`, the destructor at `!target_`.

Why it cannot deadlock (Review Focus 4):
- The guard runs in the job's `done` callback on the command-processor thread (`gnm_exec.cpp` job loop), after `host_gpu_submit()`, and holds no lock while it waits. `host_gpu_submissions_completed` takes `g.mu` only for each poll, and counts only fences bb-submit has put on the queue.
- It waits only for submissions already handed over. Flip N-k's need is below flip N's own whenever a submission went in between, and then every submission it names has been flushed. When no submission went in between (Ruling 1), it waits for nothing.
- A GPU that never gets there is given up on after 250 ms; a lost device makes the completion count ~0 at once.
- Game-thread flip requests (`recorded` false) and flips with no GPU work never wait.

TDD:
- RED 1, `cmake --build /home/user/ci/build --target bbg_frame_pacing_test`: `tests/bbg_frame_pacing_test.cpp:7:10: fatal error: 'host/perf/frame_pacing.cpp' file not found`.
- RED 2, the plan's `FrameNeeds` verbatim with the glue: `FAIL: no submission since flip N-1: its need may be the recording under way, never waited for`, exit 1. Every other check passed, the give-up timing included.
- GREEN with the clamp: `bbg_frame_pacing_test: ok` (0.25 s: the stuck case is the 250 ms give-up).

Local verification (the plan's Step 5; build `-Out $WS\out\p3-t08`):
```powershell
& $WS\scripts\switch-check.ps1 -Exe $WS\out\p3-t08\bbhost.exe -Name sc-t08-k1 -Env @{BBHOST_BBG_FRAMES_AHEAD="1"} -Expect @('bbg frames ahead \(1\): waited', 'exit: flip \d+ reached')
& $WS\scripts\switch-check.ps1 -Exe $WS\out\p3-t08\bbhost.exe -Name sc-t08-k2-shallow -Env @{BBHOST_BBG_FRAMES_AHEAD="2"; BBHOST_GPU_SLOTS="8"; BBHOST_CP_BACKLOG="4"} -Expect @('bbg frames ahead \(2\)', '8 command buffers in flight at most')
```
Expected: both pass and the title reaches flip 18. The startup line is `bbg perf: BBHOST_BBG_FRAMES_AHEAD=1`. `gave up` stays 0: a non-zero value at the title means a wait never saw its submission complete, so stop and debug. The A/B recipes are in the plan (p95/p99 and hitches first).

## Task 9: bbport's memory-module rules in the buffer shadow, and the host-bind audit (`BBHOST_BBG_SHADOW_*`)

Files:
- New: `src/host/perf/core/shadow_policy_core.h` (`frames_rule_copy`, `pingpong_copy`, `retry_due`), `src/host/perf/vk/shadow_policy.h`, `src/host/perf/vk/shadow_policy.cpp` (Vulkan builds), `tests/bbg_shadow_policy_test.cpp`.
- Hooks in `src/host/buffer_shadow.cpp`:
  - `shadow_locate_locked`: an audit call at every return that leaves the bind in host memory, 12 in all (not-mirrorable, no-mirror, dynamic or gpu-written, no-room, no-import, then in the copy loop dynamic, gpu-written, rewritten, unwatchable, no upload command buffer (Ruling 3), the live-share refusal and the no-room `go_dynamic`). Also the retry block before the dyn/gpu early return, the hot rule replacing `if (++pg.window_uploads > kMaxUploads) {`, and the pre-arm walk's `kMaxUploads` break taken only under upstream's rule.
  - `go_dynamic`: the policy note (when, and whether for being rewritten), last statement.
  - `mirror_room`: the floor, after the cap test.
  - `shadow_written_locked`: the ping-pong note inside `if (by_gpu) {`.

Behaviour: `SHADOW_HOT=frames` (copied in 3 of the last 60 flips sends a page to host memory) or `never` (re-copy every time), `SHADOW_PINGPONG=n`, `SHADOW_RETRY_S=s`, `SHADOW_FLOOR_MB=n`, `SHADOW_AUDIT=1`, as the plan writes them. Lines:
- `bbg shadow audit: host binds/flip by reason: not-mirrorable <n> no-mirror <n> dynamic <n> gpu-written <n> no-room <n> no-import <n> unwatchable <n> rewritten <n>; top ranges: 0x<va>=<n>/flip(<reason>) ...`;
- `bbg shadow policy: hot=<rule> retried <n> bounced-out <n> (pages tracked <n>)`.

Off: `shadow_policy_any()`, `shadow_retry_ms()`, `shadow_floor_bytes()` and `shadow_audit_on()` are cached zeros, so `rewritten = ++pg.window_uploads > kMaxUploads` exactly as before, and the pre-arm walk breaks where it did.

TDD: RED `fatal error: 'host/perf/core/shadow_policy_core.h' file not found`; GREEN `bbg_shadow_policy_test: ok`. Two window-boundary cases were added (flips 0, 30 and 59 are three of the last 60; at flip 60 the copy at flip 0 is out), and one for retry with the switch unset.

Local verification (the plan's Step 5; build `-Out $WS\out\p3-t09`):
```powershell
$pkg = "$WS\out\p3-t09"; $base = "$WS\docs\perf\validation-base-2026-10-12.txt"
& $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t09-audit -Env @{BBHOST_BBG_SHADOW_AUDIT="1"} -Expect @('bbg shadow audit: host binds/flip by reason:')
& $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t09-frames -F12 -Env @{BBHOST_BBG_SHADOW_HOT="frames"; BBHOST_BBG_SHADOW_PINGPONG="2"; BBHOST_BBG_SHADOW_RETRY_S="10"} -Expect @('exit: flip \d+ reached') -Validate -Baseline $base
& $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t09-never -Env @{BBHOST_BBG_SHADOW_HOT="never"; BBHOST_BBG_SHADOW_FLOOR_MB="2048"} -Expect @('exit: flip \d+ reached')
```
Expected: all pass. The audit line is written from a host bind at or after flip 16, so it needs one there (the title's CPU-written vertex rings should give that). The policy line appears only once a page is copied under a rule, so it may be absent at the title. The world diagnosis is the plan's Step 6 (`area-check.ps1 ... -Env @("BBHOST_BBG_SHADOW_AUDIT=1")`).

## Task 12: background pipeline builds with bbport's skip rules (`BBHOST_BBG_ASYNC_SKIP`)

Files:
- New: `src/host/perf/core/async_skip_core.h` (`PassHistory`, `skip_eligible`, `SkipBudget`), `src/host/perf/vk/async_skip.h`, `src/host/perf/vk/async_skip.cpp`, `tests/bbg_async_skip_test.cpp`.
- Hooks in `src/host/render.cpp`:
  - `async_skip_draw_of` just before the definition `static bool draw_impl(const GpuDraw& d) {`;
  - in `draw_impl`, the lean-variant block's middle branch, which queues the variant and returns true;
  - the check right after that block: the variant this draw would bind is still on a worker and the fallback was never created;
  - one line before `begin_pass(s, pass_key(s), sample_hazard);`, the pass history.
- Hooks elsewhere: `src/host/occlusion.cpp` `occlusion_interval_open_locked()` after `occlusion_draw_locked`, declared in `src/host/gpu_internal.h`.

Behaviour: a draw is skipped only when all of these hold:
- the last flip had 300 draws or more and is not a loading screen;
- its pass was drawn in each of the last 8 flips;
- it has more than 12 vertices;
- it is not indirect, not tessellated, stores to no buffer or image, and no occlusion query is measuring it;
- its pipeline is within its 45 flips from the first skip.

Line: `bbg async skip: <x> draws/flip skipped while their pipeline compiled, <y> built in place` at flip 16 and every 300 flips. Off: `async_skip_on()` is false, so the lean block takes upstream's branches and the two added checks do nothing.

Why a skip is safe (Review Focus 3):
- Draws with side effects are never skipped: stores, occlusion-measured, indirect and tessellated draws are excluded by the rules above, and so are full-screen passes (12 vertices or fewer).
- The skip returns come before `begin_pass` and before the draw's query, pipeline bind and `g_recorded` updates. The draw's packet is still published on scope exit (`DrawCmds::~DrawCmds` -> `publish`), and `replay_packet` applies its descriptor writes and draws only `if (p.draw)`. So a set the set cache or set reuse hands a later draw is written before that draw, and nothing else is recorded.
- A skipped draw inside an interval would not hide the object anyway: the occlusion module counts it unmeasured and the interval reads visible.

TDD: RED `fatal error: 'host/perf/core/async_skip_core.h' file not found`; GREEN `bbg_async_skip_test: ok`. The test has the plan's cases (`occlusion_never`, `fullscreen_never`, `pass_every_frame`, `budget_expires`) plus: indirect, tessellated, the 12-vertex boundary, a pass never drawn, a pass not drawn every frame, a budget per pipeline, and 5000 passes in one flip (recent ones kept).

Local verification (the plan's Step 6; build `-Out $WS\out\p3-t12`):
```powershell
& $WS\scripts\smoke.ps1 -Exe $WS\out\p3-t12\bbhost.exe -Name sc-t12-cold -Env @{BBHOST_BBG_ASYNC_SKIP="1"} -Seconds 420
Select-String -Path $WS\sandbox\sc-t12-cold\logs\run.log -Pattern 'bbg async skip:'
& $WS\scripts\switch-check.ps1 -Exe $WS\out\p3-t12\bbhost.exe -Name sc-t12 -F12 -Env @{BBHOST_BBG_ASYNC_SKIP="1"} -Expect @('bbg async skip:') -Validate -Baseline $WS\docs\perf\validation-base-2026-10-12.txt
```
Expected:
- `smoke: ok`, and the line `bbg async skip: 0.00 draws/flip skipped while their pipeline compiled, <y> built in place` is present (Ruling 6 puts it at flip 16 even with nothing compiling);
- at the title, under 300 draws a flip, nothing is skipped: the rule working;
- the switch check passes.

The world check is the plan's Step 7 (`area-check.ps1 -EnvA @("BBHOST_BBG_ASYNC_SKIP=0") -EnvB @("BBHOST_BBG_ASYNC_SKIP=1") -NoCaches ...`).

## Task 14: write-watch fault window (`BBHOST_BBG_FAULT_WINDOW_KB`)

Files:
- New: `src/host/perf/core/fault_window_core.h` (`window_ahead`, `window_pages_for_kb`), `src/host/perf/fault_window.cpp` (the perf hook that sets the window after presets and logs the line), `tests/bbg_fault_window_test.cpp`.
- Hooks: `src/core/write_watch.h` declares `write_watch_set_window_pages`. `src/core/write_watch.cpp` adds the include, `std::atomic<std::uint32_t> g_window_pages{0}` beside `g_ahead_on`, the window block in `write_watch_on_fault` right after the look-ahead and before `protect_pages`, and the setter at the end of the file.

Behaviour: 16-1024 KiB, rounded up to a power of two of 4 KiB pages. The first fault in a window also releases the armed pages up to the window's end, stopping at a page never armed (as the look-ahead does), on top of whatever the look-ahead released. The handler reads only the relaxed atomic. Line: `bbg: a write-watch fault releases the armed pages to the end of its <n> KiB window (BBHOST_BBG_FAULT_WINDOW_KB)`. Off: the window is 0 and the block is skipped.

TDD (the test also drives the real fault path: it includes `core/write_watch.cpp`, installs a SIGSEGV handler as `write_watch_test` does, and writes to armed `mmap` pages):
- RED 1: `fatal error: 'host/perf/core/fault_window_core.h' file not found`.
- RED 2, with the core header: `error: use of undeclared identifier 'write_watch_set_window_pages'` (2 errors).
- RED 3, with the setter but no block in `write_watch_on_fault`: exactly the six window checks failed (`the window's first page released the 15 after it`, `every page of the window reads written`, `a write further into a released window does not fault`, `mid-window: one fault`, `mid-window: from its own page to the window's end, none before it`, `the armed run went with the fault`), and the unset checks passed.
- GREEN: `bbg_fault_window_test: ok`. Upstream's `write_watch_test` still passes.
- Disassembly of `write_watch_on_fault` in the linked `bbhost`: no `call` and no `%fs` access, only its two raw `mprotect` syscalls (the window helper is inlined).

Local verification (the plan's Step 4; build `-Out $WS\out\p3-t14`):
```powershell
& $WS\scripts\switch-check.ps1 -Exe $WS\out\p3-t14\bbhost.exe -Name sc-t14 -Env @{BBHOST_BBG_FAULT_WINDOW_KB="64"} -Expect @('bbg: a write-watch fault releases the armed pages to the end of its 64 KiB window')
```
Expected: it passes, and the exit report's write-watch fault count is printed; compare it with `sc-base`'s. The line itself was already seen here (see Verification), as were `[perf] fault_window_kb` and `BBHOST_BBG_PERF`.

## Task 15: host GPU-feeding threads on performance cores (`BBHOST_BBG_PCORES`)

Files:
- New: `src/host/perf/core/pcores_core.h` (`performance_cpu_sets`), `tests/bbg_pcores_test.cpp`.
- Hook: `src/core/host_clock.cpp`, the includes plus `pcores_for_this_thread` (Windows only) above `host_thread_set_class`, called as the first statement of its Windows branch when `BBHOST_BBG_PCORES` starts with `1`.

Callers, all named threads: `bb-cp<N>` (gnm_exec.cpp), `bb-record`, `bb-submit`, `bb-present` and `bb-vblank`.

Behaviour: the thread is put on the CPU sets of the highest `EfficiencyClass` (`SetThreadSelectedCpuSets`, looked up by name) and says so: `timing: <who> runs on the <n> performance-core CPU sets of <m> (BBHOST_BBG_PCORES)`. A CPU with one class (AMD, older Intel) and Linux are unchanged. Off: the `getenv` test is false and the helper is not called.

TDD: RED `fatal error: 'host/perf/core/pcores_core.h' file not found`; GREEN `bbg_pcores_test: ok`. One case was added: three classes, as with low-power E-cores, give the highest only, in order.

Windows code: syntax-checked here with `clang++ --target=x86_64-w64-mingw32` against Ubuntu's mingw-w64 11 headers and libstdc++ 13, `-Wall -Wextra`: no error, no warning. mingw-w64 11 has `SYSTEM_CPU_SET_INFORMATION` with the `CpuSet` union, so no local definition was needed. Arch's newer mingw-w64 has it too, but the real cross build is local.

Local verification (the plan's Step 3; build `-Out $WS\out\p3-t15`):
```powershell
& $WS\scripts\switch-check.ps1 -Exe $WS\out\p3-t15\bbhost.exe -Name sc-t15 -Env @{BBHOST_BBG_PCORES="1"} -Expect @('timing: bb-submit runs on the 8 performance-core CPU sets of 20', 'timing: bb-record runs on the 8 performance-core')
```
Expected: it passes; the 265KF reports 8 of 20.

## Rulings

1. Ruling: Task 8's `FrameNeeds::push` returns 0 when flip N-k's need is not below flip N's own.
   - Why: Review Focus 4 says the bound "only ever waits for submissions already handed over". The plan's argument holds only while every job submits: with `BBHOST_SUBMIT_EVERY_JOB=0`, which `kLateUpstream` lets a preset set, flip N-k's need can be the recording still under way, which only the waiting command processor itself could hand over. The plan's code would then sit out the 250 ms give-up on every such flip. With the clamp, a need below flip N's is always flushed. RED 2 above shows the new case failing against the plan's code.
   - Cost if wrong: when a flip's job recorded nothing, the bound waits one flip later than it could.
2. Ruling: `bbg_frame_pacing_test` also tests the guard. It includes `host/perf/frame_pacing.cpp` beside stand-ins for `host_gpu_submissions_completed`, the clock, `host_sleep_us`, `hle_video_flip_count`, `perf_uint` (k = 1) and `perf_report_due`, as `translation_cache_test` includes its source.
   - Cases: the 250 ms give-up, an early end when the work completes, no wait for a game-thread request or a flip with no GPU work.
   - Why: Review Focus 4 names this test, and the plan's version tests only the ring.
   - Cost if wrong: none to the build; the test takes 0.25 s.
3. Ruling: Task 9 audits one more return than the plan lists: `upload_cmd_locked()` failing (no command buffer for the copy) leaves the bind in host memory, so it is counted as `no-mirror`.
   - Why: "Audit at each return to host memory".
   - Cost if wrong: one more counted path that practically never runs.
4. Ruling: Task 9's hook comments.
   - The first audit hook carries the full marker `// bbgoated (host/perf/vk/shadow_policy.cpp, BBHOST_BBG_SHADOW_AUDIT)` on its own line; the other eleven end with `// bbgoated (BBHOST_BBG_SHADOW_AUDIT)`.
   - The `go_dynamic` note is a two-line braced block.
   - The retry block sits before the upstream comment of the dyn/gpu early return, not between that comment and its `if`.
   - Why: lines stay within the file's ~150 columns (it has none longer), and the upstream comment stays with its code.
   - Cost if wrong: cosmetic.
5. Ruling: Task 12's `occlusion_interval_open_locked()` is `g_on && g_iv.open && g_iv.queries < g_iv.cap` (a draw now would get a query), not the plan's `g_on && g_iv.open`.
   - Why: `g_iv.open` is set at every dump and never cleared, so the plan's accessor is true for every draw after the first dump. Occlusion is on by default, so the skip would never fire. The draws a query measures are the ones `occlusion_draw_locked` gives one: up to 64 after a block's first dump, then one after any other dump. Those are protected. A draw past the cap reads unmeasured, so visible, whether drawn or skipped.
   - Cost if wrong: one extra draw per window is protected (the one after an end dump); and should the game ever measure more than the cap in one window, the extra draws could be skipped, which only makes that window read visible.
6. Ruling: Task 12's periodic line is also checked from `async_skip_note_pass_locked`, which runs for every drawn draw while the switch is on.
   - Why: the plan's Step 6 expects the line in an 18-flip smoke at the title. There, by flip 16, every pipeline may already be built, and `async_skip_draw_locked` would not be called again.
   - Cost if wrong: one `perf_report_due` per draw while the switch is on.
7. Ruling: Task 12's `PassHistory` prunes at most once a flip (`pruned_`), and the header includes `<iterator>` for `std::next`.
   - Why: the plan pruned whenever the map held more than 4096 passes. If more than 4096 were drawn within 120 flips (render targets at new addresses each frame), that is a 4096-entry walk at every draw.
   - Cost if wrong: none.
8. Ruling: Task 14's `window_ahead` is `__attribute__((no_stack_protector, always_inline))`, like every helper `write_watch_on_fault` calls.
   - Why: the handler runs on guest threads whose FS is the guest's. A debug build could otherwise call it out of line, with a canary.
   - Cost if wrong: none; clang 18 and the mingw target both accept it.
9. Ruling: `bbg_fault_window_test` includes `core/write_watch.cpp`, as `write_watch_test` links it, and drives the real fault path on `mmap` pages. The plan's test covers only the arithmetic.
   - Why: it is the only way to test the hook in `write_watch_on_fault` and its switch-off identity in the cloud.
   - Cost if wrong: none.
10. Ruling: Task 15 keeps the plan's raw `getenv("BBHOST_BBG_PCORES")` with `p[0] == '1'` rather than `bbg::perf_on`, and its one `timing:` line per thread.
    - Why: these are the plan's design, keeping `core/` free of `host/perf` code. `[perf] pcores = "1"` and `BBHOST_BBG_PERF` still work, because `perf_init` sets the environment before those threads start.
    - Cost if wrong: `pcores = "true"` or `"on"` reads as off, and the switch writes five lines instead of one (see Open concerns).
11. Ruling: commit trailers follow the README (rule 5): the `Claude-Session: https://claude.ai/code/session_01QamtaLwymAJnp4iySL9WYK` line, no other trailer, author KevY007, as the base's commits have.
    - Bodies follow 86f9fcc: the plan's subject, a paragraph on what the switch changes and that it is off by default, a RED/GREEN line.
    - Cost if wrong: amend the trailers locally before the merge.
12. Ruling: `cloud/p3-b` was pushed after each task commit, not only at the end, as a guard against losing the container. Only that branch was pushed. One push came back `[remote rejected] (failure)`; the plain retry two seconds later went through.
    - Cost if wrong: none.
13. Ruling: extra pure-test cases beyond the plan's (listed per task). They pin the documented boundaries (12 vertices, the 60-flip window, the per-pipeline budget, three efficiency classes, the 16 KiB and 1 MiB clamps).
    - Cost if wrong: none.

## Verification (commands and summary lines)

Build environment, as task R did it:
- `apt-get install` of `tools/ci_build.sh`'s list.
- Vulkan-Headers v1.4.350, SDL release-3.4.16 and FFmpeg n7.1.1 cloned with git into `/home/user/ci/src` under the names `fetch()` globs for.
- `CI_APT=0 CI_WORK=/home/user/ci`; the script itself unmodified.

| Run | Command | Result |
|---|---|---|
| ebb97dc (task R head), ON, first CI run (builds the prefix) | `CI_APT=0 CI_WORK=/home/user/ci CI_CMAKE_ARGS="-DBBGOATED=ON" bash tools/ci_build.sh` | `100% tests passed, 0 tests failed out of 60`; 343 warnings |
| be5ef07 (base), ON, incremental | `cmake --build /home/user/ci/build && ctest` | 61/61 |
| after Task 8 | ON incremental / OFF fresh (`cmake -S . -B build-off` with the script's arguments, no `CI_CMAKE_ARGS`) | 62/62 / 60/60 (OFF: 343 warnings) |
| after Task 9 | ON / OFF incremental | 63/63 / 61/61 |
| after Task 12 | ON / OFF incremental | 64/64 / 62/62 |
| after Task 14 | ON / OFF incremental | 65/65 / 63/63 |
| after Task 15 | ON / OFF incremental | 66/66 / 64/64 |
| final code head ad56cdf, ON, fresh build tree | the CI script as above | `100% tests passed, 0 tests failed out of 66` (25 skipped: game files), Total Test time 13.43 s; 343 warnings, the same (file, message, count) set as ebb97dc's |
| final code head ad56cdf, OFF, fresh build tree | the script's configure line without `CI_CMAKE_ARGS`, `cmake --build`, `ctest` | `100% tests passed, 0 tests failed out of 64` (25 skipped), Total Test time 12.98 s; 343 warnings, the same set as ebb97dc's |

The new tests in the final ON run: `bbg_async_skip_test`, `bbg_fault_window_test`, `bbg_frame_pacing_test` (0.25 s), `bbg_pcores_test` and `bbg_shadow_policy_test` passed, beside P3A's `bbg_perf_test`.

Windows target (no Windows build here): each changed source was syntax-checked with `clang++ --target=x86_64-w64-mingw32 -std=c++20 -fsyntax-only -Wall -Wextra` against mingw-w64 11 headers, libstdc++ 13 and the Vulkan headers. The script was a copy of `tools/win_syntax_check.sh` with Ubuntu's paths, kept outside the repo.
- Files checked: `video.cpp`, `buffer_shadow.cpp`, `render.cpp`, `occlusion.cpp`, `write_watch.cpp`, `host_clock.cpp`, `frame_pacing.cpp`, `fault_window.cpp`, `vk/shadow_policy.cpp`, `vk/async_skip.cpp`, `bbg_perf.cpp`.
- No errors. `write_watch.cpp`, `host_clock.cpp` and every new file had no warnings. `video.cpp` (1), `buffer_shadow.cpp` (12), `render.cpp` (94) and `occlusion.cpp` (16) give the same warnings as their be5ef07 versions.

Smoke without game files: the CI-built `bbhost` of ad56cdf (`bbhost v0.2.18-81-gad56cdf`, BBGOATED=ON) ran headless in a scratch sandbox (`BBHOST_CONFIG_DIR`, `HOME`). It got through static initialization, `config_load` and `bbg::perf_init`, then stopped at the missing-paths check with exit 2.
- Every switch unset: zero `bbg ` or `bbg:` lines.
- `BBHOST_BBG_FAULT_WINDOW_KB=64 BBHOST_BBG_FRAMES_AHEAD=1 BBHOST_BBG_SHADOW_AUDIT=1 BBHOST_BBG_ASYNC_SKIP=1 BBHOST_BBG_PCORES=1`: `bbg perf: BBHOST_BBG_FRAMES_AHEAD=1 BBHOST_BBG_SHADOW_AUDIT=1 BBHOST_BBG_ASYNC_SKIP=1 BBHOST_BBG_FAULT_WINDOW_KB=64 BBHOST_BBG_PCORES=1`, then `bbg: a write-watch fault releases the armed pages to the end of its 64 KiB window (BBHOST_BBG_FAULT_WINDOW_KB)`.
- `BBHOST_BBG_PERF=FAULT_WINDOW_KB=100` gives the 128 KiB window; `[perf] fault_window_kb = 16` in the sandbox's `bbgoated.toml` gives 16 KiB; the environment's 1024 beats `[perf]`; `=0` gives no window line.

Other checks:
- Online paths, with the workspace's `src/bbgoated` linked to the code repo (`/src/` is ignored there, so nothing in the workspace changed): `pwsh -NoProfile -File scripts/check-online-untouched.ps1 -Branch cloud/p3-b` printed `online paths untouched since 566184a (1 upstream cherry-picks allowed)`. PowerShell 7.4.6 came from the GitHub release, its SHA-256 checked against the release's `hashes.sha256`.
- Mode changes: `git diff --cached --summary | grep -c "mode change"` printed 0 before each commit, and `git diff --summary be5ef07..cloud/p3-b` has none.
- Trial merge with P3A: `git merge-tree --write-tree --name-only origin/cloud/p3-a cloud/p3-b` (p3-a at 973d6fb) exits 0 with a tree and no conflicted files. The same merge, made on a throwaway local branch (`git merge origin/cloud/p3-a`, deleted afterwards, never pushed) and built fresh with `-DBBGOATED=ON`: `100% tests passed, 0 tests failed out of 70`, 343 warnings. git auto-merged buffer_shadow.cpp, gpu_internal.h and render.cpp without overlapping hunks.
  - Where the branches meet: P3A's Task 6 changes the `if (g_lazy_barriers) {` line in `draw_impl`, about 11 lines above Task 12's line before `begin_pass`. P3A's Task 5 changes `shadow_end_uploads_locked` in buffer_shadow.cpp, which Task 9 does not touch.

## What needs local verification

1. The Windows build of the branch (alone, then merged with `cloud/p3-a`): `git -C $WS\src\bbgoated fetch origin cloud/p3-b:cloud/p3-b`, then `powershell -NoProfile -File $WS\scripts\build.ps1 -CMakeArgs "-DBBGOATED=ON" -Out $WS\out\p3-b` with the branch checked out (or the controller's merge). New for the mingw build: `host_clock.cpp`'s CPU-set code (`SYSTEM_CPU_SET_INFORMATION`, `GetSystemCpuSetInformation` and `SetThreadSelectedCpuSets` by name). Everything else is platform-neutral and was syntax-checked for the target here.
2. Switch-off identity in game (Review Focus 6): a smoke with no `BBHOST_BBG_*` set has zero `bbg ` lines, and the same `tools/log_health.py` classes and exit counters as the base.
3. Each task's switch checks, as listed under the task (the plan's own commands and expected lines), with the validation runs the plan names: Task 9's `sc-t09-frames` and Task 12's `sc-t12` with `-Validate -Baseline $WS\docs\perf\validation-base-2026-10-12.txt`.
4. The world recipes and A/B passes in the plan (Tasks 8, 9, 12, 14, 15), for the later harness pass. They are listed in each task's "A/B (later)" block.
5. Task 15 needs the 265KF: the expected count, 8 of 20 CPU sets, cannot be seen anywhere else.

## Open concerns

- Task 8: the wait delays `post_gnm_eop()`, the job's done callback. That is the intended back-pressure. A GPU stall costs at most 250 ms a flip before the guard gives up, and `gave up` in the periodic line says so.
- Task 9: the audit line is written from the host-bind path, so a run with no host bind after flip 16 has no audit line. The policy line likewise needs a copy under a rule. This is as the plan expects.
- Task 12: the check after the lean block, as the plan writes it, may also skip a draw that lacks a binding while its pipeline's variant builds. Such a draw needs the fallback anyway, which it then builds in place after the variant arrives or the 45-flip budget ends. Only the moment of that build moves, never a draw with side effects.
- Task 14's perf hook runs from `bbg::perf_init`, which `main()` calls only in BBGOATED=ON builds (P3A's Task 3). In a BBGOATED=OFF build `BBHOST_BBG_FAULT_WINDOW_KB` therefore does nothing; the other four switches are read lazily and work from the environment in both builds.
- Task 15 logs one `timing:` line per GPU-feeding thread (five or more), as the plan's switch check expects. The Global Constraints allow a switch "at most one log line at start", so the plan contradicts itself here; the plan's design was kept.
- The plan's local commands name `bbhost.exe`; a BBGOATED=ON package's executable may be `bbgoated.exe` (task R's report uses that name). Use the package's own.
