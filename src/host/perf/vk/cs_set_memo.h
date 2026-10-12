#pragma once
// bbgoated (BBHOST_BBG_CS_SET_MEMO=1): a dispatch whose final set-0 writes
// match a set this slot already made binds that set instead of allocating and
// writing another - upstream PR #100's immutable compute descriptor reuse, by
// Raikzer (host/immutable_descriptor_memo.h). Only with upstream's set cache:
// without it every dispatch's params descriptor names a slice of its own.
// Under g.mu.
#include "host/gpu_internal.h"

namespace gpu {
bool cs_set_memo_on();
// host_gpu_dispatch, once set 0's writes are built against no set: the slot's
// set that holds exactly them - and `writes` is emptied, nothing to write - or
// a new set, every write pointed at it and remembered. False - said and
// counted as a failure - when no set could be had.
bool cs_set_memo_take_locked(std::vector<VkWriteDescriptorSet>& writes, VkDescriptorSet& set);
// retire_slot_locked, after the slot's pool reset: its sets are gone.
void cs_set_memo_pool_reset_locked(int slot);
// defer_destroy_view and defer_destroy_private_view: no set holding the view is bound again.
void cs_set_memo_forget_view_locked(VkImageView view);
}  // namespace gpu
