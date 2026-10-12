// See cs_buffers.h.
#include "host/perf/vk/cs_buffers.h"

#include "host/perf/bbg_perf.h"
#include "host/perf/core/cs_cb_core.h"
#include "log.h"

#include <mutex>
#include <vector>

namespace gpu {
namespace {
std::once_flag g_once;
VkDescriptorSetLayout g_set = VK_NULL_HANDLE;
VkPipelineLayout g_layout = VK_NULL_HANDLE;
std::uint64_t g_dispatches = 0, g_bound = 0, g_walked = 0, g_next = 0, g_from = 0;  // under g.mu

void make() {
    std::vector<VkDescriptorSetLayoutBinding> binds;
    for (std::uint32_t b : bbg::cs_cb_set_bindings()) binds.push_back({b, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr});
    VkDescriptorSetLayoutCreateInfo lci{};
    lci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    lci.bindingCount = static_cast<std::uint32_t>(binds.size());
    lci.pBindings = binds.data();
    if (vkCreateDescriptorSetLayout(g.device, &lci, nullptr, &g_set) != VK_SUCCESS) g_set = VK_NULL_HANDLE;
    // Set 0 is the shared layout itself and, as in g.pipe_layout, there are no
    // push constants: the two are compatible for set 0, so a pipeline made with
    // this one runs with set 0 bound through either.
    const VkDescriptorSetLayout sets[2] = {g.set_layout, g_set};
    VkPipelineLayoutCreateInfo plci{};
    plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plci.setLayoutCount = 2;
    plci.pSetLayouts = sets;
    if (!g_set || vkCreatePipelineLayout(g.device, &plci, nullptr, &g_layout) != VK_SUCCESS) g_layout = VK_NULL_HANDLE;
    host_log("bbg: compute constant buffers %s (BBHOST_BBG_CS_CB=bound)",
             g_layout ? "bound as storage buffers in set 1, the page-table walk as the fallback" : "left to the page table: the layout failed");
}

void report() {
    if (!bbg::perf_report_due(g_next)) return;
    const std::uint64_t flip = hle_video_flip_count();
    const double f = static_cast<double>(flip > g_from ? flip - g_from : 1);
    host_log("bbg cs cb: %.1f dispatches/flip with constant buffers; %.1f bound, %.1f left to the page table", g_dispatches / f, g_bound / f,
             g_walked / f);
    g_dispatches = g_bound = g_walked = 0;
    g_from = flip;
}
}  // namespace

VkDescriptorSetLayout bbg_cs_cb_set_layout() {
    std::call_once(g_once, make);
    return g_set;
}

VkPipelineLayout bbg_cs_pipe_layout() {
    std::call_once(g_once, make);
    return g_layout;
}

bool bbg_cs_cb() {
    static const bool on = bbg::perf_value("CS_CB") == "bound";
    return on && bbg_cs_pipe_layout() != VK_NULL_HANDLE;
}

void bbg_cs_cb_options(gcn::TranslateOptions& o) {
    if (bbg_cs_cb()) bbg::cs_cb_options(o);
}

bool bbg_cs_cb_set_locked(const gcn::TranslateResult& meta, const gcn::StageParams& params, const std::vector<VkDescriptorBufferInfo>& infos,
                          std::vector<VkWriteDescriptorSet>& writes, VkDescriptorSet& set) {
    set = VK_NULL_HANDLE;
    if (!bbg::cs_cb_binds(bbg_cs_cb(), meta.buffers.size())) return true;
    report();
    set = alloc_set_locked(bbg_cs_cb_set_layout());
    if (!set) {
        host_log("gpu: descriptor set allocation failed");
        g.failures.fetch_add(1);
        return false;
    }
    for (const bbg::CbWrite& c : bbg::cs_cb_writes(meta.buffers, infos.size())) {
        VkWriteDescriptorSet w{};
        w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        w.dstSet = set;
        w.dstBinding = c.binding;
        w.descriptorCount = 1;
        w.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        w.pBufferInfo = &infos[c.info];
        writes.push_back(w);
        ++((params.cb_valid >> c.info) & 1 ? g_bound : g_walked);
    }
    ++g_dispatches;
    return true;
}

void bbg_cs_cb_bind_locked(VkDescriptorSet set) {
    rec().bind_sets(VK_PIPELINE_BIND_POINT_COMPUTE, bbg_cs_pipe_layout(), bbg::kCsCbSet, 1, &set);
}

}  // namespace gpu
