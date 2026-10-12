// host/stage_share.h: a vertex range staged once in a recording serves the
// later draws that stage it, or a range inside it, again - read from the same
// bytes of the same source.
#include "host/stage_share.h"

#include <cstdint>
#include <cstdio>
#include <vector>

namespace {

int g_fail = 0;
#define CHECK(c)                                                      \
    do {                                                              \
        if (!(c)) {                                                   \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); \
            ++g_fail;                                                 \
        }                                                             \
    } while (0)

using Seen = std::vector<StagedRange<int>>;
constexpr int kHost = 1;  // the import the range was read through
constexpr int kCopy = 2;  // a copy version of the same guest range

// {va, bytes, src, src_offset, chunk, offset}
void same_range_shares_its_copy() {
    const Seen seen = {{0x1000, 0x400, kHost, 0x51000, 7, 0x100}};
    int chunk = 0;
    std::uint64_t off = 0;
    CHECK(stage_share_find(seen, 0x1000, 0x400, kHost, 0x51000, &chunk, &off));
    CHECK(chunk == 7);
    CHECK(off == 0x100);
}

void range_inside_shares_at_its_offset() {
    const Seen seen = {{0x1000, 0x400, kHost, 0x51000, 7, 0x100}, {0x8000, 0x2000, kHost, 0x58000, 9, 0x600}};
    int chunk = 0;
    std::uint64_t off = 0;
    CHECK(stage_share_find(seen, 0x8100, 0x1f00, kHost, 0x58100, &chunk, &off));  // ends where the copy ends
    CHECK(chunk == 9);
    CHECK(off == 0x700);
}

void range_past_a_copy_is_copied() {
    const Seen seen = {{0x1000, 0x400, kHost, 0x51000, 7, 0x100}};
    int chunk = 0;
    std::uint64_t off = 0;
    CHECK(!stage_share_find(seen, 0x1000, 0x401, kHost, 0x51000, &chunk, &off));  // one byte longer
    CHECK(!stage_share_find(seen, 0x0fff, 0x10, kHost, 0x50fff, &chunk, &off));   // starts before it
    CHECK(!stage_share_find(seen, 0x13ff, 0x2, kHost, 0x513ff, &chunk, &off));    // runs off its end
    CHECK(!stage_share_find(Seen{}, 0x1000, 0x10, kHost, 0x51000, &chunk, &off));
}

// A GX copy-back between two draws gives the guest range new bytes in a copy
// version: the later draw reads that, not the earlier draw's copy.
void another_source_is_copied_again() {
    const Seen seen = {{0x1000, 0x400, kHost, 0x51000, 7, 0x100}};
    int chunk = 0;
    std::uint64_t off = 0;
    CHECK(!stage_share_find(seen, 0x1000, 0x400, kCopy, 0x0, &chunk, &off));
    CHECK(!stage_share_find(seen, 0x1000, 0x400, kHost, 0x60000, &chunk, &off));  // same buffer, other bytes
    CHECK(!stage_share_find(seen, 0x1100, 0x100, kHost, 0x51000, &chunk, &off));  // inside, but not where it maps
}

void no_wrap_at_the_top_of_memory() {
    const Seen seen = {{0xfffffffffffff000ull, 0x1000, kHost, 0x0, 3, 0}};
    int chunk = 0;
    std::uint64_t off = 0;
    CHECK(!stage_share_find(seen, 0xfffffffffffff800ull, 0x1000, kHost, 0x800, &chunk, &off));
    CHECK(stage_share_find(seen, 0xfffffffffffff800ull, 0x800, kHost, 0x800, &chunk, &off));
    CHECK(off == 0x800);
}

}  // namespace

int main() {
    same_range_shares_its_copy();
    range_inside_shares_at_its_offset();
    range_past_a_copy_is_copied();
    another_source_is_copied_again();
    no_wrap_at_the_top_of_memory();
    if (g_fail) {
        std::printf("stage_share_test: %d failure(s)\n", g_fail);
        return 1;
    }
    std::printf("stage_share_test: ok\n");
    return 0;
}
