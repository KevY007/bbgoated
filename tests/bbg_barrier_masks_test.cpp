// host/perf/vk/barrier_masks.h: the stage and access masks of the barriers
// BBHOST_BBG_PRECISE_BARRIERS narrows - every stage a render target or
// snapshot is used in, nothing else, and access masks the stages support
// (bbgoated).
#if __has_include(<vulkan/vulkan.h>)
#include "host/perf/vk/barrier_masks.h"

#include <cstdio>

namespace {
// The pipeline stages each access type is supported in (the Vulkan spec's
// table of supported access types), for the ones these masks may hold.
VkPipelineStageFlags supported(VkAccessFlags bit) {
    const VkPipelineStageFlags shaders = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
                                         VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
                                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    switch (bit) {
        case VK_ACCESS_SHADER_READ_BIT: case VK_ACCESS_SHADER_WRITE_BIT: case VK_ACCESS_UNIFORM_READ_BIT: return shaders;
        case VK_ACCESS_COLOR_ATTACHMENT_READ_BIT: case VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT: return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT: case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:
            return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        case VK_ACCESS_TRANSFER_READ_BIT: case VK_ACCESS_TRANSFER_WRITE_BIT: return VK_PIPELINE_STAGE_TRANSFER_BIT;
        default: return 0;  // anything else (host, indirect, vertex input, the generic MEMORY_* bits) is not expected here
    }
}
bool access_fits(VkAccessFlags access, VkPipelineStageFlags stages) {
    for (VkAccessFlags bit = 1; bit; bit <<= 1) {
        if ((access & bit) && !(supported(bit) & stages)) return false;
    }
    return true;
}
bool has(VkFlags mask, VkFlags bits) { return (mask & bits) == bits; }
}  // namespace

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    using namespace gpu;
    const VkPipelineStageFlags never = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT | VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT |
                                       VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_HOST_BIT |
                                       VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT | VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT |
                                       VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    const VkPipelineStageFlags tess = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    for (bool t : {false, true}) {
        const VkPipelineStageFlags shaders = bbg_shader_stages(t), users = bbg_target_users(t);
        const VkPipelineStageFlags color = bbg_color_consumers(t), depth = bbg_depth_consumers(t);
        check(has(shaders, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                               VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT),
              "shaders: vertex, geometry (rect lists), fragment and compute");
        check(t ? has(shaders, tess) : !(shaders & tess), "shaders: tessellation stages only with the feature");
        check(has(users, shaders | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                             VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT),
              "users: drawn into, depth-tested, sampled or stored by any shader, copied or cleared");
        check(has(color, shaders | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT),
              "colour consumers: drawn into, any shader, copied");
        check(has(depth, shaders | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
                             VK_PIPELINE_STAGE_TRANSFER_BIT),
              "depth consumers: both fragment tests, any shader, copied");
        check(!(color & (VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT)) &&
                  !(depth & VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT),
              "consumers: a colour image is no depth attachment and the other way round");
        check((color & ~users) == 0 && (depth & ~users) == 0, "consumers: within the users");
        check(!((shaders | users | color | depth) & never), "never vertex input, indirect arguments, the host or all commands");
        check(access_fits(kBbgColorConsumerAccess, color), "colour access: each bit in a stage of its mask");
        check(access_fits(kBbgDepthConsumerAccess, depth), "depth access: each bit in a stage of its mask");
        check(access_fits(kBbgColorConsumerAccess, users) && access_fits(kBbgDepthConsumerAccess, users),
              "after a read of a target the users wait: the written image's access still fits");
        check(access_fits(VK_ACCESS_SHADER_READ_BIT, shaders), "a region copy's image: sampled by the shaders");
    }
    check(has(kBbgColorConsumerAccess, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT |
                                           VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT),
          "colour access: attachment, sampled and stored, copied both ways");
    check(has(kBbgDepthConsumerAccess, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                           VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT),
          "depth access: attachment, sampled, copied both ways");
    const VkAccessFlags generic = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT |
                                  VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    check(!(kBbgColorConsumerAccess & generic) && !(kBbgDepthConsumerAccess & generic), "access: no generic, host, indirect or vertex bits");
    if (!fail) std::printf("bbg_barrier_masks_test: ok\n");
    return fail ? 1 : 0;
}
#else
#include <cstdio>
int main() {
    std::printf("bbg_barrier_masks_test: no Vulkan headers, nothing to test\n");
    return 0;
}
#endif
