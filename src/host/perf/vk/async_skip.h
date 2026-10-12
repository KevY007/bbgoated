#pragma once
// bbgoated (BBHOST_BBG_ASYNC_SKIP=1): a draw whose pipeline a worker is still
// building is skipped rather than built on the command processor (report E
// S4); see host/perf/core/async_skip_core.h for the rules. Under g.mu.
#include <cstdint>

namespace gpu {
struct AsyncSkipDraw {
    std::uint64_t pipeline_key = 0, pass = 0;
    std::uint32_t count = 0;
    bool indirect = false, tessellated = false, stores = false, occlusion = false;
};
bool async_skip_on();
bool async_skip_draw_locked(const AsyncSkipDraw& d);  // true: skip it
void async_skip_note_pass_locked(std::uint64_t pass);  // each drawn draw's pass
}  // namespace gpu
