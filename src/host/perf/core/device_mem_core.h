#pragma once
// bbgoated (BBHOST_BBG_PARAMS_MEM, BBHOST_BBG_PT_MEM), the part without Vulkan:
// the copies a submission's upload buffer runs before the submission's work,
// the params ring's span a submission wrote, and which heap is resizable BAR.
// host/perf/vk/device_mem.cpp records the copies.
#include <cstdint>
#include <vector>

namespace bbg {

// The copies from one buffer into another, as vkCmdCopyBuffer takes them
// (Region: VkBufferCopy, or anything with its three fields).
template <class Buffer, class Region>
struct CopyList {
    Buffer src, dst;
    std::vector<Region> regions;
};

// A copy into the list of its source and destination - one list per pair, in
// the order first seen - merged into the list's last region when it continues
// that region in both buffers.
template <class Buffer, class Region>
inline void copies_add(std::vector<CopyList<Buffer, Region>>& lists, Buffer src, Buffer dst, std::uint64_t src_offset, std::uint64_t dst_offset,
                       std::uint64_t bytes) {
    if (!bytes) return;
    for (CopyList<Buffer, Region>& c : lists) {
        if (c.src != src || c.dst != dst) continue;
        Region& last = c.regions.back();
        if (last.srcOffset + last.size == src_offset && last.dstOffset + last.size == dst_offset) {
            last.size += bytes;
        } else {
            c.regions.push_back({src_offset, dst_offset, bytes});
        }
        return;
    }
    lists.push_back({src, dst, {{src_offset, dst_offset, bytes}}});
}

struct Span {
    std::uint64_t offset, bytes;
};
// The params blocks a submission wrote: slot `slot`'s region of the ring
// (`per_slot` blocks of `stride` bytes each), its first `queued` blocks.
inline Span params_span(std::uint32_t slot, std::uint32_t per_slot, std::uint32_t queued, std::uint32_t stride) {
    return {static_cast<std::uint64_t>(slot) * per_slot * stride, static_cast<std::uint64_t>(queued) * stride};
}

// A device-local host-visible heap bigger than the 256 MiB window the CPU
// sees without resizable BAR (an integrated GPU's is its whole memory).
inline bool resizable_bar_heap(std::uint64_t heap_bytes) { return heap_bytes > (512ull << 20); }

}  // namespace bbg
