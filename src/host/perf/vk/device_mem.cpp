// See device_mem.h.
#include "host/perf/vk/device_mem.h"

#include "host/perf/bbg_perf.h"
#include "host/perf/core/device_mem_core.h"
#include "log.h"

#include <string>
#include <vector>

namespace gpu {

VkBuffer g_bbg_params_twin = VK_NULL_HANDLE;

namespace {
VkDeviceMemory g_twin_memory = VK_NULL_HANDLE;
std::vector<bbg::CopyList<VkBuffer, VkBufferCopy>> g_copies;  // under g.mu
std::vector<void (*)()>& sources() {
    static std::vector<void (*)()> s;
    return s;
}
}  // namespace

int bbg_params_mem() {
    static const int m = [] {
        const std::string v = bbg::perf_value("PARAMS_MEM");
        return v == "rebar" ? kParamsRebar : v == "device" ? kParamsDevice : kParamsUpstream;
    }();
    return m;
}

bool bbg_pt_rebar() {
    static const bool on = bbg::perf_value("PT_MEM") == "rebar";
    return on;
}

std::uint32_t bbg_rebar_type(std::uint32_t type_bits) {
    const std::uint32_t t = find_memory_type(
        type_bits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    static bool said = false;
    if (!said) {
        said = true;
        if (t == UINT32_MAX) {
            host_log("bbg: no device-local host-visible memory type: rebar placements stay where they were");
        } else {
            const std::uint64_t heap = g.mem_props.memoryHeaps[g.mem_props.memoryTypes[t].heapIndex].size;
            host_log("bbg: device-local host-visible memory: type %u, heap %llu MiB (%s)", t, static_cast<unsigned long long>(heap >> 20),
                     bbg::resizable_bar_heap(heap) ? "resizable BAR" : "the 256 MiB window, no resizable BAR");
        }
    }
    return t;
}

bool bbg_params_twin_create_locked(VkDeviceSize bytes) {
    VkBufferCreateInfo bci{};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = bytes;
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkBuffer b = VK_NULL_HANDLE;
    if (vkCreateBuffer(g.device, &bci, nullptr, &b) != VK_SUCCESS) return false;
    VkMemoryRequirements req{};
    vkGetBufferMemoryRequirements(g.device, b, &req);
    VkMemoryAllocateInfo mai{};
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize = req.size;
    mai.memoryTypeIndex = find_memory_type(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (mai.memoryTypeIndex == UINT32_MAX || vkAllocateMemory(g.device, &mai, nullptr, &g_twin_memory) != VK_SUCCESS) {
        vkDestroyBuffer(g.device, b, nullptr);
        host_log("bbg: the params ring's device-local twin could not be made; the ring is read where it is");
        return false;
    }
    vkBindBufferMemory(g.device, b, g_twin_memory, 0);
    g_bbg_params_twin = b;
    host_log("bbg: params ring twin in device-local memory (%llu MiB), filled before each submission (BBHOST_BBG_PARAMS_MEM=device)",
             static_cast<unsigned long long>(bytes >> 20));
    return true;
}

void bbg_presubmit_add_locked(VkBuffer src, VkBuffer dst, VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize bytes) {
    bbg::copies_add(g_copies, src, dst, src_offset, dst_offset, bytes);
}

bool bbg_presubmit_register(void (*fn)()) {
    sources().push_back(fn);
    return true;
}

void bbg_presubmit_locked() {
    if (g_bbg_params_twin && g.queued) {
        const bbg::Span s = bbg::params_span(static_cast<std::uint32_t>(g.slot), kMaxQueued * kStageSlots, g.queued, kParamsStride);
        bbg_presubmit_add_locked(g.ubo, g_bbg_params_twin, s.offset, s.offset, s.bytes);
    }
    for (void (*fn)() : sources()) fn();
    if (g_copies.empty()) return;
    const VkCommandBuffer cmd = shadow_upload_cmd_for_bbg_locked();
    if (!cmd) {
        static int logs = 0;
        if (logs++ < 4) host_log("bbg: no upload command buffer for this submission: its constant and params copies are lost");
        g_copies.clear();
        return;
    }
    for (const auto& c : g_copies) vkCmdCopyBuffer(cmd, c.src, c.dst, static_cast<std::uint32_t>(c.regions.size()), c.regions.data());
    shadow_uploads_feed_shaders_locked();
    g_copies.clear();
}

}  // namespace gpu
