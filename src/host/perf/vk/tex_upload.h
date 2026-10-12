#pragma once
// bbgoated (BBHOST_BBG_TEX_UPLOAD=stream|early): where a texture upload's
// commands go (host/perf/core/tex_upload_core.h). Upstream records them in
// place (g_cmd(), the recorder drained first) after ending the open pass.
// stream: recorder ops at the same place in the stream, no drain. early: the
// submission's upload buffer, which runs before its work - no pass ends - for
// a new image's first upload that no pending GPU write feeds. Under g.mu.
#include "host/gpu_internal.h"

namespace gpu {
// An upload's commands: in place as upstream records them (always inlined,
// so g_cmd() still sees upload_surface as its caller), recorder ops (stream),
// or the upload buffer (early).
struct UploadCmds {
    enum Mode { kInPlace, kStream, kEarly } mode = kInPlace;
    VkCommandBuffer early_cmd = VK_NULL_HANDLE;
    bool early() const { return mode == kEarly; }
    __attribute__((always_inline)) void barrier(VkPipelineStageFlags src, VkPipelineStageFlags dst, const VkMemoryBarrier* mb,
                                                const VkImageMemoryBarrier* ib) {
        const std::uint32_t nm = mb ? 1u : 0u, ni = ib ? 1u : 0u;
        if (mode == kStream) return rec().pipeline_barrier(src, dst, 0, nm, mb, 0, nullptr, ni, ib);
        vkCmdPipelineBarrier(mode == kEarly ? early_cmd : g_cmd(), src, dst, 0, nm, mb, 0, nullptr, ni, ib);
    }
    __attribute__((always_inline)) void copy_buffer(VkBuffer src, VkBuffer dst, const VkBufferCopy& region) {
        if (mode == kStream) return rec().copy_buffer(src, dst, 1, &region);
        vkCmdCopyBuffer(mode == kEarly ? early_cmd : g_cmd(), src, dst, 1, &region);
    }
    __attribute__((always_inline)) void fill_buffer(VkBuffer dst, VkDeviceSize offset, VkDeviceSize size, std::uint32_t data) {
        if (mode == kStream) return rec().fill_buffer(dst, offset, size, data);
        vkCmdFillBuffer(mode == kEarly ? early_cmd : g_cmd(), dst, offset, size, data);
    }
    __attribute__((always_inline)) void copy_buffer_to_image(VkBuffer src, VkImage image, VkImageLayout layout, std::uint32_t n,
                                                             const VkBufferImageCopy* regions) {
        if (mode == kStream) return rec().copy_buffer_to_image(src, image, layout, n, regions);
        vkCmdCopyBufferToImage(mode == kEarly ? early_cmd : g_cmd(), src, image, layout, n, regions);
    }
    __attribute__((always_inline)) void untile(const UntileGpuPass& pass) {
        if (mode == kInPlace) return untile_gpu_record_locked(pass);
        untile_gpu_record_to_locked(mode == kEarly ? early_cmd : VK_NULL_HANDLE, pass);
    }
};
// The upload of guest bytes [base, base + bytes). `first`: the surface's first
// upload, its image new. `keep`: in place under early regardless (a
// GPU-written surface, the BBHOST_GPU_UNTILE=2 compare).
UploadCmds upload_cmds_for_locked(std::uint64_t base, std::uint64_t bytes, bool first, bool keep);
}  // namespace gpu
