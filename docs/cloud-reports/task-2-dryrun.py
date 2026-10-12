#!/usr/bin/env python3
"""Dry runs of docs/bbgoated/pr73-port.md (task 2): PR #73's hunk subsets against a bbgoated tree.

    python3 -I task-2-dryrun.py <scratch worktree> [--base bbgoated/main] [--tool <pr73_hunks.py>]
                                [--strict-only] [--stop-after a2|b|c|d]

--strict-only checks each subset of table B alone against <base>, in a throwaway index, before
and after section E's step 1 (nothing in any worktree changes). Without it, the scratch worktree
(made with `git worktree add --detach <dir>`; its changes are thrown away) is reset to <base> and
the port is replayed in order: E step 1, (a1), (a2) with the edits of section C, E step 3, the
plan's Task 4 build scaffolding, (b), (c), (d). Each step is committed on the worktree's detached
HEAD, never pushed, so its tree can be built. Run where git and python3 are (Linux, or WSL on
the code repo's checkout). Written for the cloud run of 2026-10-12; see task-2.md.
"""
import argparse
import os
import subprocess
import sys
import tempfile
from pathlib import Path

HEAD = "539d45f"
ap = argparse.ArgumentParser()
ap.add_argument("worktree")
ap.add_argument("--base", default="bbgoated/main")
ap.add_argument("--tool", default=str(Path(__file__).resolve().parents[4] / "scripts" / "dev" / "pr73_hunks.py"))
ap.add_argument("--scratch", default="")
ap.add_argument("--strict-only", action="store_true")
ap.add_argument("--stop-after", default="d", choices=["a2", "b", "c", "d"])
ARGS = ap.parse_args()
TOOL = ARGS.tool
SCR = Path(ARGS.scratch or tempfile.mkdtemp(prefix="pr73-dry-"))
SCR.mkdir(parents=True, exist_ok=True)
WT = Path(ARGS.worktree)
BASE = ARGS.base
STOP = ARGS.stop_after
LOG = []
SETS = [(".gitignore", "a1", "1"), ("bbhost.example.toml", "a2", "1"), ("src/core/config.h", "a2", "1"),
        ("src/core/config.cpp", "a2", "1-3"), ("src/host/settings.h", "a2", "1,2"), ("src/host/options.cpp", "a2", "1,15"),
        ("src/main.cpp", "a2", "1"), ("tests/config_test.cpp", "a2", "1"), ("src/host/gpu.cpp", "a2", "6,9,14"),
        ("src/host/render.cpp", "a2", "1,2,21,22,24,25,26,28,29,30"), ("src/host/window.cpp", "a2", "24"),
        ("src/host/window.cpp", "b", "2,3,4,25,26,27,28,29,33,34"), ("src/host/window.h", "b", "1"),
        ("src/host/overlay.cpp", "b", "1-3"), ("src/host/overlay.h", "b", "1,2"), ("src/engine/live_resolution.cpp", "b", "1-4"),
        ("src/host/options.cpp", "b", "2,3,4,5,6,7,8,11,12,13,14,16"), ("src/host/options.h", "b", "1"),
        ("src/host/launcher.cpp", "b", "1-6"), ("src/host/gpu.cpp", "c", "8"), ("tools/win_package.sh", "c", "1"),
        ("src/gcn/translate.cpp", "d", "1-10"), ("src/gcn/translate.h", "d", "1,2"), ("src/host/translation_cache.cpp", "d", "1"),
        ("tests/translation_cache_test.cpp", "d", "1"), ("src/host/gpu_internal.h", "d", "1"), ("src/host/gpu.cpp", "d", "13"),
        ("src/host/render.cpp", "d", "3-20,23,27")]


def read(path):
    with open(path, encoding="utf-8", errors="surrogateescape", newline="") as f:
        return f.read()


def write(path, text):
    with open(path, "w", encoding="utf-8", errors="surrogateescape", newline="") as f:
        f.write(text)


def git(*args, check=True):
    r = subprocess.run(["git", "-C", str(WT)] + list(args), capture_output=True, text=True, encoding="utf-8", errors="replace")
    if check and r.returncode:
        raise SystemExit("git %s failed:\n%s%s" % (" ".join(args), r.stdout, r.stderr))
    return r


def say(line):
    LOG.append(line)
    print(line, flush=True)


