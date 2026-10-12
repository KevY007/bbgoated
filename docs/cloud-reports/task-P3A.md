# Task P3A: Phase 3 switch core and the first wave-1 switches (Tasks 3, 4, 5, 6)

Cloud run of 2026-10-12 (session https://claude.ai/code/session_01FKTi1Sxm9MRvAgyJfmUNBa; the commits carry the controller's `Claude-Session` line, as README rule 5 asks).

| Repo | Branch | Base | Commits (oldest first) |
|---|---|---|---|
| KevY007/bbgoated | `cloud/p3-a` | `cloud/task-R-rebase` ebb97dc | be5ef07 Task 3, a2ff6cf Task 4, f293a50 Task 5, 973d6fb Task 6, then this report |

The workspace repo is unchanged.

## Result

- Four commits, one per task, with the plan's subjects. `cloud/p3-a` was pushed right after the switch core (be5ef07) and again after each task. P3B's `cloud/p3-b` branches from be5ef07 (`git merge-base` says so).
- Every task is test-first. Each has a header-only core under `src/host/perf/core/` with a `tests/bbg_*_test.cpp`, and Task 4 also has a Python test for its tool. RED and GREEN for each are below.
- Linux CI (upstream's `tools/ci_build.sh` unmodified, from scratch, on an export of 973d6fb):
  - BBGOATED=ON: `100% tests passed, 0 tests failed out of 65` (23 skipped).
  - BBGOATED=OFF: `100% tests passed, 0 tests failed out of 63` (23 skipped).
  - The base ebb97dc gave `100% tests passed, 0 tests failed out of 60` (25 skipped) before lavapipe was installed. The 5 new tests are the bbg ones. Two upstream GPU tests (`gcn_sampling_test`, `indirect_clamp_test`) run on lavapipe instead of skipping, and they pass on the base build as well.
- Switch-off identity:
  - With every `BBHOST_BBG_*` unset, the binary logs no `bbg ` line: the startup path (`bbhost --headless` in a sandbox), and the stream self-test on lavapipe, before and after each task.
  - Every hook is a call that returns at its first test, or the same values as before.
  - Compiler warnings are unchanged: the whole OFF build has the same 343 warnings (file and message) at ebb97dc and at 973d6fb, and the new files add none.
  - Task 6 keeps upstream's `BBHOST_PASS_BARRIERS` truth table for every value. A test checks it.
- `check-online-untouched.ps1 -Branch cloud/p3-a` (workspace script, PowerShell 7.4.6 here, release tarball SHA-256 checked) after each commit printed `online paths untouched since 566184a (1 upstream cherry-picks allowed)`. `git diff --summary ebb97dc..HEAD` has no `mode change` lines. `git diff --check` is clean.
- Integration checks, local only, nothing pushed:
  - `cloud/p3-a` rebases onto the current `origin/bbgoated/main` (dde2fa6) without conflicts. That is ebb97dc plus four tools/docs commits.
  - `git merge-tree` of `cloud/p3-a` with `cloud/p3-b` (4afe1b7, its head when I checked) is clean. The merged tree's build and ctest are under "Integration" below.

## Rulings

1. Ruling: the base is `cloud/task-R-rebase` ebb97dc, as the brief says.
   - Why: `origin/bbgoated/main` had moved to dde2fa6 (ebb97dc + `area check --sweep` and docs, touching `src/hle/video.cpp`/`system.cpp`/`modules.h` and tools) before this run, but the ruling is binding and P3B waits on the same base.
   - Cost if wrong: none found. The branch rebases cleanly onto dde2fa6.
2. Ruling: the `cmake/bbgoated.cmake` perf block goes between the stage-share and red-zone blocks, not at the end.
   - Why: Phase 2 appends its `include(...bbgoated-upscale.cmake)` at the end and rewrites the DLAA lines there. A block between two untouched blocks merges with both. No existing line is reordered or rewritten.
   - Cost if wrong: move eight lines.
3. Ruling: `perf_report()` lists the upstream switches a preset may set (`kLateUpstream`) only beside a bbgoated switch, or when `perf_init` set something.
   - Why: the plan's code made a `bbg perf: BBHOST_GPU_SLOTS=3` line out of an upstream variable set by hand. The binding ruling says no `bbg ` line with every `BBHOST_BBG_*` unset.
   - Cost if wrong: a log with only upstream switches set does not echo them in a `bbg perf:` line.
4. Ruling: Tasks 4, 5 and 6 get pure cores and tests the plan does not have. They are `census_core.h`, `device_mem_core.h` and `lazy_barriers_core.h`, with `bbg_census_test`, `bbg_device_mem_test`, `bbg_lazy_barriers_test` and `bbg_sites_test.py`.
   - Why: the brief asks for each task test-first, and README rule 7 says logic gets a failing test first. The plan's interfaces are unchanged: Task 7 still finds `bbg_rebar_type`, `create_dev_buffer(..., rebar)`, `bbg_presubmit_add_locked`, `bbg_presubmit_register` and `g_bbg_params_twin`.
   - Cost: small. The census lines now order ties by offset, which is deterministic.
5. Ruling: `tests/bbg_sites_test.py` is registered with one `add_test` in `cmake/bbgoated.cmake`, under `if(TARGET config_test AND Python3_Interpreter_FOUND)`, in Task 4's commit.
   - Why: the glob takes only `tests/bbg_*_test.cpp`. Upstream registers its Python tests the same way.
   - Cost if wrong: one cmake edit after Task 3, which P3B does not touch.
6. Ruling: `tools/bbg_sites.py` fixes two bugs in the plan's version, both found by running it on every real call site of the Linux binary.
   - (a) It symbolizes `offset - 1`, the call instruction, because a census offset is a return address. In the Task 4 commit's binary, the plan's tool put `submit_locked`'s call at `gpu.cpp:4009`, a line of another function's inlined code, instead of `gpu.cpp:5141`. Four sites came out as `gpu_internal.h:132`, two as `?`.
   - (b) It drops `(anonymous namespace)::` before cutting the parameter list. The plan's regex cut those names to `gpu::` or nothing at 9 of the 32 pass-end sites: `begin_pass`, `clear_depth`, `copy_layers_locked`, `tess_ls_pass_locked`, `record_target_readback`, `upload_surface`, `resolve_locked`, `ensure_locked` and `transfer_begin_locked`.
   - The script is split into functions with a `main()` so the test can import it. Its command line and output format are the plan's.
   - Cost if wrong: none; the old output was wrong.
7. Ruling: the params ring gets the same "no room in video memory: stay where it was" retry that the plan gives `create_dev_buffer`, plus a `bbg: no room for the params ring in video memory; it stays in memory type N` line.
   - Why: without it, `BBHOST_BBG_PARAMS_MEM=rebar` on a full 256 MiB BAR window would end device creation with `gpu: params buffer allocation failed`.
   - Cost: 6 lines in `init_locked`, reached only with the switch on.
8. Ruling: `lazy_barriers()` reads `BBHOST_PASS_BARRIERS` and `BBHOST_BBG_LAZY_BARRIERS_NV` once, at its first use, but asks `g.vendor_id` at every call. The plan latched all three at the first call.
   - Why: `smoke.ps1` runs with `BBHOST_HEADLESS=1`. A headless run makes the device lazily on the command processor, and the first flip report (`host_gpu_recorder_report` -> `render_barriers_report`) can run before that. With vendor 0 latched, the NVIDIA switch would stay off for the whole run, and Task 6's own check would fail.
   - Cost: per draw, two static-guard loads and a compare instead of one global bool load.
9. Ruling: `device_mem.cpp` fills its two Vulkan structs as `X x{}; x.sType = ...;`.
   - Why: upstream's `stream_selftest.cpp` does the same. The plan's `X x{VK_STRUCTURE_TYPE_...}` added two `-Wmissing-field-initializers` warnings.
   - Cost: none.
10. Ruling: the tests use the `check` lambda that the plan and bbgoated's existing tests (`dlaa_aa_test.cpp`, `patch_manifest_cpu_test.cpp`) use.
    - Why: README rule 6 asks for the existing `tests/*_test.cpp` style. Upstream's tests use a `CHECK` macro and bbgoated's use this lambda; both return non-zero on a failure.
    - Cost if wrong: a mechanical rewrite.
11. Ruling: lavapipe (`mesa-vulkan-drivers` 25.2.8), `vulkan-validationlayers` 1.3.275, gdb and mingw-w64 gcc/binutils were installed in the cloud machine, nowhere else.
    - Why: they let upstream's stream self-test exercise Tasks 5 and 6 on a CPU Vulkan device under the validation layer, and let `bbg_sites.py --windows` be tested on a real PE.
    - Cost: none to the branch.

## Task 3: the switch core (be5ef07)

**Files**
- `src/host/perf/core/perf_plan.h`: list parsing and precedence.
- `src/host/perf/core/perf_switches.h`: the 26 switch names with help, and `kLateUpstream`. All 13 entries were re-checked on ebb97dc: each is read after `main()` starts. `BBHOST_PASS_BARRIERS` is read that way from Task 6 on.
- `src/host/perf/bbg_perf.h`, `src/host/perf/bbg_perf.cpp`.
- `tests/bbg_perf_test.cpp`.
- `src/main.cpp`.
- `cmake/bbgoated.cmake`.

**Hooks**
- `src/main.cpp` `main()`: `bbg::perf_init();` as the last line of the `#if defined(BBHOST_BBGOATED)` block after the first `config_load` (after `redzone_audit_install();`), and `#include "host/perf/bbg_perf.h"` after `host/bbgoated.h`.
- `cmake/bbgoated.cmake`: the globs `src/host/perf/*.cpp`, `src/host/perf/vk/*.cpp` (in `if(Vulkan_FOUND)`) and `tests/bbg_*_test.cpp` (in `if(TARGET config_test)`).

**RED** (the test and the globs first):
```
$ cmake --build build-on --target bbg_perf_test
/home/user/bbgoated/tests/bbg_perf_test.cpp:3:10: fatal error: 'host/perf/core/perf_plan.h' file not found
ninja: build stopped: subcommand failed.
```

**GREEN**
```
$ cmake --build build-on --target bbg_perf_test && build-on/bbg_perf_test
bbg_perf_test: ok
$ ctest --test-dir build-on -R bbg_perf_test
1/1 Test #59: bbg_perf_test ....................   Passed    0.00 sec
```

**Functional check in the cloud.** The Linux binary with a sandbox `BBHOST_CONFIG_DIR`. `perf_init` runs before the missing game paths end the run (exit 2):
```
no BBHOST_BBG_*                                   -> 0 lines with "bbg "
BBHOST_BBG_PERF="FRAMES_AHEAD=0,nosuch"           -> [bbhost] bbg perf: no preset named "nosuch"
                                                     [bbhost] bbg perf: BBHOST_BBG_FRAMES_AHEAD=0
BBHOST_GPU_SLOTS=3 only (upstream, by hand)       -> 0 lines with "bbg " (ruling 3)
[perf] frames_ahead = 2, pass_breaks = "1", preset = "nosuch2";
  env BBHOST_GPU_SLOTS=3 BBHOST_BBG_PERF="BBHOST_GPU_SLOTS=4,FRAMES_AHEAD=1,BBHOST_OCCLUSION=0"
                                                  -> bbg perf: BBHOST_OCCLUSION=0 (BBHOST_BBG_PERF) is read before the configuration is: set it in the environment
                                                     bbg perf: no preset named "nosuch2"
                                                     bbg perf: BBHOST_BBG_PASS_BREAKS=1 BBHOST_BBG_FRAMES_AHEAD=1 BBHOST_GPU_SLOTS=3
```
That last block shows four rules at once: `BBHOST_BBG_PERF` beats `[perf]`, the environment beats both, a quoted TOML value is unquoted, and a variable read before `main()` is refused.

**Local verification** (plan Step 6; `$pkg` = a Windows build of `cloud/p3-a`):
```powershell
& $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t03-off -Expect @('exit: flip \d+ reached')
& $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t03-on -Env @{BBHOST_BBG_PERF="FRAMES_AHEAD=0,nosuch"} -Expect @('bbg perf: BBHOST_BBG_FRAMES_AHEAD=0', 'bbg perf: no preset named "nosuch"')
(Select-String -Path $WS\sandbox\sc-t03-off\logs\run.log -Pattern 'bbg ' | Measure-Object).Count   # 0
```

## Task 4: pass-break and transfer-batch census, `BBHOST_BBG_PASS_BREAKS=1` (a2ff6cf)

**Files**
- `src/host/perf/census.h`, `src/host/perf/census.cpp`.
- `src/host/perf/core/census_core.h`: `census_line`, `census_flips`.
- `tools/bbg_sites.py`.
- `tests/bbg_census_test.cpp`, `tests/bbg_sites_test.py`.
- `cmake/bbgoated.cmake`: the Python test (ruling 5).

**Hooks** (one line each, plus the include):
- `src/host/render.cpp` `render_end_pass_locked`: `bbg::census_pass_end_locked(__builtin_return_address(0));` as the first line inside `if (g_pass.active)`.
- `src/host/gpu.cpp` `transfer_begin_locked`: `bbg::census_batch_open_locked(__builtin_return_address(0));` after `if (g.in_transfer) return;`.

Off by default: each hook is a call that returns at a function-local static. In the Linux build neither hooked function is inlined or tail-called. `render_end_pass_locked` has 32 real calls from 30 functions and `transfer_begin_locked` 8 from 8, so the return address is always a real caller's.

**RED** (header missing, then a stub with the right signatures):
```
$ cmake --build build-on --target bbg_census_test
tests/bbg_census_test.cpp:4:10: fatal error: 'host/perf/core/census_core.h' file not found
$ (stub: census_line returns the tag, census_flips 0) build-on/bbg_census_test
FAIL: no sites: the total alone
FAIL: most first, ties by offset; hex offsets from the image base; per flip
FAIL: the total counts every site
FAIL: 16 sites at most
FAIL: a report covers the flips since the last, at least one
exit 1
$ python3 tests/bbg_sites_test.py      # tools/bbg_sites.py as the plan writes it, split into functions
FAIL: test_call_instruction_not_return_address ... AssertionError: 6699 != 6698
FAIL: test_function_names ... AssertionError: 'gpu::' != 'gpu::begin_pass'
FAILED (failures=2)
```

**GREEN**
```
$ build-on/bbg_census_test
bbg_census_test: ok
$ python3 tests/bbg_sites_test.py
Ran 3 tests in 0.000s
OK
```

**Functional check of the tool**
- Linux: a census log made from every real call site of the hooked functions in the final binary (return addresses from `objdump -d`), symbolized with `tools/bbg_sites.py LOG --exe build-on/bbhost`. Every site comes out with its function and the exact line of its call. Examples:
  - `gpu::submit_locked (gpu.cpp:5171)`, `gpu::flush_deferred_writes_locked (gpu.cpp:6469)`;
  - `gpu::clear_depth (render.cpp:6665)`, `gpu::upload_surface (textures.cpp:1154)`, `transfer_begin_locked (gpu.cpp:6368)`;
  - `gpu::begin_pass (render.cpp:6560)`: line 6560 is `end_pass_for_draw_locked`'s `render_end_pass_locked();`, inlined into `begin_pass`, so the line identifies it.
- Windows path: a mingw-gcc PE whose `end_pass()` calls a census with `__builtin_return_address(0) - &__ImageBase`. With its return-address offsets, `tools/bbg_sites.py run.log --exe census_pe.exe --windows` printed `submit_work (census_pe.c:9)` and `draw_work (census_pe.c:14)`. That checks the ImageBase from `objdump -p`, the mingw addr2line and the `-1`.
- The stream self-test on lavapipe passes with the switch off and on, under the validation layer, with 0 validation messages. It has no flips, so it prints no census line.

**Local verification** (plan Step 4):
```powershell
& $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t04 -Env @{BBHOST_BBG_PASS_BREAKS="1"} -Expect @('bbg pass ends: all=', 'bbg transfer batches: all=')
$dbg = (Get-ChildItem $pkg -Filter *.debug.exe | Select-Object -First 1).Name
wsl -d archlinux -u root -- python3 /mnt/c/Users/rehab/Desktop/bbgoated/src/bbgoated/tools/bbg_sites.py /mnt/c/Users/rehab/Desktop/bbgoated/sandbox/sc-t04/logs/run.log --exe /mnt/c/Users/rehab/Desktop/bbgoated/out/<pkg>/$dbg --windows
& $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t04-off -Expect @('exit: flip \d+ reached')
```
Expected:
- `switch-check: ok sc-t04`, and the off run's log has no `bbg pass ends` line.
- The symbolizer names `gpu::flush_deferred_writes_locked (gpu.cpp:...)` for batches at each submission, and `gpu::submit_locked (gpu.cpp:...)` and the draw path's pass ends for passes.
- The draw path's site can be named after the function `end_pass_for_draw_locked` is inlined into (`gpu::begin_pass`). The line is the call's.

## Task 5: params ring and page tables in video memory, `BBHOST_BBG_PARAMS_MEM=rebar|device`, `BBHOST_BBG_PT_MEM=rebar` (f293a50)

**Files**
- `src/host/perf/vk/device_mem.h`, `src/host/perf/vk/device_mem.cpp`: the plan's interface.
- `src/host/perf/core/device_mem_core.h`: `CopyList`/`copies_add` (merging), `params_span`, `resizable_bar_heap`.
- `tests/bbg_device_mem_test.cpp`.

**Hooks**
- `src/host/gpu_internal.h`: `create_dev_buffer(..., bool cached = false, bool rebar = false)`, and the exports `shadow_upload_cmd_for_bbg_locked()` and `shadow_uploads_feed_shaders_locked()` after `shadow_end_uploads_locked()`.
- `src/host/buffer_shadow.cpp`:
  - the two exports after `shadow_stage_locked`;
  - `shadow_end_uploads_locked`'s closing barrier, widened to shader and uniform reads (VS, FS, CS, and the tessellation stages when enabled) only when the flag is set. Unset, the masks are the old ones.
- `src/host/gpu.cpp`:
  - `create_dev_buffer`: the rebar type, and a retry in the upstream type when it has no room;
  - `init_locked`, params ring: `TRANSFER_SRC` for `device`, the rebar type with a retry (ruling 7), and the twin after `g_params_desc = ...`;
  - `init_locked`, page tables: `g.l1` and `g.sink_l2` with `bbg_pt_rebar()`; the sink page stays device-local;
  - `set_page_locked`;
  - `alloc_params_slot_locked`: the twin as the descriptor's buffer;
  - `submit_locked`: `bbg_presubmit_locked();` right after `g.recording = false;`, ahead of the `cmds[2] = {shadow_end_uploads_locked(), g.cmd_}` line and its comment.

Every params descriptor (`alloc_params_slot_locked`'s and `g_params_desc`, which is always a dynamic UBO offset by those slots) points at the twin in `device` mode. `g.ubo_map` is only ever written, never read, so `rebar` does no uncached reads.

**RED**
```
$ cmake --build build-on --target bbg_device_mem_test
tests/bbg_device_mem_test.cpp:4:10: fatal error: 'host/perf/core/device_mem_core.h' file not found
$ (stub: copies_add does nothing, params_span {0,0}, resizable_bar_heap false) build-on/bbg_device_mem_test
FAIL: a copy continuing the last one in both buffers extends it
FAIL: a gap in either buffer starts a region of its own
FAIL: one list per source and destination, in the order first seen
FAIL: the span covers exactly the blocks the submission wrote
FAIL: a heap of all video memory is
exit 1
```

**GREEN**
```
$ build-on/bbg_device_mem_test
bbg_device_mem_test: ok
```

**Functional check in the cloud.** Upstream's stream self-test (`bbhost --stream-selftest=3`) on lavapipe (llvmpipe, Mesa 25.2.8, Vulkan 1.4), with `BBHOST_VK_VALIDATE=1 VK_KHRONOS_VALIDATION_VALIDATE_SYNC=1`. A temporary, uncommitted patch to the self-test allocated 3 params blocks before each of its random submissions, so the twin copy is actually recorded. The patch was reverted, and the committed tree has no self-test change.
```
off     gpu: params ring in memory type 0 (flags 0xf)                                    stream selftest: pass
rebar   bbg: device-local host-visible memory: type 0, heap 16093 MiB (resizable BAR)    stream selftest: pass
device  bbg: params ring twin in device-local memory (32 MiB), filled before each submission (BBHOST_BBG_PARAMS_MEM=device)   pass
pt      bbg: device-local host-visible memory: type 0, heap 16093 MiB (resizable BAR)    stream selftest: pass
validation message IDs, identical in all four runs:
  38 x 0x5c0ec5d6 vkCmdFillBuffer (SYNC-HAZARD-WRITE-AFTER-WRITE, the self-test's own fills)
   1 x 0x79de34d4 and 1 x 0x901f59ec vkCreateDevice (the 1.3.275 layer does not know maintenance8)
gdb, device, one round: the twin copies recorded 40 times, the widened barrier emitted 40 times
```
Without the patch, each value passes the self-test. The off run has 0 `bbg` lines.

**Local verification** (plan Step 7, unchanged):
```powershell
$base = "$WS\docs\perf\validation-base-2026-10-12.txt"
& $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t05-off -Expect @('params ring in memory type \d+ \(flags 0xe\)')
& $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t05-prebar -F12 -Env @{BBHOST_BBG_PARAMS_MEM="rebar"} -Expect @('bbg: device-local host-visible memory: type', 'params ring in memory type \d+ \(flags 0x7\)') -Validate -Baseline $base
& $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t05-pdev -F12 -Env @{BBHOST_BBG_PARAMS_MEM="device"} -Expect @('params ring twin in device-local memory') -Validate -Baseline $base
& $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t05-pt -F12 -Env @{BBHOST_BBG_PT_MEM="rebar"} -Expect @('bbg: device-local host-visible memory: type') -Validate -Baseline $base
```
Expected:
- Four `switch-check: ok`.
- `validation: no new message IDs` in the three switched runs.
- No `bbg:` line in the off run.
- On the 5080 the rebar line should say `resizable BAR` and the ring `flags 0x7`. A line `bbg: no room for the params ring in video memory` means the BAR window was full and the ring stayed in its old type.

## Task 6: upstream's lazy pass barriers on NVIDIA, `BBHOST_BBG_LAZY_BARRIERS_NV=1` (973d6fb)

**Files**
- `src/host/perf/core/lazy_barriers_core.h`: `pass_barriers_env`, `lazy_barriers_on`.
- `tests/bbg_lazy_barriers_test.cpp`.

**Hooks** (`src/host/render.cpp`):
- `const bool g_lazy_barriers = [] {...}();` becomes `bool lazy_barriers()` (ruling 8).
- Its three uses: `end_pass_for_draw_locked` (`if (!lazy_barriers())`), `draw_impl` (`if (lazy_barriers()) {` before `begin_pass`), and `render_barriers_report`.
- `git grep -n g_lazy_barriers src` prints nothing. All three callers hold `g.mu`.

**RED**
```
$ cmake --build build-on --target bbg_lazy_barriers_test
tests/bbg_lazy_barriers_test.cpp:5:10: fatal error: 'host/perf/core/lazy_barriers_core.h' file not found
$ (stub: always off) build-on/bbg_lazy_barriers_test
FAIL: the switch on an NVIDIA device: on
FAIL: BBHOST_PASS_BARRIERS=lazy: on, whatever the vendor
FAIL: BBHOST_PASS_BARRIERS empty: as if unset
FAIL: switch unset: upstream's truth table for every BBHOST_PASS_BARRIERS value
exit 1
```

**GREEN**
```
$ build-on/bbg_lazy_barriers_test
bbg_lazy_barriers_test: ok
```
The last case compares the switch-unset result with upstream's `e && strcmp(e, "lazy") == 0`, for 8 values × 4 vendors.

**Functional check in the cloud.** gdb stops at the self-test's first `gpu::submit_locked` (device made, vendor 0x10005 Mesa), sets `gpu::g.vendor_id` to 0, then 0x1002, then 0x10de, and calls `gpu::render_barriers_report()` after each:
```
no switch                               ""  ""  ""
LAZY_BARRIERS_NV=1                      ""  ""  "pass barriers (lazy): 0 paid, 0 left out, 0 of the paid in the middle of a pass"
LAZY_BARRIERS_NV=1 PASS_BARRIERS=full   ""  ""  ""
PASS_BARRIERS=lazy                      line line line
LAZY_BARRIERS_NV=1 PASS_BARRIERS=       ""  ""  line
```
A call before the vendor is known does not latch the switch off.

**Local verification** (plan Step 2, unchanged):
```powershell
& $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t06-nv -F12 -Env @{BBHOST_BBG_LAZY_BARRIERS_NV="1"} -Expect @('pass barriers \(lazy\): \d+ paid') -Validate -Baseline $base
& $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t06-full -Env @{BBHOST_BBG_LAZY_BARRIERS_NV="1"; BBHOST_PASS_BARRIERS="full"} -Expect @('exit: flip \d+ reached')
& $WS\scripts\switch-check.ps1 -Exe $pkg\bbhost.exe -Name sc-t06-off -Expect @('exit: flip \d+ reached')
```
Expected:
- The first run passes with `validation: no new message IDs` and the lazy line in the flip reports.
- In the other two, `Select-String -Path <run.log> -Pattern 'pass barriers \(lazy\)'` finds nothing.

## Test results (commands and summary lines)

Build directories were configured as `tools/ci_build.sh` does: clang 18, lld, its prefix with Vulkan-Headers v1.4.350, SDL3 3.4.16 and ffmpeg 7.1.1. The sources were cloned with git into `$CI_WORK/src` under the names its `fetch()` looks for.

| State | BBGOATED=ON | BBGOATED=OFF |
|---|---|---|
| ebb97dc (base), `CI_WORK=... CI_APT=0 CI_CMAKE_ARGS="-DBBGOATED=ON" bash tools/ci_build.sh` | 100% of 60 (25 skipped) | - |
| be5ef07 (Task 3), `ctest` | 100% of 61 (25 skipped) | 100% of 59 (25 skipped) |
| a2ff6cf (Task 4) | 100% of 63 (23 skipped; lavapipe from here on) | 100% of 61 (23 skipped) |
| f293a50 (Task 5) | 100% of 64 (23 skipped) | 100% of 62 (23 skipped) |
| 973d6fb (Task 6), `ctest` | 100% of 65 (23 skipped) | 100% of 63 (23 skipped) |
| 973d6fb, `tools/ci_build.sh` from scratch on an export | `100% tests passed, 0 tests failed out of 65` | `100% tests passed, 0 tests failed out of 63` |

The skips are the tests that need game files (exit 77).

## Integration (local only, nothing pushed)

- `git rebase origin/bbgoated/main` (dde2fa6) of `cloud/p3-a`, in a scratch clone: clean.
- `git merge-tree --write-tree cloud/p3-a origin/cloud/p3-b` (4afe1b7): clean, tree e583f03.
- Build and ctest of that merged tree (BBGOATED=ON): builds, and `100% tests passed, 0 tests failed out of 69` (23 skipped). That covers both branches' bbg tests: async_skip, census, device_mem, fault_window, frame_pacing, lazy_barriers, perf, shadow_policy, sites.
- The merged binary with nothing set logs 0 `bbg` lines, at startup and in the stream self-test, which passes.
- P3B's head may have moved since. The controller's local merge is the one that counts.

## What needs local verification

1. The Windows build of `cloud/p3-a` (`scripts\build.ps1 -CMakeArgs "-DBBGOATED=ON"`) and `check-online-untouched.ps1 -Branch cloud/p3-a`.
2. Each task's `switch-check.ps1` runs above, with smoke, F12 and validation against `validation-base-2026-10-12.txt`, and `assert-user-files-unchanged.ps1` after the GPU runs.
3. Task 4's symbolizer on the Windows `.debug.exe`, in WSL Arch with `x86_64-w64-mingw32-addr2line`.
4. Later, in a GPU slot: the plan's A/B recipes, Task 4's world diagnosis (`area-check.ps1 ... -Env @("BBHOST_BBG_PASS_BREAKS=1")`) and Task 5's and 6's A/Bs.

## Open concerns and notes for the controller

- **Hooks only run in BBGOATED builds.** `perf_init()` sits in main.cpp's `#if defined(BBHOST_BBGOATED)` block, as the plan places it. So in a BBGOATED=OFF build, `BBHOST_BBG_PERF`/`[perf]` are ignored and `perf_register_hook` hooks never run. Switches read lazily from the environment still work there. P3B's Task 14 (`fault_window.cpp`) is configured only through a hook, so `BBHOST_BBG_FAULT_WINDOW_KB` does nothing in an OFF build. If OFF builds should honour it, `perf_init()` (or the hooks alone) would move out of that `#if`.
- **Lazy mode bypasses the census.** With lazy barriers on (Task 6), pass ends between draws go through `end_pass_for_draw_locked`'s lazy path, not `render_end_pass_locked`, so the census counts only the ends that pay a barrier. That is the plan's "how many pass ends remain".
- **Self-test hazards on lavapipe.** The stream self-test's own `vkCmdFillBuffer`s make 38 `SYNC-HAZARD-WRITE-AFTER-WRITE` messages on lavapipe with sync validation, with every switch off, and the self-test still reports `validation messages 0`. That is upstream's, not this branch's; I mention it in case the validation runner meets it.
- **Line numbers in the VRAM report shift.** `create_dev_buffer`'s `vkAllocateMemory` site moves (the memory tracker tags sites by `__LINE__`), as with any edit above it in `gpu.cpp`.
