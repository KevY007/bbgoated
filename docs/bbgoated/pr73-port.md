# PR #73 in bbgoated: what is taken, left out and resolved

PR #73 (FIocker, `upstream-pr/73` = 539d45f, on upstream master 566184a) is ported piece by piece,
never merged: frame generation, Reflex and its presenter/frame-loop timing changes are left out (they
change the default path with the features off: report I §5.4). Hunk numbers are those of
`git diff 566184a 539d45f -- <file>`; `scripts/dev/pr73_hunks.py` in the workspace prints them
(`--list`) and writes the patch for `git apply --3way` (`--hunks 1,3-5 --out <patch>`).

Commits: (a1) the NGX bridge, (a2) the DLSS evaluation path, (b) super resolution presented at the
output size with its settings, (c) FSR 3.1.5 / FSR 4 v07, (d) object motion vectors - Tasks 3-7 of
the Phase 2 plan, in that order. Each is one `bbg/upscale:` commit that keeps PR #73's file and
function names, so a later upstream merge of PR #73 resolves mostly to "theirs".

## PR #73 at 539d45f, and what changed since the review

539d45f is four commits on 566184a: adb8f38 (temporal upscaling, paced frame generation, motion
vectors and Reflex: the squash of the head report I reviewed, 3bfcb65 on v0.2.18, rebased), 85d81a9
(DLSS 3x/4x generation rows in the normal menus), a3a7fa7 (FSR generation descriptor and constant
retirement) and 539d45f (docs).

- Files PR #73 adds are byte for byte 3bfcb65's, except `cmake/fsr.cmake` (+18 lines: it applies
  the new `cmake/fsr-frame-retirement.patch`, a3a7fa7), `src/host/display_settings.h` (85d81a9
  removed `experimental_mfg_enabled`), and frame-generation files and docs that are not taken. So
  line numbers quoted against 3bfcb65's `dlaa.cpp` (report I, the plan) are still right.
- Files PR #73 modifies have 3bfcb65's hunk counts, except `src/host/options.cpp` (17 -> 16:
  85d81a9 dropped the experimental-MFG hunk, so 3bfcb65's h12-h17 are h11-h16 now).
  `src/net/session.cpp` and `src/hle/video.cpp` are no longer touched: master's 59da4a6 (PR #118)
  made the same thread-naming change. The changed lines differ only in `render.cpp` h16, `gpu.cpp` h14
  (`fsr_retire_locked` no longer depends on the fence's result), the frame-generation rows and the
  fps_cap comment in `options.cpp`/`launcher.cpp`, `frame_rate.cpp` (Reflex, not taken) and
  `CMakeLists.txt` (not taken).
- The plan's five hand resolutions against next-steps (its table C, C1-C5) are gone: FIocker
  resolved `translate.h`'s StageParams asserts and `render.cpp`'s `vs_decomp` the way the plan
  proposed, and the contexts of `options.cpp` h11, `options.h` h1 and `launcher.cpp` h5 now hold
  next-steps' lines. Every subset in table B applies to the rebased bbgoated/main, strictly or with
  `--3way` (section F).

## Rulings for Tasks 3-7 (controller, 2026-10-12)

- **R1 - the DLAA fork's code is on bbgoated/main already.** Task 4 reverts it in five files,
  takes PR #73's files and hunks, and re-applies bbgoated's DLAA edits as marked blocks: section E.
- **R2 - bbgoated's DLAA edits survive the port.** The anti-aliasing hold (`host_opt_hold`,
  `host/dlaa_aa.*`, `tests/dlaa_aa_test.cpp`), its release when DLAA fails or idles,
  `BBHOST_TEST_DLAA_FAIL` and the pump-loop tick: section E, step 3 (E1-E10). The BDA drop is
  PR #73's own now (`dlaa.cpp:933-938`); Task 4's edit 4 makes it say so in the log.
