// See cb_ring.h.
#include "host/perf/vk/cb_ring.h"

#include "host/perf/bbg_perf.h"
#include "host/perf/core/cb_ring_core.h"
#include "host/perf/vk/device_mem.h"
#include "log.h"

#include <algorithm>
#include <string>
#include <vector>

namespace gpu {
namespace {
enum Mode { kOff, kRebar, kDevice };
int mode() {
    static const int m = [] {
        const std::string v = bbg::perf_value("CB_SNAPSHOT");
        return v == "rebar" ? kRebar : v == "device" ? kDevice : kOff;
    }();
    return m;
}
std::uint64_t max_bytes() {
    static const std::uint64_t b = std::min<std::uint64_t>(bbg::perf_uint("CB_MAX_KB", 64), 1024) << 10;
    return b;
}
bool dedup_on() {
    static const bool v = bbg::perf_on("CB_DEDUP");
    return v;
}
bool compute_on() {
    static const bool v = bbg::perf_on("CB_SNAPSHOT_CS");
    return v;
}
// The GPU-write registry runs for the snapshots and for BBHOST_BBG_TEX_UPLOAD=early's
// uploads, which ask the same question of a texture's bytes.
bool tracked() {
    static const bool on = mode() != kOff || bbg::perf_value("TEX_UPLOAD") == "early";
    return on;
}

constexpr std::uint64_t kChunkBytes = 4ull << 20;
struct RingChunk {
    DevBuffer host;  // rebar: the ring itself; device: the staging the copy reads
    DevBuffer dev;   // device: what shaders read
};
bbg::SlotRing<RingChunk> g_ring(kSlots, kChunkBytes);
bbg::GpuWrites g_writes;
bbg::SnapshotDedup g_dedup;
std::uint64_t g_dedup_serial = ~0ull;
std::vector<std::uint8_t> g_stage;  // with dedup: the snapshot built in cached memory, hashed, then copied
struct Stats {
    std::uint64_t bound = 0, bytes = 0, shared = 0, gpu_pending = 0, anywhere = 0, large = 0, unmapped = 0, full = 0, dispatch = 0;
} g_stats;
std::uint64_t g_next = 0, g_from_flip = 0;

std::uint64_t cap_chunks() {
    static const std::uint64_t c = [] {
        if (mode() != kRebar) return std::uint64_t{64};  // 256 MiB of staging and of device memory, at most
        const std::uint32_t t = bbg_rebar_type(~0u);
        if (t == UINT32_MAX) return std::uint64_t{64};
        const std::uint64_t heap = g.mem_props.memoryHeaps[g.mem_props.memoryTypes[t].heapIndex].size;
        return std::clamp<std::uint64_t>(heap / 4 / kChunkBytes, 4, 64);  // at most a quarter of the BAR heap
    }();
    return c;
}

bool make_chunk(RingChunk& c) {
    if (mode() == kRebar) return create_dev_buffer(c.host, kChunkBytes, true, false, true);
    if (!create_dev_buffer(c.host, kChunkBytes, true)) return false;
    if (create_dev_buffer(c.dev, kChunkBytes, false)) return true;
    defer_destroy(c.host);
    c.host = DevBuffer{};
    return false;
}

// device: this recording's chunks into device memory, ahead of its work (host/perf/vk/device_mem.cpp).
void presubmit() {
    if (mode() != kDevice) return;
    for (const auto& h : g_ring.held(g.slot)) bbg_presubmit_add_locked(h.chunk.host.buffer, h.chunk.dev.buffer, 0, 0, h.used);
}

void report() {
    if (!bbg::perf_report_due(g_next)) return;
    const std::uint64_t flip = hle_video_flip_count();
    const double f = static_cast<double>(flip > g_from_flip ? flip - g_from_flip : 1);
    host_log("bbg cb snapshot (%s): %.0f/flip bound (%.2f MiB/flip, %.0f shared); in place: gpu-written %.0f (%.0f behind a store "
             "of no known range), larger %.0f, unmapped %.0f, ring full %.0f, dispatches %.0f; ring %llu chunks",
             mode() == kRebar ? "rebar" : "device", g_stats.bound / f, g_stats.bytes / f / (1 << 20), g_stats.shared / f,
             g_stats.gpu_pending / f, g_stats.anywhere / f, g_stats.large / f, g_stats.unmapped / f, g_stats.full / f,
             g_stats.dispatch / f, static_cast<unsigned long long>(g_ring.made()));
    g_stats = Stats{};
    g_from_flip = flip;
}
}  // namespace

bool cb_snapshot_on() { return mode() != kOff; }

bool cb_snapshot_locked(std::uint64_t base, std::uint64_t bytes, bool inside_version, bool dispatch, VkDescriptorBufferInfo& out) {
    if (mode() == kOff) return false;
    static const bool presubmit_registered = bbg_presubmit_register(&presubmit);
    (void)presubmit_registered;
    report();
    bbg::SnapshotAsk q;
    q.bytes = bytes;
    q.max_bytes = max_bytes();
    q.dispatch = dispatch;
    q.dispatch_on = compute_on();
    q.inside_version = inside_version;
    const bool sized = bytes && bytes <= q.max_bytes;  // the two below are asked only of what could be copied
    q.gpu_pending = sized && g_writes.pending(base, bytes, g.completed_submits);
    // Read on the CPU, so mapped with CPU read access: memory the guest mapped
    // for the GPU alone is no-access on the host, and a copy of it would fault.
    q.mapped = sized && hle_kernel_va_readable(base, bytes);
    switch (bbg::snapshot_refusal(q)) {
        case bbg::kSnapDispatch: return ++g_stats.dispatch, false;
        case bbg::kSnapLarger: return ++g_stats.large, false;
        case bbg::kSnapGpuWritten:
            if (g_writes.pending_anywhere(g.completed_submits)) ++g_stats.anywhere;
            return ++g_stats.gpu_pending, false;
        case bbg::kSnapUnmapped: return ++g_stats.unmapped, false;
        default: break;
    }
    begin_recording_locked();  // the copy belongs to the recording that binds it
    const auto* src = reinterpret_cast<const std::uint8_t*>(static_cast<std::uintptr_t>(base));
    std::uint64_t hash = 0;
    if (dedup_on()) {
        if (g_dedup_serial != g.record_serial) {
            g_dedup.reset();
            g_dedup_serial = g.record_serial;
        }
        g_stage.assign(src, src + bytes);
        copy_versions_overlay_locked(base, bytes, g_stage.data());
        hash = bbg::hash_bytes(g_stage.data(), bytes);
        bbg::SnapshotDedup::Where w{};
        if (g_dedup.find(base, bytes, hash, w)) {
            const RingChunk& c = g_ring.held(g.slot)[static_cast<std::size_t>(w.chunk)].chunk;
            out = {mode() == kDevice ? c.dev.buffer : c.host.buffer, w.offset, bytes};
            ++g_stats.shared;
            return true;
        }
    }
    int index = 0;
    std::uint64_t at = 0;
    const std::uint64_t align = std::max<std::uint64_t>(256, g.ssbo_align);
    if (!g_ring.alloc(g.slot, bytes, align, cap_chunks(), make_chunk, index, at)) return ++g_stats.full, false;
    const RingChunk& c = g_ring.held(g.slot)[static_cast<std::size_t>(index)].chunk;
    auto* dst = static_cast<std::uint8_t*>(c.host.map) + at;
    if (dedup_on()) {
        std::memcpy(dst, g_stage.data(), bytes);
        g_dedup.add(base, bytes, hash, {index, at});
    } else {
        std::memcpy(dst, src, bytes);
        copy_versions_overlay_locked(base, bytes, dst);
    }
    out = {mode() == kDevice ? c.dev.buffer : c.host.buffer, at, bytes};
    ++g_stats.bound;
    g_stats.bytes += bytes;
    return true;
}

void cb_ring_gpu_wrote_locked(std::uint64_t va, std::uint64_t bytes) {
    if (!tracked()) return;
    g_writes.wrote(va, bytes, g.flushes);
}

void cb_ring_stage_writes_locked(const gcn::TranslateResult& meta, const std::uint32_t* user, const GxStageRecords* gx,
                                 StoreResolve resolve) {
    if (!tracked() || (meta.store_buffers.empty() && !meta.untraced_stores)) return;
    if (meta.untraced_stores) g_writes.wrote_anywhere(g.flushes);
    for (const gcn::ResourcePath& path : meta.store_buffers) {
        std::uint32_t v[4]{};
        if (!(resolve ? resolve(path, user, gx, 4, v) : resolve_resource(path, user, 4, v))) {
            g_writes.wrote_anywhere(g.flushes);  // a store the host cannot place
            continue;
        }
        const bbg::GuestRange r = bbg::vsharp_store_range(v);
        g_writes.wrote(r.base, r.bytes, g.flushes);
    }
}

bool cb_ring_gpu_write_pending_locked(std::uint64_t va, std::uint64_t bytes) {
    return tracked() && g_writes.pending(va, bytes, g.completed_submits);
}

void cb_ring_slot_retired_locked(int slot) {
    if (!tracked()) return;
    g_ring.retire(slot);
    g_writes.retire(g.completed_submits);
}

}  // namespace gpu
