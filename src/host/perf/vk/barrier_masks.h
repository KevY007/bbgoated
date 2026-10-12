#pragma once
// bbgoated (BBHOST_BBG_PRECISE_BARRIERS=1): stage and access masks for the
// barriers whose consumers the resource tells (report A R3 step 3): a render
// target or snapshot is next drawn into, sampled, read by compute or copied -
// never vertex input, indirect arguments or read by the host - and a region
// copy's image is only sampled. Every other barrier keeps ALL_COMMANDS.
#include "host/perf/bbg_perf.h"

#include <vulkan/vulkan.h>

namespace gpu {
inline bool precise_barriers() {
    static const bool v = bbg::perf_on("PRECISE_BARRIERS");
    return v;
}
// Every shader stage a draw or a dispatch has here: the rect lists' geometry
// shader too (the device is always made with it), and the tessellation
// stages when the device has them (`tessellation`: Gpu::has_tessellation).
inline VkPipelineStageFlags bbg_shader_stages(bool tessellation) {
    return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
           VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
           (tessellation ? VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT : 0);
}
// Every stage a target or snapshot is used in: what may have written or read
// it before a transfer writes it, and what may write it after a transfer or a
// compute pass read it (a copy's source, whose next pass or clear must wait).
inline VkPipelineStageFlags bbg_target_users(bool tessellation) {
    return bbg_shader_stages(tessellation) | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
           VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
}
inline VkPipelineStageFlags bbg_color_consumers(bool tessellation) {
    return bbg_shader_stages(tessellation) | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
}
constexpr VkAccessFlags kBbgColorConsumerAccess = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                                  VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT |
                                                  VK_ACCESS_TRANSFER_WRITE_BIT;
inline VkPipelineStageFlags bbg_depth_consumers(bool tessellation) {
    return bbg_shader_stages(tessellation) | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
           VK_PIPELINE_STAGE_TRANSFER_BIT;
}
constexpr VkAccessFlags kBbgDepthConsumerAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                                  VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
}  // namespace gpu
