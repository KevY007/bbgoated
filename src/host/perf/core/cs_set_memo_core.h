#pragma once
// bbgoated (BBHOST_BBG_CS_SET_MEMO=1, host/perf/vk/cs_set_memo.cpp), the part
// without Vulkan: the memo key of a dispatch set's final writes - upstream PR
// #100's immutable compute descriptor reuse, by Raikzer, whose memo
// (host/immutable_descriptor_memo.h) is taken as it is.
#include "host/immutable_descriptor_memo.h"

#include <cstddef>
#include <cstdint>

namespace bbg {

// The key of the writes `w[0..n)` into a set of `layout`: one binding per
// descriptor, in write order, its buffer or its sampler, view and layout -
// everything but the destination set. `Write` is VkWriteDescriptorSet (or
// anything with its fields).
template <class Write>
gpu::ImmutableDescriptorKey descriptor_key_of(const Write* w, std::size_t n, std::uint64_t layout) {
    gpu::ImmutableDescriptorKey key;
    key.layout = layout;
    for (std::size_t k = 0; k < n; ++k) {
        for (std::uint32_t i = 0; i < w[k].descriptorCount; ++i) {
            gpu::ImmutableDescriptorBinding b;
            b.binding = w[k].dstBinding;
            b.array_element = w[k].dstArrayElement + i;
            b.type = static_cast<std::uint32_t>(w[k].descriptorType);
            if (w[k].pBufferInfo) {
                b.buffer = gpu::descriptor_handle(w[k].pBufferInfo[i].buffer);
                b.offset = w[k].pBufferInfo[i].offset;
                b.range = w[k].pBufferInfo[i].range;
            }
            if (w[k].pImageInfo) {
                b.sampler = gpu::descriptor_handle(w[k].pImageInfo[i].sampler);
                b.view = gpu::descriptor_handle(w[k].pImageInfo[i].imageView);
                b.image_layout = static_cast<std::uint32_t>(w[k].pImageInfo[i].imageLayout);
            }
            key.bindings.push_back(b);
        }
    }
    return key;
}

}  // namespace bbg