def apply_hunks(path, hunks, step):
    patch = SCR / "seq.patch"
    subprocess.run([sys.executable, "-I", TOOL, "--repo", str(WT), "--file", path, "--hunks", hunks, "--out", str(patch)],
                   check=True, capture_output=True)
    strict = git("apply", "--check", str(patch), check=False).returncode == 0
    r = git("apply", "--3way", str(patch), check=False)
    unmerged = git("diff", "--name-only", "--diff-filter=U").stdout.split()
    state = ("applies" if strict else "applies with --3way only") if r.returncode == 0 and not unmerged else \
            "CONFLICT (%s)" % ", ".join(unmerged or ["apply failed: " + r.stderr.strip().splitlines()[-1]])
    say("  %-3s %-36s %-30s %s" % (step, path, hunks, state))
    return not unmerged and r.returncode == 0


def edit(path, old, new, step, what):
    f = WT / path
    text = read(f)
    n = text.count(old)
    if n != 1:
        say("  %-3s %-36s EDIT ANCHOR FOUND %d TIMES: %s" % (step, path, n, what))
        raise SystemExit(1)
    write(f, text.replace(old, new))
    say("  %-3s %-36s edit: %s" % (step, path, what))


def checkout(step, *paths):
    git("checkout", HEAD, "--", *paths)
    say("  %-3s from %s: %s" % (step, HEAD, " ".join(paths)))


def commit(step):
    git("add", "-A")
    left = git("diff", "--cached", "--name-only", "--diff-filter=U").stdout.split()
    markers = git("grep", "-n", "-E", "^(<<<<<<<|>>>>>>>)( |$)", "--cached", check=False).stdout
    check = git("diff", "--cached", "--check", check=False).stdout
    git("-c", "user.name=dry", "-c", "user.email=dry@dry", "commit", "-q", "--allow-empty", "-m", "dry: " + step)
    sha = git("rev-parse", "--short", "HEAD").stdout.strip()
    say("  == after %s: %s, unmerged %s, conflict markers %s, whitespace errors %s" %
        (step, sha, left or "none", "yes" if markers else "none", "yes:\n" + check if check else "none"))


def strict(label, pre=None):
    """Each subset alone against BASE (plus `pre`, a patch reverse-applied first) in a throwaway index."""
    env = dict(os.environ, GIT_INDEX_FILE=str(SCR / "strict.idx"))
    run = lambda *a: subprocess.run(["git", "-C", str(WT)] + list(a), capture_output=True, text=True, encoding="utf-8", errors="replace", env=env)
    run("read-tree", BASE)
    if pre:
        r = run("apply", "-R", "--cached", str(pre))
        say("strict, %s: %s" % (label, "ok" if r.returncode == 0 else "FAILED " + r.stderr))
    else:
        say("strict, %s" % label)
    for path, task, hunks in SETS:
        if pre and task != "a1" and task != "a2":
            continue
        patch = SCR / "strict.patch"
        subprocess.run([sys.executable, "-I", TOOL, "--repo", str(WT), "--file", path, "--hunks", hunks, "--out", str(patch)],
                       check=True, capture_output=True)
        r = run("apply", "--cached", "--check", str(patch))
        say("  %-3s %-36s %-30s %s" % (task, path, hunks, "applies" if r.returncode == 0 else
                                         "CONFLICT: " + (r.stderr.strip().splitlines() or ["?"])[0]))
    (SCR / "strict.idx").unlink()


if ARGS.strict_only:
    fork = git("log", "--format=%h", "-1", "--grep=^render: optional NVIDIA DLAA on the HDR scene", BASE).stdout.strip()
    pre = SCR / "strict-revert.patch"
    git("diff", "%s~1" % fork, fork, "--output=%s" % pre, "--", "src/host/render.cpp", "src/host/gpu.cpp", "src/host/window.cpp")
    strict("each subset alone against %s" % BASE)
    strict("a1/a2 after reverting %s's render.cpp, gpu.cpp and window.cpp hunks" % fork, pre)
    sys.exit(0)

# The sequence resets and cleans the worktree it is given: never the main checkout.
if Path(WT, git("rev-parse", "--git-dir").stdout.strip()).resolve() == \
        Path(WT, git("rev-parse", "--git-common-dir").stdout.strip()).resolve():
    raise SystemExit("refusing: %s is the repository's main checkout; give a `git worktree add --detach` directory" % WT)
