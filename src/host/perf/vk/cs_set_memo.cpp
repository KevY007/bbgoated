// See cs_set_memo.h.
#include "host/perf/vk/cs_set_memo.h"

#include "host/immutable_descriptor_memo.h"
#include "host/perf/bbg_perf.h"
#include "host/perf/core/cs_set_memo_core.h"
#include "log.h"

namespace gpu {
namespace {
ImmutableDescriptorMemo<VkDescriptorSet> g_sets[kSlots];
std::uint64_t g_taken = 0, g_reused = 0, g_next = 0, g_from = 0;

void report() {
    if (!bbg::perf_report_due(g_next)) return;
    const std::uint64_t flip = hle_video_flip_count();
    const double f = static_cast<double>(flip > g_from ? flip - g_from : 1);
    host_log("bbg cs set memo: %.1f dispatch sets/flip, %.1f of them reused", g_taken / f, g_reused / f);
    g_taken = g_reused = 0;
    g_from = flip;
}
}  // namespace

bool cs_set_memo_on() {
    static const bool on = bbg::perf_on("CS_SET_MEMO") && set_cache_on();
    return on;
}

bool cs_set_memo_take_locked(std::vector<VkWriteDescriptorSet>& writes, VkDescriptorSet& set) {
    report();
    ImmutableDescriptorMemo<VkDescriptorSet>& memo = g_sets[g.slot];
    memo.observe_epoch(view_epoch());
    const ImmutableDescriptorKey key = bbg::descriptor_key_of(writes.data(), writes.size(), descriptor_handle(g.set_layout));
    ++g_taken;
    if ((set = memo.find(key))) {
        writes.clear();  // already in it: the set is never written again
        ++g_reused;
        return true;
    }
    set = alloc_set_locked();
    if (!set) {
        host_log("gpu: descriptor set allocation failed");
        g.failures.fetch_add(1);
        return false;
    }
    for (VkWriteDescriptorSet& w : writes) w.dstSet = set;
    memo.remember(key, set);
    return true;
}

void cs_set_memo_pool_reset_locked(int slot) {
    if (cs_set_memo_on()) g_sets[slot].clear();
}

void cs_set_memo_forget_view_locked(VkImageView view) {
    if (!cs_set_memo_on() || !view) return;
    for (ImmutableDescriptorMemo<VkDescriptorSet>& memo : g_sets) memo.forget_view(descriptor_handle(view));
}

}  // namespace gpu
