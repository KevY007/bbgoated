#pragma once
// bbgoated: every Phase 3 switch (BBHOST_BBG_<name>) with a line of help, and
// the upstream switches a preset or [perf] may set - only those read after
// main() starts (lazily or at device creation; one read in a static
// initializer is fixed before the configuration exists). docs/bbgoated/SWITCHES.md
// documents each; tests/bbg_switches_doc_test.py keeps the two in step.
namespace bbg {
struct Switch {
    const char* name;  // without the BBHOST_BBG_ prefix
    const char* help;
};
inline constexpr Switch kSwitches[] = {
    {"PASS_BREAKS", "count render-pass ends and transfer batches by caller (diagnostic)"},
    {"PARAMS_MEM", "params ring: rebar (video memory the CPU writes) or device (a device-local twin filled before each submission)"},
    {"PT_MEM", "page tables: rebar"},
    {"LAZY_BARRIERS_NV", "upstream lazy pass barriers, on NVIDIA only, when BBHOST_PASS_BARRIERS is unset"},
    {"CB_SNAPSHOT", "constant buffers copied as recorded into a per-slot ring: rebar or device"},
    {"CB_DEDUP", "with CB_SNAPSHOT: identical snapshots in one recording share a copy"},
    {"CB_MAX_KB", "with CB_SNAPSHOT: the largest buffer copied, KiB (default 64, at most 1024)"},
    {"CB_SNAPSHOT_CS", "with CB_SNAPSHOT and CS_CB: compute constant buffers too"},
    {"FRAMES_AHEAD", "the command processor waits for the GPU to finish the frame N flips back (1-3)"},
    {"SHADOW_HOT", "buffer shadow hot rule: frames (copied in 3 of 60 flips) or never"},
    {"SHADOW_RETRY_S", "buffer shadow: a page gone to host memory for being rewritten is tried again after this many seconds"},
    {"SHADOW_PINGPONG", "buffer shadow: a page bounced between GPU writes and copies this many times goes to host memory"},
    {"SHADOW_FLOOR_MB", "buffer shadow: mirrors may always hold this much video memory"},
    {"SHADOW_AUDIT", "buffer shadow: host-memory binds by reason and guest range (diagnostic)"},
    {"FOLD_CLEARS", "clears done by the next pass's load op: depth (clears at a draw) or all (fills too)"},
    {"CS_CB", "compute constant buffers bound as storage buffers: bound"},
    {"ASYNC_SKIP", "a draw whose pipeline is compiling on a worker is skipped (bbport's rules)"},
    {"TEX_UPLOAD", "texture uploads: stream (recorder ops) or early (before the submission's work)"},
    {"FAULT_WINDOW_KB", "a write-watch fault releases the armed pages up to the end of this window"},
    {"PCORES", "the host's GPU-feeding threads on performance cores"},
    {"DYN_FIELDS", "library dynamic state recorded field by field (PR #100)"},
    {"BIND_DEDUP", "identical push descriptors, vertex buffers and bindless set binds left out (PR #100)"},
    {"CS_SET_MEMO", "compute descriptor sets reused by content within a slot (PR #100)"},
    {"PRECISE_BARRIERS", "narrower stage and access masks where the consumers are known"},
    {"DISPATCH_BARRIERS", "post-dispatch barriers: batch"},
    {"TRANSFER_BARRIERS", "transfer-batch end barriers: lazy"},
};
inline constexpr int kSwitchCount = static_cast<int>(sizeof(kSwitches) / sizeof(kSwitches[0]));
inline constexpr const char* kLateUpstream[] = {
    "BBHOST_PASS_BARRIERS",  "BBHOST_GPU_SLOTS",       "BBHOST_CP_BACKLOG",   "BBHOST_LIGHT_GRID_BASIS", "BBHOST_LIGHT_GRID",
    "BBHOST_SUBMIT_EVERY_JOB", "BBHOST_BINDLESS",      "BBHOST_PARAMS_WC",    "BBHOST_SHADOW_VRAM_MB",   "BBHOST_SHADOW_CUSHION_MB",
    "BBHOST_SHADOW_LIVE",    "BBHOST_VRAM_RESERVE_MB", "BBHOST_CB_REALIGN"};
}  // namespace bbg
