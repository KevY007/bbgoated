// host/perf/core/cs_set_memo_core.h and upstream PR #100's
// host/immutable_descriptor_memo.h: a dispatch's descriptor set reused by its
// final writes within a slot (bbgoated, BBHOST_BBG_CS_SET_MEMO).
#include "host/immutable_descriptor_memo.h"
#include "host/perf/core/cs_set_memo_core.h"

#include <cstdint>
#include <cstdio>
#include <vector>

namespace {
// The fields of VkWriteDescriptorSet and its infos the key reads.
struct BufferInfo {
    std::uint64_t buffer, offset, range;
};
struct ImageInfo {
    std::uint64_t sampler, imageView;
    std::uint32_t imageLayout;
};
struct Write {
    std::uint64_t dstSet;
    std::uint32_t dstBinding, dstArrayElement, descriptorCount, descriptorType;
    const ImageInfo* pImageInfo;
    const BufferInfo* pBufferInfo;
};
}  // namespace

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    const BufferInfo params{0x1000, 0, 512};
    const ImageInfo images[2] = {{0, 0x2000, 1}, {0, 0x2100, 1}};
    const ImageInfo sampler{0x3000, 0, 0};
    std::vector<Write> w = {{0, 0, 0, 1, 8, nullptr, &params}, {0, 16, 0, 2, 2, images, nullptr}, {0, 48, 0, 1, 0, &sampler, nullptr}};
    const gpu::ImmutableDescriptorKey key = bbg::descriptor_key_of(w.data(), w.size(), 77);
    check(key.layout == 77, "key: the set layout");
    check(key.bindings.size() == 4, "key: one binding a descriptor, two for a two-element write");
    check(key.bindings.size() == 4 && key.bindings[0].binding == 0 && key.bindings[0].type == 8 && key.bindings[0].buffer == 0x1000 &&
              key.bindings[0].range == 512,
          "key: the params buffer, its offset and range");
    check(key.bindings.size() == 4 && key.bindings[1].binding == 16 && key.bindings[1].array_element == 0 && key.bindings[1].view == 0x2000 &&
              key.bindings[2].binding == 16 && key.bindings[2].array_element == 1 && key.bindings[2].view == 0x2100 &&
              key.bindings[2].image_layout == 1,
          "key: an array write's elements in order, each with its view and layout");
    check(key.bindings.size() == 4 && key.bindings[3].binding == 48 && key.bindings[3].sampler == 0x3000, "key: the sampler");
    w[0].dstSet = 0x9999;
    check(bbg::descriptor_key_of(w.data(), w.size(), 77) == key, "key: the destination set is not part of it");
    w[1].dstArrayElement = 1;
    check(!(bbg::descriptor_key_of(w.data(), w.size(), 77) == key), "key: another array element is another key");
    w[1].dstArrayElement = 0;
    const ImageInfo moved[2] = {{0, 0x2000, 1}, {0, 0x2200, 1}};
    w[1].pImageInfo = moved;
    check(!(bbg::descriptor_key_of(w.data(), w.size(), 77) == key), "key: another view is another key");
    w[1].pImageInfo = images;
    check(bbg::descriptor_key_of(w.data(), 1, 77).bindings.size() == 1, "key: only the writes given");

    // The memo (upstream PR #100's), as the dispatch uses it: per slot, per
    // view lifetime, never overwritten.
    gpu::ImmutableDescriptorMemo<std::uint64_t, 2> slot0, slot1;
    slot0.observe_epoch(1);
    slot1.observe_epoch(1);
    slot0.remember(key, 501);
    check(slot0.find(key) == 501 && slot1.find(key) == 0, "memo: a set serves only its own slot's pool");
    slot0.remember(key, 502);
    check(slot0.find(key) == 501, "memo: a remembered set is never replaced");
    slot0.forget_view(0x2100);
    check(slot0.find(key) == 0, "memo: a view let go forgets every set holding it");
    slot0.remember(key, 503);
    slot0.observe_epoch(2);
    check(slot0.find(key) == 0 && slot0.size() == 0, "memo: the view epoch moving empties it");
    slot0.remember(key, 504);
    slot0.clear();
    check(slot0.find(key) == 0, "memo: the slot's pool reset empties it");
    if (!fail) std::printf("bbg_cs_set_memo_test: ok\n");
    return fail ? 1 : 0;
}
