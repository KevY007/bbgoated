#pragma once
// bbgoated (BBHOST_BBG_FOLD_CLEARS): clears held back for the target's next
// pass - the bookkeeping, keyed by an image's handle as an integer.
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace bbg {
template <class Entry> class HeldClears {
public:
    // A newer clear of the same image replaces the held one (the older is overwritten anyway).
    void hold(std::uint64_t image, const Entry& e) {
        drop(image);
        held_.emplace_back(image, e);
    }
    bool take(std::uint64_t image, Entry& out) {
        for (auto it = held_.begin(); it != held_.end(); ++it) {
            if (it->first != image) continue;
            out = it->second;
            held_.erase(it);
            return true;
        }
        return false;
    }
    // The oldest held clear `pick` chooses (a sampled binding names its target by address).
    template <class Pick> bool take_first(Pick pick, Entry& out) {
        for (auto it = held_.begin(); it != held_.end(); ++it) {
            if (!pick(it->second)) continue;
            out = it->second;
            held_.erase(it);
            return true;
        }
        return false;
    }
    bool has(std::uint64_t image) const {
        for (const auto& p : held_) if (p.first == image) return true;
        return false;
    }
    bool drop(std::uint64_t image) {
        for (auto it = held_.begin(); it != held_.end(); ++it) {
            if (it->first != image) continue;
            held_.erase(it);
            return true;
        }
        return false;
    }
    std::vector<Entry> take_all() {  // oldest first
        std::vector<Entry> out;
        out.reserve(held_.size());
        for (auto& p : held_) out.push_back(p.second);
        held_.clear();
        return out;
    }
    std::size_t size() const { return held_.size(); }
    bool empty() const { return held_.empty(); }

private:
    std::vector<std::pair<std::uint64_t, Entry>> held_;
};

// A newer clear stands for the held one only when it clears every aspect the
// held one did (VkImageAspectFlags): a fill clears depth and stencil, a draw's
// clear may clear depth alone, and the stencil fill must still land.
inline bool fold_covers(std::uint32_t newer_aspects, std::uint32_t held_aspects) { return (held_aspects & ~newer_aspects) == 0; }

// A pass's load op does a held clear when the clear was of the whole image and
// the pass draws layer 0 of it over all of it: its render area is the image.
inline bool fold_by_load_op(std::uint32_t layer, std::uint32_t image_w, std::uint32_t image_h, std::uint32_t area_w, std::uint32_t area_h) {
    return layer == 0 && image_w == area_w && image_h == area_h;
}
}  // namespace bbg