git("reset", "-q", "--hard")
git("clean", "-q", "-fd")
git("checkout", "-q", "--detach", BASE)
say("base %s = %s" % (BASE, git("rev-parse", "--short", "HEAD").stdout.strip()))

# ---- R1 step 1: the DLAA fork's hunks out of render.cpp, gpu.cpp, window.cpp, main.cpp, cmake/bbgoated.cmake
fork = git("log", "--format=%h", "-1", "--grep=^render: optional NVIDIA DLAA on the HDR scene", BASE).stdout.strip()
tick = git("log", "--format=%h", "-1", "--grep=^bbg/upscale: DLAA's anti-aliasing hold runs in the pump loop", BASE).stdout.strip()
say("R1.1 revert (fork pick %s, pump-loop tick %s)" % (fork, tick))
for c, paths in ((fork, ["src/host/render.cpp", "src/host/gpu.cpp", "src/host/window.cpp"]), (tick, ["src/main.cpp"])):
    patch = SCR / ("revert-%s.patch" % c)
    git("diff", "%s~1" % c, c, "--output=%s" % patch, "--", *paths)
    r = git("apply", "-R", "--index", str(patch), check=False)
    say("  R1  git diff %s~1 %s -- %s, git apply -R: %s" % (c, c, " ".join(paths), "ok" if r.returncode == 0 else "FAILED " + r.stderr))
edit("cmake/bbgoated.cmake",
     "\n# DLAA (the YoucefNabil/bbhost fork, src/host/dlaa.*): NVIDIA DLAA through the runtime-loaded\n"
     "# ngx_bridge.dll, wherever the Vulkan renderer is built.\n"
     "if(Vulkan_FOUND)\n"
     "  target_sources(bbhost PRIVATE src/host/dlaa.cpp)\n"
     "  set_source_files_properties(src/host/dlaa.cpp PROPERTIES COMPILE_OPTIONS \"-Wno-missing-field-initializers\")\n"
     "endif()\n", "", "R1", "delete the fork's dlaa.cpp block (cmake/bbgoated-upscale.cmake registers it)")
commit("R1.1 revert")

# ---- (a1) the NGX bridge
say("a1")
checkout("a1", "tools/ngx_bridge")
apply_hunks(".gitignore", "1", "a1")
commit("a1")

# ---- (a2) the DLSS evaluation path
say("a2")
checkout("a2", "src/host/dlaa.cpp", "src/host/dlaa.h", "src/host/display_settings.h", "src/host/fsr_upscaler.h",
         "src/host/object_motion.cpp", "src/host/object_motion.h", "src/host/motion_history.h",
         "src/host/shaders/dlaa_mv.comp", "src/host/shaders/dlaa_mv.spv.h", "tools/dlaa_cbscan.py", "docs/dlaa.md",
         "tests/display_settings_test.cpp", "tests/dlaa_motion_test.cpp")
for path, hunks in (("src/host/render.cpp", "1,2,21,22,24,25,26,28,29,30"), ("src/host/gpu.cpp", "6,9,14"),
                    ("src/host/window.cpp", "24"), ("src/core/config.h", "1"), ("src/core/config.cpp", "1-3"),
                    ("src/host/settings.h", "1,2"), ("src/host/options.cpp", "1,15"), ("src/main.cpp", "1"),
                    ("tests/config_test.cpp", "1"), ("bbhost.example.toml", "1")):
    apply_hunks(path, hunks, "a2")
# The plan's Task 4 hand edits (PR #73's hunks taken in part).
edit("src/host/render.cpp",
     "        if (s.object_motion) {\n"
     "            params.motion_positions = object_motion_positions_bda();\n"
     "            params.motion_store = object_allocation.store;\n"
     "            params.motion_load = object_allocation.load;\n"
     "            params.motion_vertices = object_allocation.vertices;\n"
     "            params.motion_first_vertex = object_allocation.first_vertex;\n"
     "            params.motion_instances = object_allocation.instances;\n"
     "            params.motion_first_instance = object_allocation.first_instance;\n"
     "            params.motion_flags = object_allocation.flags;\n"
     "            params.motion_scale[0] = s.vport[0];\n"
     "            params.motion_scale[1] = s.vport[2];\n"
     "        }\n", "", "a2", "h25's object-motion params block out until (d)")
