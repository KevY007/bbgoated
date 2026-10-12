#pragma once
// bbgoated (BBHOST_BBG_FRAMES_AHEAD): the flips' GPU needs - how many
// submissions must have run for a flip's frame to have run - newest last.
#include <cstdint>

namespace bbg {
class FrameNeeds {
public:
    // Records flip N's need; returns flip N-ahead's, or 0 when there is none
    // (ahead 0 or past the ring: off). Also 0 when flip N-ahead's need is not
    // below flip N's own: nothing was submitted between them, so it may count
    // the recording still open, which only the thread that would wait for it
    // hands over (BBHOST_SUBMIT_EVERY_JOB=0 merges a backlog's jobs into one).
    // Below it, every submission it counts was handed over before flip N.
    std::uint64_t push(std::uint64_t need, unsigned ahead) {
        ring_[count_ % kRing] = need;
        ++count_;
        if (ahead == 0 || ahead >= kRing || count_ <= ahead) return 0;
        const std::uint64_t then = ring_[(count_ - 1 - ahead) % kRing];
        return then < need ? then : 0;
    }

private:
    static constexpr unsigned kRing = 8;
    std::uint64_t ring_[kRing] = {};
    std::uint64_t count_ = 0;
};
}  // namespace bbg
