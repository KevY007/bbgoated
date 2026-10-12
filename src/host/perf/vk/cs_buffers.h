#pragma once
// bbgoated (BBHOST_BBG_CS_CB=bound): compute shaders read their constant
// buffers through storage buffers bound over guest memory
// (TranslateOptions::cb_ssbo, the page-table walk kept as the fallback), as
// graphics stages do - instead of every load walking the page table into host
// memory (report A R4). The bindings are a descriptor set of their own, set 1
// (host/perf/core/cs_cb_core.h); set 0 is the shared one, bound as before.
#include "host/gpu_internal.h"

namespace gpu {
bool bbg_cs_cb();  // the switch, and its layouts made (any thread)
// compute_options: under the switch, constant buffers through storage buffers in set 1 (any thread).
void bbg_cs_cb_options(gcn::TranslateOptions& o);
// Set 1's layout, storage buffers 112-127, and the layout every compute
// pipeline is made with under the switch: the shared set, then set 1.
VkDescriptorSetLayout bbg_cs_cb_set_layout();
VkPipelineLayout bbg_cs_pipe_layout();
// A dispatch's constant buffers resolved as a draw's (render.cpp's
// resolve_stage_buffers), into `params` (cb_valid, cb_bias_dw, ...) and one
// info per binding. Under g.mu.
void resolve_dispatch_buffers_locked(const gcn::TranslateResult& meta, const std::uint32_t* user, gcn::StageParams& params,
                                     std::vector<VkDescriptorBufferInfo>& infos);
// host_gpu_dispatch, beside set 0's writes: set 1 for a program with constant
// buffers, its writes (from `infos`, which must outlive them) appended to
// `writes`. `set` stays null when the dispatch binds none. False - said and
// counted as a failure - when no set could be had. Under g.mu.
bool bbg_cs_cb_set_locked(const gcn::TranslateResult& meta, const gcn::StageParams& params, const std::vector<VkDescriptorBufferInfo>& infos,
                          std::vector<VkWriteDescriptorSet>& writes, VkDescriptorSet& set);
// After set 0's bind: set 1, with the compute layout (compatible with the
// shared one for set 0, which stays bound).
void bbg_cs_cb_bind_locked(VkDescriptorSet set);
}  // namespace gpu