edit("src/host/gpu.cpp", '#include "core/portable.h"\n',
     '#include "core/portable.h"\n#include "host/dlaa.h"\n#include "host/fsr_upscaler.h"\n', "a2", "h1's two includes (not reflex.h)")
edit("src/host/gpu.cpp", "        g.fifo_latest_ready_ext = nullptr;\n    }\n",
     "        g.fifo_latest_ready_ext = nullptr;\n    }\n    dlaa_add_device_extensions(g.phys, dext);\n", "a2",
     "h7's first line (not the derivatives block)")
edit("src/host/window.cpp", '#include "host/bindings.h"\n', '#include "host/bindings.h"\n#include "host/dlaa.h"\n',
     "a2", "h1's dlaa.h include")
edit("src/core/config.cpp",
     '    if (const char* e = std::getenv("BBHOST_FG_BACKEND")) {\n'
     '        if (std::strcmp(e, "dlss") == 0 || std::strcmp(e, "fsr3") == 0)\n'
     '            c.frame_generation_backend = e;\n'
     '    }\n', "", "a2", "h2's BBHOST_FG_BACKEND block out")
edit("src/core/config.cpp",
     '           "# Frame generation backend: dlss, fsr3\\n"\n'
     '           "frame_generation_backend = \\"dlss\\"\\n"\n'
     '           "# DLSS Frame Generation requires nvngx_dlssg.dll and supported hardware.\\n"\n'
     '           "# This generates displayed frames without changing the gameplay rate.\\n"\n'
     '           "frame_generation = false\\n"\n'
     '           "frame_generation_factor = 2\\n"\n'
     '           "# NVIDIA Reflex: off, on, boost. Unsupported devices retain ordinary pacing.\\n"\n'
     '           "reflex = \\"on\\"\\n"\n', "", "a2", "h3's frame-generation and Reflex template lines out")
edit("bbhost.example.toml",
     'frame_generation_backend = "dlss" # dlss or fsr3 (2x only)\n', "", "a2", "frame_generation_backend line out")
edit("bbhost.example.toml",
     "frame_generation = false # The game still simulates at its own rate\n"
     "frame_generation_factor = 2 # 2, 3 or 4; unsupported higher factors fall back to 2\n", "", "a2",
     "frame_generation lines out")
# The plan's Task 4 Step 2: four marked edits in PR #73's dlaa.cpp.
edit("src/host/dlaa.cpp", '#include "host/reflex.h"\n// See dlaa.h.\n', "// See dlaa.h.\n", "a2", "edit 1: no reflex.h")
edit("src/host/dlaa.cpp",
     "    if (g_bridge.fg_extensions && !ngx_failed(g_bridge.fg_extensions(VK_NULL_HANDLE, VK_NULL_HANDLE,\n",
     "    // bbgoated: frame generation's NGX extensions only when it is requested (it is not ported).\n"
     "    if (fg_requested() && g_bridge.fg_extensions && !ngx_failed(g_bridge.fg_extensions(VK_NULL_HANDLE, VK_NULL_HANDLE,\n",
     "a2", "edit 2a: FG instance extensions only when requested")
edit("src/host/dlaa.cpp",
     "    if (g_bridge.fg_extensions && !ngx_failed(g_bridge.fg_extensions(g.instance, phys,\n",
     "    // bbgoated: frame generation's NGX extensions only when it is requested (it is not ported).\n"
     "    if (fg_requested() && g_bridge.fg_extensions && !ngx_failed(g_bridge.fg_extensions(g.instance, phys,\n",
     "a2", "edit 2b: FG device extensions only when requested")
edit("src/host/dlaa.cpp", "        guides.reflex_frame_id = host::reflex::cp_thread_frame_id();\n",
     "        guides.reflex_frame_id = 0;  // bbgoated: Reflex is not ported\n", "a2", "edit 3: no Reflex frame id")
