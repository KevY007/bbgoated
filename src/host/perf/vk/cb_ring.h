#pragma once
// bbgoated (BBHOST_BBG_CB_SNAPSHOT=rebar|device): constant buffers copied as
// the draw is recorded into a ring each submission slot owns - video memory
// the CPU writes (rebar), or host staging copied into device memory ahead of
// the submission's work (device) - instead of read from guest memory when the
// GPU runs (report A R2). host/perf/core/cb_ring_core.h holds the rules.
// Under g.mu.
#include "host/gpu_internal.h"

namespace gpu {
bool cb_snapshot_on();
// [base, base + bytes) as this point in the stream has it (copy versions laid
// over), bound from the ring. False - and `out` untouched - when not taken:
// off, larger than BBHOST_BBG_CB_MAX_KB, not mapped CPU-readable, a GPU write
// to it still pending (unless `inside_version`: the version's bytes are the
// source), no room, or a dispatch's without BBHOST_BBG_CB_SNAPSHOT_CS.
bool cb_snapshot_locked(std::uint64_t base, std::uint64_t bytes, bool inside_version, bool dispatch, VkDescriptorBufferInfo& out);
// The GPU-write registry, which BBHOST_BBG_TEX_UPLOAD=early asks too. A write
// on the GPU's timeline, recorded into the submission under way:
void cb_ring_gpu_wrote_locked(std::uint64_t va, std::uint64_t bytes);
// a shader stage's buffer stores - one that does not trace to a V# could write
// anywhere - registered before the stage's bindings resolve. Each V# is found
// as the stage's bindings are: `resolve` with the GX draw's records (render.cpp's
// resolve_binding), or through `user` alone as a dispatch's stores are:
using StoreResolve = bool (*)(const gcn::ResourcePath& path, const std::uint32_t* user, const GxStageRecords* gx, int ndw, std::uint32_t* out);
void cb_ring_stage_writes_locked(const gcn::TranslateResult& meta, const std::uint32_t* user, const GxStageRecords* gx = nullptr,
                                 StoreResolve resolve = nullptr);
// whether a submission that has not run yet writes into the range.
bool cb_ring_gpu_write_pending_locked(std::uint64_t va, std::uint64_t bytes);
// retire_slot_locked, last: the slot's chunks are free again; finished writes forgotten.
void cb_ring_slot_retired_locked(int slot);
}  // namespace gpu
