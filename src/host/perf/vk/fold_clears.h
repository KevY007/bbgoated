#pragma once
// bbgoated (BBHOST_BBG_FOLD_CLEARS=depth|all): a clear of a whole single-layer
// target is held and done by the target's next pass (VK_ATTACHMENT_LOAD_OP_CLEAR);
// anything else recorded first records it as before (rec(), g_cmd(), a pass
// that ends, a sampled view, a region copy), and a target destroyed meanwhile
// takes its clear with it. Under g.mu.
#include "host/gpu_internal.h"

#include <cstdint>

namespace gpu {
int fold_clears_mode();  // 0 off, 1 depth (clears at a draw), 2 all (fills too)
// A held clear carries what recording it needs rather than its RtImage: the
// target's entry can move between the maps (a snapshot promoted) or leave them
// before its image goes (a target grown or carried into a new format copies
// from the old image first), and an image does not change under its handle.
struct FoldClear {
    VkImage image = VK_NULL_HANDLE;
    std::uint64_t base = 0;  // the target's, for the clears' own log lines
    VkFormat format = VK_FORMAT_UNDEFINED;
    bool depth_target = false;
    bool at_draw = false;  // clear_depth at a draw; else clear_image_locked's
    VkImageAspectFlags aspects = 0;
    VkClearValue value{};  // the load op's, the same bits the clear command took
    float rgba[4] = {};    // clear_image_locked's arguments
    std::uint32_t first = 0, count = 1;
    bool depth = false, stencil = false;  // clear_depth's arguments
    float dval = 0.0f;
    std::uint32_t sval = 0;
};
extern bool g_fold_pending;  // something is held: the hooks' only test
// render.cpp: a held clear recorded as it would have been, with its own pass
// end and barrier (clear_image_record_locked or clear_depth).
void fold_record_as_before(const FoldClear& c);
void fold_hold_locked(const FoldClear& c);
void fold_record_locked(const FoldClear& c);  // a taken clear recorded as before (and counted for the report)
bool fold_take_locked(VkImage image, FoldClear& out);
void fold_record_all_locked();
void fold_record_image_locked(VkImage image);
// A draw's image binding (its T# or V# words): a held clear of the target it
// names lands first. The view memos hand out a target's view without
// render_target_view, so the binding is matched by the address alias_rt reads.
void fold_record_sampled_locked(const std::uint32_t* w);
void fold_forget_image_locked(VkImage image);
void fold_note_done_locked(bool depth);
}  // namespace gpu