edit("src/host/dlaa.cpp",
     "    exts.erase(std::remove_if(exts.begin(), exts.end(), [](const char* name) {\n"
     "        return !std::strcmp(name, \"VK_EXT_buffer_device_address\");\n"
     "    }), exts.end());\n",
     "    const auto before = exts.size();\n"
     "    exts.erase(std::remove_if(exts.begin(), exts.end(), [](const char* name) {\n"
     "        return !std::strcmp(name, \"VK_EXT_buffer_device_address\");\n"
     "    }), exts.end());\n"
     "    // bbgoated: say so - add_missing logged it as added a moment ago.\n"
     "    if (exts.size() != before)\n"
     "        host_log(\"dlaa: VK_EXT_buffer_device_address dropped: the core bufferDeviceAddress feature is on \"\n"
     "                 \"(VUID-VkDeviceCreateInfo-pNext-04748)\");\n",
     "a2", "edit 4: the BDA strip says so")
commit("a2 (PR #73's DLAA path)")

# ---- R1 step 3: bbgoated's DLAA edits on PR #73's dlaa.cpp / dlaa.h, and the pump-loop tick
say("R1.3 re-apply")
edit("src/host/dlaa.cpp", '// See dlaa.h.\n#include "host/dlaa.h"\n',
     '// See dlaa.h.\n#include "host/dlaa.h"\n#include "host/dlaa_aa.h"  // bbgoated: the anti-aliasing hold\n'
     '#include "host/options.h"  // bbgoated: host_opt_hold, host_opt_release\n', "R1", "E1 includes")
edit("src/host/dlaa.cpp", "namespace gpu {\nnamespace {\n",
     "namespace gpu {\n\n"
     "// bbgoated: the anti-aliasing hold's inputs for the pump loop's thread (dlaa_running):\n"
     "// NGX started, DLAA failed for the session after that (fail_locked), and DLAA idle -\n"
     "// ten seconds of 3D frames since the last frame it resolved.\n"
     "std::atomic<bool> g_dlaa_ngx_ok{false};\n"
     "std::atomic<bool> g_dlaa_off{false};\n"
     "std::atomic<bool> g_dlaa_idle{false};\n\n"
     "namespace {\n", "R1", "E2 the three atomics")
edit("src/host/dlaa.cpp", "    g_s.ngx_ok = !ngx_failed(r);\n",
     "    g_s.ngx_ok = !ngx_failed(r);\n"
     "    g_dlaa_ngx_ok.store(g_s.ngx_ok, std::memory_order_release);  // bbgoated: for dlaa_running\n", "R1",
     "E3 NGX's result published")
edit("src/host/dlaa.cpp", "    bool anchor_missing_said = false;\n",
     "    bool anchor_missing_said = false;\n"
     "    std::uint64_t last_eval_flip = 0;  // bbgoated: the flip of the last frame DLAA resolved\n", "R1",
     "E4 State::last_eval_flip")
edit("src/host/dlaa.cpp", "    release_locked();\n    g_s.failed = true;\n}\n",
     "    release_locked();\n    g_s.failed = true;\n"
     "    g_dlaa_off.store(true, std::memory_order_release);  // bbgoated: the anti-aliasing hold lets go\n}\n", "R1",
     "E5 fail_locked publishes the failure")
edit("src/host/dlaa.cpp",
     "                host_log(\"dlaa: 600 flips of 3D frames without the anchor pipeline; DLAA is not running (BBHOST_DLAA_ANCHOR names another)\");\n"
     "            }\n        }\n",
     "                host_log(\"dlaa: 600 flips of 3D frames without the anchor pipeline; DLAA is not running (BBHOST_DLAA_ANCHOR names another)\");\n"
     "            }\n        }\n"
     "        // bbgoated: 3D frames for ten seconds since the last DLAA frame (no anchor,\n"
     "        // or none with its scene and depth): DLAA is idle, and the game's\n"
     "        // anti-aliasing is not held off for it until DLAA resolves frames again.\n"
     "        if (g_s.last_eval_flip && slot->flip > g_s.last_eval_flip + 600 && !g_dlaa_idle.load(std::memory_order_relaxed)) {\n"
     "            g_dlaa_idle.store(true, std::memory_order_relaxed);\n"
     "            host_log(\"dlaa: 600 flips of 3D frames since the last DLAA frame; the game's anti-aliasing is not held off until DLAA runs again\");\n"
     "        }\n", "R1", "E6 idle after 600 flips")