- **R3 - the default reconstruction is DLAA where NGX is available (NVIDIA + `ngx_bridge.dll`), Off
  elsewhere; super resolution and FSR are opt-in.** PR #73's default is `[dlss] mode = "off"`, so
  bbgoated changes it in two places, each marked:
  - with no `[dlss] mode` set (in the config files or `BBHOST_DLSS_MODE`), the mode seeds DLAA
    unless `[dlaa] enabled` is false (R5): Task 4's transitional block and, from Task 5, the
    `kDlssMode` seed in `host_options_load()` (`options.cpp` h11), with `config_value("dlss.mode")`
    telling a set key from the default;
  - the first-start template (`config.cpp` h3) and `bbhost.example.toml` (h1) do not write
    `mode = "off"`: a written key would win over the default (R5), so it goes in as a comment.

  "Off elsewhere" needs no code: `dlaa.cpp` turns itself off for the session without the bridge,
  on another GPU or on Linux (`load_bridge`, `dlaa_add_device_extensions`), and the game renders
  unchanged, as with the fork's DLAA today. Two consequences:
  - Ruling (task 2), a third marked edit, in Task 5: PR #73 refuses live resolution changes with
    any mode but Off (`res_live()` h6, `apply(kResolution)` h7), so with DLAA the default every
    player would lose them, on any GPU. DLAA recreates its feature when the scene size changes
    (`ensure_locked`), as the fork's did, so bbgoated narrows both checks to the modes that change
    the render size: DLSS super resolution (`dlss_mode >= 2`) and FSR (`upscaler_backend != 0`).
  - Review Focus 1 reads accordingly: the harness A/B for (a2) compares the Phase 1 build with the
    a2 build at their defaults (both DLAA on NVIDIA), and Off against Off (`BBHOST_DLSS_MODE=off`).
- **R4 - bbgoated's options file is its own** (`bbgoated-options.toml`, since the separation). The
  plan's `bbgoated_owned_option` and `src/host/bbgoated_options.{h,cpp}` (with its test) are
  dropped: PR #73's rows, and bbgoated's `dlss_preset` and `upscale_sharpness` rows, save in
  `bbgoated-options.toml` like any other key, and the setup window's `[dlss]` keys go to
  `bbgoated.toml` (`config_user_file()`). bbhost's `bbhost-options.toml` is only ever read, at the
  first start, so Review Focus 2's check of the shared file holds by construction.
- **R5 - `[dlaa] enabled = false` still means Off at start, and `[dlaa] preset` still works;
  `[dlss] mode` wins when both are set.** PR #73's `config_on()` never reads `[dlaa] enabled` (its
  `dlss_mode()` is never empty), so the seed above carries it. `[dlaa] preset` and
  `BBHOST_DLAA_PRESET` are PR #73's own (`config_preset()`, `dlaa.cpp:98`); the plan's F10
  `dlss_preset` row (Task 5) is read only when neither names a preset.
- **R7 - the game's anti-aliasing is held off whenever a temporal reconstruction runs: DLAA, DLSS
  super resolution or FSR.** Tasks 5 and 6 widen `reconstruction_running()` (E9; Task 4 named the
  predicate so, `dlaa_running()` is its older name): its "provider started" input
  is NGX for DLSS and the FSR provider for FSR (`g_dlaa_ngx_ok` alone is false with FSR, which loads
  no bridge), a failed SR or FSR (`g_sr_render_failed` with FSR selected; DLSS falls back to native
  DLAA) counts as failed, and the idle clock (E8) is fed where a frame was actually reconstructed: at
  the anchor after DLAA's evaluate, and in `dlss_sr_upscale_hud_locked` after a successful SR or FSR
  evaluation - in super-resolution modes the anchor only records guides.
- **R8, R9 - base and head:** the rebased bbgoated/main on 566184a, and PR #73 at 539d45f. Every
  plan reference to 3bfcb65 reads 539d45f and every c421ea0 reads 566184a.

## A. Files PR #73 adds

