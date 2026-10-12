#pragma once
// bbgoated (BBHOST_BBG_CS_CB=bound, host/perf/vk/cs_buffers.cpp), the part
// without Vulkan: a dispatch's constant buffers bound as storage buffers, as a
// draw's are, in a descriptor set of their own. Set 0 stays the shared
// layout's, so every compute pipeline - the tessellation LS pass's too, which
// binds set 0 alone - is made with one layout whose set 0 is the shared one.
#include "gcn/translate.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace bbg {

constexpr std::uint32_t kCsCbSet = 1;

// A dispatch's translation: constant buffers through storage buffers in set 1
// from binding 112, and a V# that does not bind walks the page table as before.
inline void cs_cb_options(gcn::TranslateOptions& o) {
    o.cb_ssbo = true;
    o.cb_no_fallback = false;
    o.cb_descriptor_set = static_cast<int>(kCsCbSet);
    o.cb_binding_base = gcn::kBindingStorageBuffer0;
}

// Set 1's bindings: a storage buffer for each binding a translation can make.
inline std::vector<std::uint32_t> cs_cb_set_bindings() {
    std::vector<std::uint32_t> out;
    for (std::uint32_t k = 0; k < gcn::kMaxBuffers; ++k) out.push_back(gcn::kBindingStorageBuffer0 + k);
    return out;
}

// Set 1's writes: each binding the translation made, with the info
// resolve_stage_buffers gave it (one per binding, in order).
struct CbWrite {
    std::uint32_t binding;
    std::size_t info;
};
inline std::vector<CbWrite> cs_cb_writes(const std::vector<gcn::BufferBinding>& buffers, std::size_t infos) {
    std::vector<CbWrite> out;
    for (std::size_t i = 0; i < buffers.size() && i < infos; ++i) out.push_back({buffers[i].binding, i});
    return out;
}

// Whether a dispatch binds set 1: under the switch, for a program with constant buffers.
inline bool cs_cb_binds(bool on, std::size_t buffers) { return on && buffers != 0; }

}  // namespace bbg