edit("src/host/dlaa.cpp", "    for (State::Depth& d : g_s.depths) d.writes = 0;\n    if (g_s.failed) return;\n",
     "    for (State::Depth& d : g_s.depths) d.writes = 0;\n    if (g_s.failed) return;\n"
     "    // bbgoated: BBHOST_TEST_DLAA_FAIL=<flip> fails DLAA for the session at that flip once\n"
     "    // it has resolved frames - the anti-aliasing hold's release is tested with it.\n"
     "    static const std::uint64_t test_fail_flip = [] {\n"
     "        const char* e = std::getenv(\"BBHOST_TEST_DLAA_FAIL\");\n"
     "        return e ? std::strtoull(e, nullptr, 10) : 0ull;\n"
     "    }();\n"
     "    if (test_fail_flip && flip >= test_fail_flip && g_s.evaluations) {\n"
     "        fail_locked(\"BBHOST_TEST_DLAA_FAIL\");\n"
     "        return;\n"
     "    }\n", "R1", "E7 BBHOST_TEST_DLAA_FAIL")
edit("src/host/dlaa.cpp", "    scene->fill_last = false;\n    g_s.reset = false;\n",
     "    scene->fill_last = false;\n    g_s.reset = false;\n"
     "    g_s.last_eval_flip = flip;  // bbgoated: DLAA resolved this frame (not idle)\n"
     "    g_dlaa_idle.store(false, std::memory_order_relaxed);\n", "R1", "E8 the idle clock restarts")
dlaa = WT / "src/host/dlaa.cpp"
text = read(dlaa)
assert text.endswith("}  // namespace gpu\n")
write(dlaa, text + """
// bbgoated: the game's anti-aliasing held off while DLAA runs (host/dlaa_aa.h).
namespace gpu {

bool dlaa_running() {
    return dlaa_active(config_on(), g_dlaa_ngx_ok.load(std::memory_order_acquire), g_dlaa_off.load(std::memory_order_acquire),
                       g_dlaa_idle.load(std::memory_order_relaxed), g_s.enabled.load(std::memory_order_relaxed));
}

// The pump loop's thread, every pass. While DLAA runs, the game's own
// anti-aliasing is held off; it comes back when DLAA is turned off, if the
// player had it on (host/dlaa_aa.h). Looked at only when DLAA or a setting changed.
void dlaa_aa_tick() {
    static bool last_active = false;
    static std::uint64_t last_serial = 0;
    const bool active = dlaa_running();
    const std::uint64_t serial = host_opt_serial();
    if (active == last_active && serial == last_serial) return;
    last_active = active;
    switch (dlaa_aa_step(active, host_opt_get("anti_alias"), host_opt_held("anti_alias"))) {
    case kDlaaAaHold:
        host_opt_hold("anti_alias", false);
        host_log("dlaa: the game's anti-aliasing is off while DLAA runs (your setting is kept; it is back when DLAA is turned off)");
        break;
    case kDlaaAaRelease:
        host_opt_release("anti_alias");
        host_log("dlaa: the game's anti-aliasing is back to your setting (DLAA off)");
        break;
    case kDlaaAaKeep:
        break;
    }
    last_serial = host_opt_serial();  // our own change is not news
}

}  // namespace gpu
""")
say("  R1  %-36s edit: E9 dlaa_running and dlaa_aa_tick appended" % "src/host/dlaa.cpp")
edit("src/host/dlaa.h", "namespace gpu {\n\nstruct DrawRec;\n",
     "namespace gpu {\n\n"
     "// bbgoated: whether DLAA runs now (enabled, NGX ready, not failed or idle, not switched off with Ctrl+F1).\n"
     "bool dlaa_running();\n"
     "// bbgoated, main.cpp's pump loop (with or without a window): the game's anti-aliasing held off while DLAA runs (host/dlaa_aa.h).\n"
     "void dlaa_aa_tick();\n\n"
     "struct DrawRec;\n", "R1", "E10 declarations")
r = git("apply", "--3way", str(SCR / ("revert-%s.patch" % tick)), check=False)
say("  R1  the tick patch, git apply --3way: %s" % ("ok" if r.returncode == 0 else "FAILED " + r.stderr))
commit("R1.3 re-apply")