| Path | Taken in | Notes |
|---|---|---|
| tools/ngx_bridge/{CMakeLists.txt, ngx_bridge.cpp, ngx_bridge.h, ngx_transfer_sync.h, ngx_transfer_sync_test.cpp, ngx_probe.cpp, ngx_gpu_test.cpp, BBPORT-NOTICE.txt} | a1 | `git checkout 539d45f -- tools/ngx_bridge` replaces the fork's v1 CMakeLists.txt, ngx_bridge.cpp and ngx_bridge.h (ae0b56f) and adds the rest; + a provenance header in ngx_bridge.cpp/.h (no licence stated by its authors yet). `ngx_transfer_sync_test` (a mock, no GPU) is registered in cmake/bbgoated-upscale.cmake, as CMakeLists h6 does |
| src/host/dlaa.cpp | a2 | replaces the fork's (section E). The plan's four `// bbgoated:` edits: no `host/reflex.h` (line 1); `guides.reflex_frame_id = 0` (line 1328); the two `g_bridge.fg_extensions` queries (lines 904, 930) only when `fg_requested()`; a log line when the `VK_EXT_buffer_device_address` strip (lines 933-938) removes it. Then bbgoated's DLAA edits E1-E9 (section E). Later Stage A fixes add more marked edits |
| src/host/dlaa.h | a2 | replaces the fork's; + E10 (section E) |
| src/host/display_settings.h, fsr_upscaler.h, object_motion.cpp, object_motion.h, motion_history.h | a2 | verbatim; object motion is inert until (d): only (d)'s render.cpp h23 calls `object_motion_init_locked` |
| src/host/shaders/dlaa_mv.comp, dlaa_mv.spv.h; tools/dlaa_cbscan.py; docs/dlaa.md | a2 | verbatim, each replacing the fork's file of the same name |
| tests/display_settings_test.cpp, tests/dlaa_motion_test.cpp | a2 | verbatim |
| src/host/presentation_coordinates.h, tests/presentation_coordinates_test.cpp | b | verbatim |
| cmake/fsr.cmake, cmake/fsr-clear-order.patch, cmake/fsr-debug-view.patch, cmake/fsr-frame-retirement.patch, LICENSE-FSR.txt, src/host/fsr_provider.cpp, src/host/fsr_upscaler.cpp, tools/win/Install-FSR4.ps1, tools/win/Install-FSR4.bat | c | verbatim. The debug-view and frame-retirement patches change FireBurn's FG module (the retirement patch also gives the 3.1.5 bridge a diagnostic count); both are kept so fsr.cmake stays PR #73's - a3a7fa7 rides with FSR (controller's ruling). The patches' blank context lines are one space, which `git diff --check` reports: keep them byte for byte |
| docs/fsr.md | c | without the FSR 3.1.6 frame-generation paragraphs |
| tests/motion_history_test.cpp, tests/object_motion_shader_test.cpp | d | verbatim |
| src/host/frame_generation.cpp/.h, fg_timing.h, reflex.cpp/.h, shaders/fsr_fg_hud.comp/.spv.h, tests/fg_timing_test.cpp, tools/win/fsr_provider_check.cpp, tools/win/analyze_*.py (4), docs/dlss-development.md, docs/fg-stability-validation.md, docs/fsr-validation.md, docs/reflex.md, docs/reflex-validation.md, docs/upstream-rebase-validation-2026-10-12.md, docs/data/*.csv (5) | - | left out: frame generation and Reflex, and their validation records (fsr_provider_check links FireBurn's FG module) |

## B. Files PR #73 modifies

| File | Hunks | Taken (commit) | Left out | Why left out |
|---|---|---|---|---|
| CMakeLists.txt | 6 | none | h1-h6 | sources and tests go in cmake/bbgoated-upscale.cmake, which steps aside when upstream registers them: h3's dlaa.cpp (a2) and fsr_upscaler.cpp with the FSR link minus `fsr3-vk-framegeneration-3.1.6` (c), h5's object_motion.cpp (a2) and `-Wno-missing-field-initializers`, h6's tests (display_settings_test, dlaa_motion_test: a2; ngx_transfer_sync_test: a1; presentation_coordinates_test: b; motion_history_test, object_motion_shader_test: d). h1 is reflex.cpp; h2 (`winpthread` under MINGW) only if the FSR link asks for it; h3's `BBHOST_HAVE_VULKAN`/`BBHOST_HAVE_FSR` are read only by reflex.* and frame_generation.cpp; h4 is the imgui VERSION.txt comment; h6's fsr_provider_check and fg_timing_test and its unconditional `enable_testing()` |
| third_party/imgui/VERSION -> VERSION.txt | rename | none | rename | a workaround for case-insensitive native Windows builds; the WSL cross build is case-sensitive |
| .gitignore | 1 | h1 (a1, `--3way`) | | the fork's last two lines are PR #73's last two: `--3way` merges h1 into exactly PR #73's file |
| bbhost.example.toml | 1 | h1 without its 3 frame-generation lines, `mode = "off"` as a comment (R3) (a2) | | |
| src/core/config.h | 1 | h1 (a2) | | the FG/Reflex fields stay, unused |
| src/core/config.cpp | 3 | h1 (a2); h2 without the `BBHOST_FG_BACKEND` block (a2); h3 without the frame-generation and Reflex template lines, `mode = "off"` as a comment (R3) (a2) | | |
| src/host/settings.h | 2 | h1, h2 (a2) | | the FG/Reflex fields stay, never set |
| src/main.cpp | 1 | h1 (a2) | | |
| tests/config_test.cpp | 1 | h1 (a2) | | |
| src/host/gpu.cpp | 18 | h6, h9, h14 + by hand: `#include "host/dlaa.h"`, `#include "host/fsr_upscaler.h"` (from h1) and `dlaa_add_device_extensions(g.phys, dext);` (h7's first line) (a2); h8 (c); h13 (d) | h1's `host/reflex.h`; h2-h5, h10-h12, h15-h18; the rest of h7 | Reflex submit attribution, device setup and IDs; h12 force-submits when the simulation frame changes; h7's compute-shader-derivatives block is for FSR frame generation's optical flow (FSR 4's own check, `fsr_upscaler.cpp:160-169`, asks for float16, int8, int16, integer dot products and extended storage formats, not derivatives) |
| src/host/gpu_internal.h | 4 | h1 (d) | h2-h4 | Reflex frame IDs |
| src/host/gpu.h, gpu_stub.cpp, recorder.cpp, stream_ops.h | 1, 1, 3, 2 | none | all | Reflex frame IDs through the recorder and presenter submits |
| src/hle/gnm.cpp, gnm_exec.cpp, system.cpp | 2, 7, 2 | none | all | Reflex markers and IDs |
| src/engine/frame_rate.cpp | 3 | none | all | moves the pacing wait above the host ticks and adds the Reflex frame start: game-loop timing with the features off |
| src/engine/live_resolution.cpp | 4 | h1-h4 (b) | | |
| src/gcn/translate.cpp | 10 | h1-h10 (d) | | |
| src/gcn/translate.h | 2 | h1, h2 (d) | | |
| src/host/translation_cache.cpp, tests/translation_cache_test.cpp | 1, 1 | h1 (d) | | |
| src/host/render.cpp | 30 | h1, h2, h21, h22, h24, h25 without its `if (s.object_motion) { ... }` params block, h26, h28, h29, h30 (a2); h3-h20, h23, h27 and h25's params block (d) | none | |
| src/host/window.cpp | 35 | h24 + by hand `#include "host/dlaa.h"` (a2); h2, h3, h4, h25-h29, h33, h34 + by hand: `#include "host/presentation_coordinates.h"`, the SR display size read in `vk_present` under the lock it already holds (h15's intent) and h35's removal of `host_present`'s g_display_w/h store (b) | h1's frame_generation.h and reflex.h; h5-h23; h30-h32; the rest of h35 | the present_swapchain_image refactor, the FG presenter and thread, the latency swapchain, present IDs, the acquire/present mutex, and PR #73's extra renderer-lock acquisition before the acquire (window.cpp:1377 at 539d45f) |
| src/host/window.h | 2 | h1 (b) | h2 | PresentFlip's Reflex and temporal IDs |
| src/host/overlay.cpp, overlay.h | 3, 2 | all (b) | | |
| src/host/options.h | 1 | h1 (b) | | |
| src/host/options.cpp | 16 | h1, h15 (a2); h3, h6, h7, h16, and h2, h4, h5, h8, h11, h12, h13, h14 without their frame-generation, Reflex and object-motion lines (b); those object-motion lines (d) | h9, h10 | FG backend/factor coupling in `set_index` |
| src/host/launcher.cpp | 6 | h1, h3, and h2, h4, h5, h6 without their frame-generation, Reflex and object-motion parts (b); the object-motion parts (d) | | |
| tools/win_package.sh | 1 | h1 (c) | | |

## C. Hunks taken in part, and the lines no hunk carries

Since the rebase, no hunk of table B needs a merge resolution. These are the edits a task makes by
hand after applying its hunks (the dry run in section F makes each one against an exact anchor):

- (a2) `render.cpp`: delete h25's `if (s.object_motion) { params.motion_positions = ...; ...
  params.motion_scale[1] = s.vport[2]; }` block (12 lines); (d) puts it back after
  `params.l1_table = g.l1.address;`, when StageParams has those fields.
- (a2) `gpu.cpp`: after `#include "core/portable.h"`, `#include "host/dlaa.h"` and
  `#include "host/fsr_upscaler.h"`; after the `} else { g.fifo_latest_ready_ext = nullptr; }` block,
  `dlaa_add_device_extensions(g.phys, dext);`.
- (a2) `window.cpp`: after `#include "host/bindings.h"`, `#include "host/dlaa.h"`.
- (a2) `config.cpp`: delete h2's `BBHOST_FG_BACKEND` block (4 lines) and h3's frame-generation and
  Reflex template lines (8 lines); `bbhost.example.toml`: delete the `frame_generation_backend`,
  `frame_generation` and `frame_generation_factor` lines. R3: `mode = "off"` becomes a comment in both.
- (a2) `dlaa.cpp` and `dlaa.h`: the plan's edits 1-4 (table A), then E1-E10 (section E).
- (b) `window.cpp`: after `#include "host/present_pass.h"`, `#include "host/presentation_coordinates.h"`;
  right after `const auto t_locked = std::chrono::steady_clock::now();` in `vk_present`, the marked
  block that calls `gpu::dlss_sr_display_dimensions_locked(display_va, &display_w, &display_h)` and
  stores the presented size in g_display_w/h under g_mouse_mu, calling `mouse_remap_locked()` when it
  changed; in `host_present`, delete the `// What the pointer's window position scales by.` block.
- (b) `options.cpp`: delete the frame-generation and Reflex parts h2, h4, h5, h8, h11-h14 carry
  (enum entries, rows, `g_rows` entries, `rebuild_settings()` fields and the FSR factor clamp, seeds,
  `g_default_index` entries, the `frame_generation` value migration, the `BBHOST_FG_BACKEND`,
  `BBHOST_REFLEX` and `BBHOST_DLSS_FG` blocks, the two factor clamps and the
  `g_set[kFrameGeneration].index = 0;` in the `BBHOST_DLAA` block), and until (d) their object-motion
  parts; `launcher.cpp` likewise (variables, label arrays, `[dlss]` saves, `host_opt_set*` calls,
  combos, notes; `if (upscaler_backend == 0 || fg_backend == 0)` keeps `upscaler_backend == 0`).
  Then the marked R3 edits: the DLAA seed beside h11's seeds, and h6/h7 narrowed to SR and FSR.
- (d) `options.cpp`/`launcher.cpp`: PR #73's object-motion parts of the same hunks.

Gone since the rebase: C1 (translate.h h2 against the GDS asserts: 539d45f's h2 keeps both asserts
and `kGdsCounters`), C2 (render.cpp h16 against next-steps' VS lift: 539d45f's `vs_decomp` starts
`!s.object_motion && stage_lean && (vertex_input || vs_fetchless) && !s.tess_hw && ...`, and h15's
`ps_lift` has `!s.object_motion &&`; `ds_lift`/`ds_native` need nothing, object motion skips
tessellated draws), C3 (options.cpp h11 holds next-steps' fps_cap comment, reflowed), C4
(options.h h1's context is next-steps' comment with 120) and C5 (launcher.cpp h5's context is
`host_opt_set_index("camera_tuning", camera_tuning);`). Instrumented draws still run translated,
not lifted, shaders: Task 7 measures that cost.

Gone by construction: CMakeLists.txt (not touched) and src/engine/frame_rate.cpp (Reflex, not taken).
If Reflex is ported later: 539d45f keeps next-steps' `converted` and `if (converted) each_frame(f);`;
what is left to decide is the pacing wait's move above the host ticks and calling `on_frame_begin()`
only when the Reflex mode is not Off.

## D. When upstream merges PR #73 (or a variant)

1. Rebase `bbgoated/main` as usual; the (a1)-(d) commits conflict with upstream's own versions. Take
   upstream's side for every file in table A and every hunk in table B, then re-apply only the
   `// bbgoated:` edits and bbgoated's files: src/host/upscale*, fsr_upscaler_stub.cpp, sr_split*,
   shaders/fsr_reactive.comp, shaders/sr_merge.comp, and the DLAA edits of section E (E1-E10,
   host/dlaa_aa.*, tests/dlaa_aa_test.cpp, the pump-loop tick in main.cpp).
2. cmake/bbgoated-upscale.cmake skips sources and tests upstream already registers (`IN_LIST` /
   `if(NOT TARGET ...)`); delete what is redundant.
3. Re-run the Phase 2 verification (Task 15) and record the result in UPSTREAM.md.

## E. The DLAA already on bbgoated/main (R1): revert, port, re-apply

What is there, on the rebased bbgoated/main (original SHAs in brackets):

- ae0b56f (480d92d), the fork's ed58faa: dlaa.cpp/.h, dlaa_mv.comp/.spv.h, tools/dlaa_cbscan.py,
  docs/dlaa.md, tools/ngx_bridge (v1), .gitignore, and hooks in render.cpp, gpu.cpp, window.cpp and
  cmake/bbgoated.cmake; 8e42461 (c0ebd6c), the fork's fe06cb5: dlaa.cpp and docs/dlaa.md only.
- bbgoated's own: 6664c3f (db6d8a0) the hold and the BDA drop, abe5e6f (8f823fa) `host_opt_hold`,
  0717add (52020fe) the pump-loop tick, 414027f (13fd88a) the release when DLAA fails or idles and
  `BBHOST_TEST_DLAA_FAIL`. They touch dlaa.cpp/.h, host/dlaa_aa.*, tests/dlaa_aa_test.cpp,
  options.cpp/.h, main.cpp, cmake/bbgoated.cmake, README and SETUP.

**Step 1 - revert (Task 4, before anything of PR #73's).** Git writes each patch itself
(`--output`): a PowerShell pipe or `>` would re-encode it.

- `git diff ae0b56f~1 ae0b56f --output=<patch> -- src/host/render.cpp src/host/gpu.cpp src/host/window.cpp`,
  then `git apply -R --index <patch>` (applies as it is):
  - render.cpp, all 9 hunks: `#include "host/dlaa.h"`; `GfxPipeline::dlaa_anchor`;
    `resolve_stage_buffers`' `dlaa_cb_note_locked` and `dlaa_camera_note_locked` lines;
    `draw_impl`'s anchor block; `t_dlaa_main_draw = dlaa_main_draw_locked(...)`; `t_dlaa_cb_stage`
    around `resolve_stage_buffers`; the jitter block; `t_dlaa_main_draw = false;
    dlaa_after_draw_locked(r);`; `gpu::dlaa_request_cb_capture(g_capture_dir);`.
  - gpu.cpp, all 3 hunks: `#include "host/dlaa.h"`, `dlaa_add_instance_extensions(iext);`,
    `dlaa_add_device_extensions(g.phys, dext);`.
  - window.cpp, both hunks: `#include "host/dlaa.h"` and the Ctrl+F1-F8 keys. (6664c3f's tick in
    `host_window_pump` was removed again by 0717add: nothing of it is left.)
- `git diff 0717add~1 0717add --output=<tick patch> -- src/main.cpp`, then
  `git apply -R --index <tick patch>` (applies as it is): both hunks, the
  `#include "host/dlaa.h"` under `BBHOST_HAVE_SDL3` and the `gpu::dlaa_aa_tick()` call in
  `pump_until_done`.
- cmake/bbgoated.cmake, ae0b56f's hunk, by hand (6664c3f's block follows it, so `git apply -R` does
  not match): delete the blank line, the two `# DLAA (the YoucefNabil/bbhost fork ...` comment lines
  and the `if(Vulkan_FOUND) ... src/host/dlaa.cpp ... endif()` block. From (a2) on,
  cmake/bbgoated-upscale.cmake registers dlaa.cpp. 6664c3f's dlaa_aa block stays.
- Not reverted: .gitignore (h1 merges with `--3way`); dlaa.cpp, dlaa.h, dlaa_mv.*, dlaa_cbscan.py,
  docs/dlaa.md and tools/ngx_bridge's three files, which step 2 replaces whole; and bbgoated's own
  files and edits (host/dlaa_aa.*, tests/dlaa_aa_test.cpp, `host_opt_hold` in options.cpp/.h,
  README, SETUP).

**Step 2 - port:** (a1) `git checkout 539d45f -- tools/ngx_bridge`, .gitignore h1 with `--3way`;
(a2) the files and hunks of tables A and B with the edits of section C. render.cpp, gpu.cpp and
window.cpp's a2 subsets apply as they are once step 1 is done (before it, all three conflict).

**Step 3 - re-apply bbgoated's DLAA edits** on PR #73's files, each a marked block. Line numbers
are PR #73's `dlaa.cpp` at 539d45f, before the plan's edit 1 removes its first line.

- E1, includes (after `#include "host/dlaa.h"`, line 3): `#include "host/dlaa_aa.h"` and
  `#include "host/options.h"` (host_opt_hold, host_opt_release, host_opt_held, host_opt_serial).
- E2, at the top of `namespace gpu` (line 32, before `namespace {`): `g_dlaa_ngx_ok`, `g_dlaa_off`
  and `g_dlaa_idle`, the three `std::atomic<bool>` the pump loop's thread reads.
- E3, `init_ngx_locked` (after `g_s.ngx_ok = !ngx_failed(r);`, line 518):
  `g_dlaa_ngx_ok.store(g_s.ngx_ok, std::memory_order_release);`.
- E4, `State` (after `bool anchor_missing_said = false;`, line 187):
  `std::uint64_t last_eval_flip = 0;`.
- E5, `fail_locked` (after `g_s.failed = true;`, line 540): `g_dlaa_off.store(true, ...)`.
  PR #73's other `g_s.failed = true;` (line 1270, NGX init failed) gets the same store in Task 4:
  NGX is not up then, so nothing changes, but every "off for the session" is one signal.
- E6, `dlaa_after_draw_locked` (after the "600 flips of 3D frames without the anchor pipeline"
  block, which ends at line 994, inside the depth-write branch): 600 flips of 3D frames since
  `last_eval_flip` set `g_dlaa_idle` and log it once (the rule is `dlaa_idle()`, host/dlaa_aa.h,
  tested since Task 4).
- E7, `dlaa_anchor_locked` (after `if (g_s.failed) return;`, line 1225): the
  `BBHOST_TEST_DLAA_FAIL=<flip>` block, `fail_locked("BBHOST_TEST_DLAA_FAIL")` once DLAA has
  resolved frames (`dlaa_test_fail()`, host/dlaa_aa.h).
- E8, `dlaa_anchor_locked` (after `g_s.reset = false;`, line 1378): `g_s.last_eval_flip = flip;`
  and `g_dlaa_idle.store(false, ...)`. In (a2) only DLAA reaches this line; R7 moves the
  super-resolution case (Tasks 5, 6).
- E9, after the file's closing `}  // namespace gpu` (line 1402): a marked `namespace gpu` block
  with `reconstruction_running()` (`dlaa_active(config_on(), g_dlaa_ngx_ok, g_dlaa_off,
  g_dlaa_idle, g_s.enabled)`; R7), `dlaa_running()` calling it, and `dlaa_aa_tick()`, as on
  bbgoated/main (414027f's versions).
- E10, dlaa.h (after `namespace gpu {`): the `reconstruction_running()`, `dlaa_running()` and
  `dlaa_aa_tick()` declarations.
- main.cpp: `git apply --3way <tick patch>` (step 1's) puts the tick back; it applies beside
  PR #73's h1.
- Not re-applied: 6664c3f's `VK_EXT_buffer_device_address` strip (PR #73 strips it itself;
  Task 4's edit 4 logs it) and the fork's dlaa.cpp/dlaa.h text, which PR #73's files carry.

The net effect, checked in the dry run: window.cpp ends byte for byte as before step 1; gpu.cpp
gains PR #73's three lines (`host/fsr_upscaler.h`, h9, h14); render.cpp gains PR #73's
super-resolution HUD handling and the object-motion scope (+77 lines, one moved); main.cpp gains
h1's `display:` log line; every line bbgoated added to the fork's dlaa.cpp and dlaa.h is in the
new files (comments reworded to carry `bbgoated:`).

## F. Dry runs (2026-10-12)

Against `cloud/task-R-rebase` = 9cc8dc0 (bbgoated/main rebased onto 566184a), with
`scripts/dev/pr73_hunks.py` at its defaults; nothing was committed to bbgoated/main.

- **Each subset alone, strictly** (throwaway index, `git apply --cached --check`): every (b), (c)
  and (d) subset applies, and the (a2) subsets of the files the fork did not touch. render.cpp,
  gpu.cpp and window.cpp's (a2) subsets conflict until step 1's revert, and apply strictly after
  it. .gitignore h1 needs `--3way`.
- **The whole sequence in order** in a scratch worktree - step 1, (a1), (a2) with every edit of
  section C, step 3, (b), (c), (d): every hunk applied (only .gitignore by `--3way`), every by-hand
  anchor was found once, and no step left an unmerged file or a conflict marker.
- **Built and tested** (Linux, clang 18, `-DBBGOATED=ON`, upstream's CI flags) after (a2) and step 3,
  with the plan's Task 4 scaffolding (fsr_upscaler_stub.cpp, cmake/bbgoated-upscale.cmake with
  ngx_transfer_sync_test): ctest 63 of 63 passed, among them display_settings_test,
  dlaa_motion_test (on lavapipe: "ALL DLAA MOTION VECTOR TESTS PASSED"), ngx_transfer_sync_test,
  dlaa_aa_test, config_test and bbgoated_config_test. After (b), (c) and (d), with the plan's
  Task 5 and Task 7 test registrations: 66 of 66, presentation_coordinates_test,
  motion_history_test and object_motion_shader_test among them. The FSR provider was not built:
  its CMake switch is Task 6's, so (c)'s sources sat beside the stub. No warning comes from the
  port's files; gpu.cpp and render.cpp gain 5 and 1 `-Wmissing-field-initializers` of the kind
  they already have 123 and 94 of.

`docs/cloud-reports/task-2-dryrun.py` replays both (`--strict-only` for the first, the sequence
otherwise; `--tool` names the workspace's `scripts/dev/pr73_hunks.py`), so they can be repeated when
bbgoated/main moves before Task 4. The commands, their full output and the builds are in
`docs/cloud-reports/task-2.md`.
