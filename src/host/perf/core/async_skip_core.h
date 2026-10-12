#pragma once
// bbgoated (BBHOST_BBG_ASYNC_SKIP): which draws may be skipped while their
// pipeline compiles on a worker - bbport 0.5-pre4's rules (report E S4), from
// GX state instead of guesses from PM4.
#include <cstdint>
#include <iterator>
#include <unordered_map>

namespace bbg {
struct SkipDraw {
    std::uint32_t count = 0;  // vertices or indices
    bool indirect = false, tessellated = false, stores = false, occlusion = false;
};

// Passes (pass_key) by the flips they were drawn in, the last 64.
class PassHistory {
public:
    void note(std::uint64_t pass, std::uint64_t flip) {
        E& e = m_[pass];
        const std::uint64_t f = flip + 1;
        if (f <= e.at) return;  // this flip's already (flips only move on)
        e.mask = (!e.at || f >= e.at + 64) ? 1 : ((e.mask << (f - e.at)) | 1);
        e.at = f;
        if (m_.size() > 4096) prune(flip);
    }
    // Drawn this flip and in each of the `frames` flips before it.
    bool every_frame(std::uint64_t pass, std::uint64_t flip, unsigned frames) const {
        const auto it = m_.find(pass);
        if (it == m_.end() || it->second.at != flip + 1) return false;
        const std::uint64_t want = frames >= 63 ? ~0ull : ((1ull << (frames + 1)) - 1);
        return (it->second.mask & want) == want;
    }

private:
    struct E {
        std::uint64_t at = 0, mask = 0;
    };
    void prune(std::uint64_t flip) {
        for (auto it = m_.begin(); it != m_.end();) it = it->second.at + 120 < flip + 1 ? m_.erase(it) : std::next(it);
    }
    std::unordered_map<std::uint64_t, E> m_;
};

inline bool skip_eligible(const SkipDraw& d, std::uint64_t last_frame_draws, bool loading, bool pass_every_frame) {
    return !loading && last_frame_draws >= 300 && !d.indirect && !d.tessellated && !d.stores && !d.occlusion && d.count > 12 &&
           pass_every_frame;
}

// A pipeline is skipped for `limit` flips at most from its first skip.
class SkipBudget {
public:
    bool allow(std::uint64_t key, std::uint64_t flip, unsigned limit) {
        const auto it = first_.try_emplace(key, flip).first;
        return flip - it->second <= limit;
    }

private:
    std::unordered_map<std::uint64_t, std::uint64_t> first_;
};
}  // namespace bbg