# ---- build scaffolding: the plan's Task 4 Steps 3-4 (bbgoated files), so the tree after a2 builds
say("a2 scaffolding (plan Task 4 Steps 3-4)")
write(WT / "src/host/fsr_upscaler_stub.cpp", """// SPDX-License-Identifier: GPL-3.0-or-later
// bbgoated: the FSR entry points dlaa.cpp and gpu.cpp call, for a build without the
// FSR provider (-DBBGOATED_FSR=OFF, or before Task 6 of the Phase 2 plan). FSR reports
// itself unavailable, and the temporal path keeps DLSS or the game's own picture.
#include "host/fsr_upscaler.h"
#include "log.h"

namespace gpu {

bool fsr_upscale_locked(VkPhysicalDevice, VkDevice, VkCommandBuffer, const NgxbImage&, const NgxbImage&,
                        const DlssFgGuides&, uint64_t, uint64_t) {
    static bool said = false;
    if (!said) {
        said = true;
        host_log("fsr: this build has no FSR provider (-DBBGOATED_FSR=OFF); FSR modes render unchanged");
    }
    return false;
}

void fsr_retire_locked(uint64_t) {}

void fsr_shutdown_locked() {}

}  // namespace gpu
""")
write(WT / "cmake/bbgoated-upscale.cmake", """# bbgoated (docs/bbgoated/pr73-port.md): PR #73's upscaler sources and tests that bbgoated
# takes, and bbgoated's own upscaler files. Each block steps aside when upstream's
# CMakeLists.txt already registers the source or target (a later upstream merge of PR #73),
# so that merge only deletes lines here. CMakeLists.txt itself is not touched.
if(Vulkan_FOUND)
  get_target_property(_bbg_sources bbhost SOURCES)
  foreach(_src src/host/dlaa.cpp src/host/object_motion.cpp)
    if(NOT _src IN_LIST _bbg_sources)
      target_sources(bbhost PRIVATE ${_src})
      set_source_files_properties(${_src} PROPERTIES COMPILE_OPTIONS "-Wno-missing-field-initializers")
    endif()
  endforeach()
  # The FSR provider arrives in Task 6; until then, and with -DBBGOATED_FSR=OFF, a stub.
  target_sources(bbhost PRIVATE src/host/fsr_upscaler_stub.cpp)
endif()

# Unit tests, in Linux test builds beside upstream's (its enable_testing() is UNIX-only).
if(TARGET config_test)
  if(NOT TARGET display_settings_test)
    add_executable(display_settings_test tests/display_settings_test.cpp)
    target_include_directories(display_settings_test PRIVATE src)
    target_compile_options(display_settings_test PRIVATE -UNDEBUG)
    add_test(NAME display_settings_test COMMAND display_settings_test)
  endif()
  if(Vulkan_FOUND AND NOT TARGET dlaa_motion_test)
    add_executable(dlaa_motion_test tests/dlaa_motion_test.cpp)
    target_include_directories(dlaa_motion_test PRIVATE src)
    target_link_libraries(dlaa_motion_test PRIVATE Vulkan::Vulkan)
    add_test(NAME dlaa_motion_test COMMAND dlaa_motion_test)
    set_tests_properties(dlaa_motion_test PROPERTIES SKIP_RETURN_CODE 77)
  endif()
  if(Vulkan_FOUND AND NOT TARGET ngx_transfer_sync_test)
    add_executable(ngx_transfer_sync_test tools/ngx_bridge/ngx_transfer_sync_test.cpp)
    find_package(Threads REQUIRED)
    target_link_libraries(ngx_transfer_sync_test PRIVATE Vulkan::Vulkan Threads::Threads)
    add_test(NAME ngx_transfer_sync_test COMMAND ngx_transfer_sync_test)
  endif()
endif()
""")
cm = WT / "cmake/bbgoated.cmake"
write(cm, read(cm) + "\n# Phase 2: the upscalers (PR #73's parts and bbgoated's own), in a file of their own.\n"
              "include(${CMAKE_CURRENT_LIST_DIR}/bbgoated-upscale.cmake)\n")
say("  scaffold: src/host/fsr_upscaler_stub.cpp, cmake/bbgoated-upscale.cmake, include line in cmake/bbgoated.cmake")
commit("a2 scaffolding")
if STOP == "a2":
    sys.exit(0)

