// host/gpu_internal.h: the draw-side state and binding suppression from
// upstream PR #100 (draw_library_changes, DrawBindingState) that bbgoated's
// BBHOST_BBG_DYN_FIELDS and BBHOST_BBG_BIND_DEDUP use (bbgoated).
#if __has_include(<vulkan/vulkan.h>)
#include "host/gpu_internal.h"

#include <cstdio>
#include <cstring>

namespace {
using gpu::DrawBindingState;
using gpu::DrawLibraryState;

DrawLibraryState base_state() {
    DrawLibraryState s;
    std::memset(&s, 0, sizeof(s));  // as draw_impl builds it
    s.cull = VK_CULL_MODE_BACK_BIT;
    s.front = VK_FRONT_FACE_CLOCKWISE;
    s.depth_test = VK_TRUE;
    s.depth_write = VK_TRUE;
    s.depth_compare = VK_COMPARE_OP_LESS_OR_EQUAL;
    s.front_ops = {VK_STENCIL_OP_KEEP, VK_STENCIL_OP_REPLACE, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0xff, 0xff, 1};
    s.back_ops = s.front_ops;
    return s;
}
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
    const DrawLibraryState a = base_state();
    check(draw_library_changes(a, a) == 0, "fields: the same state changes nothing");
    check(kLibraryAll == 0x7ff, "fields: eleven, all of them in kLibraryAll");
    struct Case {
        const char* what;
        void (*change)(DrawLibraryState&);
        std::uint32_t field;
    };
    const Case cases[] = {
        {"fields: cull", [](DrawLibraryState& s) { s.cull = VK_CULL_MODE_NONE; }, kLibraryCull},
        {"fields: front face", [](DrawLibraryState& s) { s.front = VK_FRONT_FACE_COUNTER_CLOCKWISE; }, kLibraryFront},
        {"fields: depth test", [](DrawLibraryState& s) { s.depth_test = VK_FALSE; }, kLibraryDepthTest},
        {"fields: depth write", [](DrawLibraryState& s) { s.depth_write = VK_FALSE; }, kLibraryDepthWrite},
        {"fields: depth compare", [](DrawLibraryState& s) { s.depth_compare = VK_COMPARE_OP_GREATER; }, kLibraryDepthCompare},
        {"fields: depth bounds test", [](DrawLibraryState& s) { s.bounds_test = VK_TRUE; }, kLibraryBoundsTest},
        {"fields: stencil test", [](DrawLibraryState& s) { s.stencil_test = VK_TRUE; }, kLibraryStencilTest},
        {"fields: front stencil ops", [](DrawLibraryState& s) { s.front_ops.passOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP; }, kLibraryFrontOps},
        {"fields: back stencil compare", [](DrawLibraryState& s) { s.back_ops.compareOp = VK_COMPARE_OP_EQUAL; }, kLibraryBackOps},
        {"fields: depth bias constant", [](DrawLibraryState& s) { s.bias_constant = 2.0f; }, kLibraryBias},
        {"fields: depth bias slope", [](DrawLibraryState& s) { s.bias_slope = 1.5f; }, kLibraryBias},
        {"fields: depth bias clamp, -0 is not 0 (the bits are set)", [](DrawLibraryState& s) { s.bias_clamp = -0.0f; }, kLibraryBias},
        {"fields: depth clamp", [](DrawLibraryState& s) { s.depth_clamp = VK_TRUE; }, kLibraryDepthClamp},
    };
    for (const Case& c : cases) {
        DrawLibraryState b = a;
        c.change(b);
        check(draw_library_changes(a, b) == c.field, c.what);
    }
    DrawLibraryState masks = a;
    masks.front_ops.compareMask = 0x0f;
    masks.back_ops.writeMask = 0;
    masks.front_ops.reference = 7;
    check(draw_library_changes(a, masks) == 0, "fields: stencil masks and reference are record_stencil_words', not library state");
    DrawLibraryState two = a;
    two.cull = VK_CULL_MODE_NONE;
    two.depth_clamp = VK_TRUE;
    check(draw_library_changes(a, two) == (kLibraryCull | kLibraryDepthClamp), "fields: two changes, two bits");

    // Bindings within one layout: identical pushes, vertex buffers and the
    // bindless set are left out; sets 0 and 1 are never tracked here.
    DrawBindingState st;
    const VkPipelineLayout l1 = reinterpret_cast<VkPipelineLayout>(0x1000), l2 = reinterpret_cast<VkPipelineLayout>(0x2000);
    const VkDescriptorSet global = reinterpret_cast<VkDescriptorSet>(0x3000);
    const VkBuffer b1 = reinterpret_cast<VkBuffer>(0x4000), b2 = reinterpret_cast<VkBuffer>(0x5000);
    st.use_layout(l1);
    check(st.bind_global(global) && !st.bind_global(global), "bindless: bound once per layout");
    const std::uint32_t bindings[2] = {4, 5};
    VkDescriptorBufferInfo infos[2] = {{b1, 0, 256}, {b2, 512, 64}};
    check(st.push(bindings, infos, 2), "push: the first is made");
    check(!st.push(bindings, infos, 2), "push: the same ranges again are left out");
    infos[1].offset = 576;
    check(st.push(bindings, infos, 2), "push: another offset is pushed");
    infos[1].range = 128;
    check(st.push(bindings, infos, 2), "push: another range is pushed");
    check(st.push(bindings, infos, 1) && st.push(bindings, infos, 2), "push: fewer bindings, then more, are pushed");
    const std::uint32_t other[2] = {4, 6};
    check(st.push(other, infos, 2), "push: another binding is pushed");
    check(st.push(other, infos, 2 * gcn::kMaxBuffers + 1) && st.push(other, infos, 2), "push: too many to track is pushed and forgotten");
    const VkBuffer vb[3] = {b1, b2, b1};
    VkDeviceSize vo[3] = {0, 64, 128};
    check(st.vertex(vb, vo, 3) && !st.vertex(vb, vo, 3), "vertex: the same buffers again are left out");
    vo[2] = 192;
    check(st.vertex(vb, vo, 3), "vertex: another offset is bound");
    check(st.vertex(vb, vo, 2), "vertex: fewer buffers are bound");
    check(st.vertex(vb, vo, 17) && st.vertex(vb, vo, 2), "vertex: more than 16 is bound and forgotten");
    // Another layout: pushes and the bindless set again, vertex buffers kept.
    st.use_layout(l2);
    check(st.layout == l2, "layout: the new one is tracked");
    check(st.bind_global(global), "layout: the bindless set is bound again");
    check(st.push(other, infos, 2), "layout: the push is made again");
    check(!st.vertex(vb, vo, 2), "layout: vertex buffers do not depend on it");
    st.use_layout(l2);
    check(!st.push(other, infos, 2) && !st.bind_global(global), "layout: the same one again forgets nothing");
    if (!fail) std::printf("bbg_draw_dedup_test: ok\n");
    return fail ? 1 : 0;
}
#else
#include <cstdio>
int main() {
    std::printf("bbg_draw_dedup_test: no Vulkan headers, nothing to test\n");
    return 0;
}
#endif
