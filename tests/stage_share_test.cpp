// host/stage_share.h: a vertex range staged once in a recording serves the
// later draws that stage it, or a range inside it, again.
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

void same_range_shares_its_copy() {
    const Seen seen = {{0x1000, 0x400, 7, 0x100}};
    int chunk = 0;
    std::uint64_t off = 0;
    CHECK(stage_share_find(seen, 0x1000, 0x400, &chunk, &off));
    CHECK(chunk == 7);
    CHECK(off == 0x100);
}

void range_inside_shares_at_its_offset() {
    const Seen seen = {{0x1000, 0x400, 7, 0x100}, {0x8000, 0x2000, 9, 0x600}};
    int chunk = 0;
    std::uint64_t off = 0;
    CHECK(stage_share_find(seen, 0x8100, 0x1f00, &chunk, &off));  // ends where the copy ends
    CHECK(chunk == 9);
    CHECK(off == 0x700);
}

void range_past_a_copy_is_copied() {
    const Seen seen = {{0x1000, 0x400, 7, 0x100}};
    int chunk = 0;
    std::uint64_t off = 0;
    CHECK(!stage_share_find(seen, 0x1000, 0x401, &chunk, &off));  // one byte longer
    CHECK(!stage_share_find(seen, 0x0fff, 0x10, &chunk, &off));   // starts before it
    CHECK(!stage_share_find(seen, 0x13ff, 0x2, &chunk, &off));    // runs off its end
    CHECK(!stage_share_find(Seen{}, 0x1000, 0x10, &chunk, &off));
}

void no_wrap_at_the_top_of_memory() {
    const Seen seen = {{0xfffffffffffff000ull, 0x1000, 3, 0}};
    int chunk = 0;
    std::uint64_t off = 0;
    CHECK(!stage_share_find(seen, 0xfffffffffffff800ull, 0x1000, &chunk, &off));
    CHECK(stage_share_find(seen, 0xfffffffffffff800ull, 0x800, &chunk, &off));
    CHECK(off == 0x800);
}

}  // namespace

int main() {
    same_range_shares_its_copy();
    range_inside_shares_at_its_offset();
    range_past_a_copy_is_copied();
    no_wrap_at_the_top_of_memory();
    if (g_fail) {
        std::printf("stage_share_test: %d failure(s)\n", g_fail);
        return 1;
    }
    std::printf("stage_share_test: ok\n");
    return 0;
}