# ---- (b) presentation at the output size and the settings rows
say("b")
checkout("b", "src/host/presentation_coordinates.h", "tests/presentation_coordinates_test.cpp")
for path, hunks in (("src/host/window.cpp", "2,3,4,25,26,27,28,29,33,34"), ("src/host/window.h", "1"),
                    ("src/host/overlay.cpp", "1-3"), ("src/host/overlay.h", "1,2"), ("src/engine/live_resolution.cpp", "1-4"),
                    ("src/host/options.cpp", "2,3,4,5,6,7,8,11,12,13,14,16"), ("src/host/options.h", "1"),
                    ("src/host/launcher.cpp", "1-6")):
    apply_hunks(path, hunks, "b")
edit("src/host/window.cpp", '#include "host/present_pass.h"\n',
     '#include "host/present_pass.h"\n#include "host/presentation_coordinates.h"\n', "b", "h1's presentation_coordinates.h")
edit("src/host/window.cpp", "    const auto t_locked = std::chrono::steady_clock::now();\n",
     "    const auto t_locked = std::chrono::steady_clock::now();\n"
     "    // bbgoated (PR #73's output-size presentation, at 539d45f in present_swapchain_image and\n"
     "    // vk_present): a frame reconstructed to the output size is shown at that size, and the\n"
     "    // pointer and the overlay follow it. Read here, under the renderer lock this function\n"
     "    // already holds - PR #73 takes it once more before the acquire.\n"
     "    gpu::dlss_sr_display_dimensions_locked(display_va, &display_w, &display_h);\n"
     "    {\n"
     "        std::lock_guard<std::mutex> ml(g_mouse_mu);\n"
     "        if (g_display_w != display_w || g_display_h != display_h) {\n"
     "            g_display_w = display_w;\n"
     "            g_display_h = display_h;\n"
     "            mouse_remap_locked();  // a pointer that did not move, mapped again: no motion is invented\n"
     "        }\n"
     "    }\n", "b", "the SR display size under vk_present's lock (h15's intent)")
edit("src/host/window.cpp",
     "        {\n"
     "            // What the pointer's window position scales by.\n"
     "            std::lock_guard<std::mutex> ml(g_mouse_mu);\n"
     "            g_display_w = display_w;\n"
     "            g_display_h = display_h;\n"
     "        }\n", "", "b", "h35's removal of host_present's g_display store")
commit("b")
if STOP == "b":
    sys.exit(0)

# ---- (c) FSR
say("c")
checkout("c", "cmake/fsr.cmake", "cmake/fsr-clear-order.patch", "cmake/fsr-debug-view.patch",
         "cmake/fsr-frame-retirement.patch", "LICENSE-FSR.txt", "src/host/fsr_provider.cpp", "src/host/fsr_upscaler.cpp",
         "tools/win/Install-FSR4.ps1", "tools/win/Install-FSR4.bat", "docs/fsr.md")
for path, hunks in (("src/host/gpu.cpp", "8"), ("tools/win_package.sh", "1")):
    apply_hunks(path, hunks, "c")
commit("c")
if STOP == "c":
    sys.exit(0)

# ---- (d) object motion
say("d")
checkout("d", "tests/motion_history_test.cpp", "tests/object_motion_shader_test.cpp")
for path, hunks in (("src/gcn/translate.cpp", "1-10"), ("src/gcn/translate.h", "1,2"),
                    ("src/host/translation_cache.cpp", "1"), ("tests/translation_cache_test.cpp", "1"),
                    ("src/host/gpu_internal.h", "1"), ("src/host/gpu.cpp", "13"), ("src/host/render.cpp", "3-20,23,27")):
    apply_hunks(path, hunks, "d")
edit("src/host/render.cpp", "        params.l1_table = g.l1.address;\n",
     "        params.l1_table = g.l1.address;\n"
     "        if (s.object_motion) {\n"
     "            params.motion_positions = object_motion_positions_bda();\n"
     "            params.motion_store = object_allocation.store;\n"
     "            params.motion_load = object_allocation.load;\n"
     "            params.motion_vertices = object_allocation.vertices;\n"
     "            params.motion_first_vertex = object_allocation.first_vertex;\n"
     "            params.motion_instances = object_allocation.instances;\n"
     "            params.motion_first_instance = object_allocation.first_instance;\n"
     "            params.motion_flags = object_allocation.flags;\n"
     "            params.motion_scale[0] = s.vport[0];\n"
     "            params.motion_scale[1] = s.vport[2];\n"
     "        }\n", "d", "h25's object-motion params block back")
commit("d")
