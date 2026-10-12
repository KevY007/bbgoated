// host/perf/core/device_mem_core.h: the copies a submission's upload buffer
// runs before its work, the params ring's span a submission wrote, and which
// heap is resizable BAR (bbgoated, BBHOST_BBG_PARAMS_MEM / BBHOST_BBG_PT_MEM).
#include "host/perf/core/device_mem_core.h"

#include <cstdint>
#include <cstdio>
#include <vector>

namespace {
struct Region {  // VkBufferCopy's fields
    std::uint64_t srcOffset, dstOffset, size;
};
using Lists = std::vector<bbg::CopyList<int, Region>>;
bool same(const Region& r, std::uint64_t src, std::uint64_t dst, std::uint64_t size) {
    return r.srcOffset == src && r.dstOffset == dst && r.size == size;
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
    Lists lists;
    bbg::copies_add(lists, 1, 2, 0, 0, 0);
    check(lists.empty(), "an empty copy is no copy");
    bbg::copies_add(lists, 1, 2, 0, 0, 512);
    bbg::copies_add(lists, 1, 2, 512, 512, 1024);
    check(lists.size() == 1 && lists[0].regions.size() == 1 && same(lists[0].regions[0], 0, 0, 1536),
          "a copy continuing the last one in both buffers extends it");
    bbg::copies_add(lists, 1, 2, 4096, 8192, 256);
    bbg::copies_add(lists, 1, 2, 4352, 4352, 256);
    check(lists.size() == 1 && lists[0].regions.size() == 3 && same(lists[0].regions[1], 4096, 8192, 256) &&
              same(lists[0].regions[2], 4352, 4352, 256),
          "a gap in either buffer starts a region of its own");
    bbg::copies_add(lists, 3, 2, 0, 0, 64);
    bbg::copies_add(lists, 1, 4, 0, 0, 64);
    bbg::copies_add(lists, 3, 2, 64, 64, 64);
    check(lists.size() == 3 && lists[1].src == 3 && lists[1].dst == 2 && lists[1].regions.size() == 1 &&
              same(lists[1].regions[0], 0, 0, 128) && lists[2].src == 1 && lists[2].dst == 4,
          "one list per source and destination, in the order first seen");

    // The params ring: slot k's region holds per_slot blocks of stride bytes,
    // block j at (k * per_slot + j) * stride (gpu.cpp alloc_params_slot_locked).
    const std::uint32_t per_slot = 2048 * 2, stride = 512;
    const bbg::Span s = bbg::params_span(3, per_slot, 5, stride);
    bool covers = s.bytes == 5ull * stride;
    for (std::uint64_t j = 0; j < 5; ++j) {
        const std::uint64_t at = (3ull * per_slot + j) * stride;
        covers = covers && at >= s.offset && at + stride <= s.offset + s.bytes;
    }
    check(covers, "the span covers exactly the blocks the submission wrote");
    check(bbg::params_span(0, per_slot, 0, stride).bytes == 0, "no blocks, no span");

    check(!bbg::resizable_bar_heap(256ull << 20) && !bbg::resizable_bar_heap(512ull << 20), "the 256 MiB window is not resizable BAR");
    check(bbg::resizable_bar_heap(16ull << 30), "a heap of all video memory is");
    if (!fail) std::printf("bbg_device_mem_test: ok\n");
    return fail ? 1 : 0;
}
