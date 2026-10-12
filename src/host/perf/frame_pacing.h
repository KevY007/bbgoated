#pragma once
// bbgoated (BBHOST_BBG_FRAMES_AHEAD=k, 1-3): once the command processor has
// recorded flip N, it waits for the GPU to finish flip N-k's work before it
// walks on - bbport's frames-ahead bound (report E S2) - so the game, held by
// its command-processor backlog, runs at most k frames ahead of the GPU.
// Declared at the top of hle_video_finish_flip: the wait is in the
// destructor, after the flip was queued and shown.
#include <cstdint>

namespace bbg {
class FramesAheadGuard {
public:
    FramesAheadGuard(std::uint64_t gpu_need, bool recorded);
    ~FramesAheadGuard();
    FramesAheadGuard(const FramesAheadGuard&) = delete;
    FramesAheadGuard& operator=(const FramesAheadGuard&) = delete;

private:
    std::uint64_t target_ = 0;
};
}  // namespace bbg
