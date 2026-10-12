#pragma once
// bbgoated constant snapshots (host/perf/vk/cb_ring.cpp), the part without
// Vulkan: which guest pages a submission still writes on the GPU's timeline,
// the content hash and per-recording dedup, the ring each submission slot
// fills, and when a binding is copied.
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <unordered_map>
#include <vector>

namespace bbg {

// Guest pages (64 KiB) a submission that may not have run yet writes on the
// GPU's timeline: a snapshot of them taken now would miss that write, so a
// binding over one stays where the GPU reads it when it runs. A write whose
// range is not known - an untraced store, or one longer than is worth a page
// each - holds every range until its submission has run.
class GpuWrites {
public:
    static constexpr unsigned kPageShift = 16;
    static constexpr std::uint64_t kMaxTrackedBytes = 64ull << 20;
    // `serial`: the submission the write is recorded into (Gpu::flushes while recording).
    void wrote(std::uint64_t va, std::uint64_t bytes, std::uint64_t serial) {
        if (!bytes) return;
        if (bytes > kMaxTrackedBytes) return wrote_anywhere(serial);
        for (std::uint64_t p = va >> kPageShift; p <= (va + bytes - 1) >> kPageShift; ++p) {
            std::uint64_t& s = pages_[p];
            if (serial + 1 > s) s = serial + 1;  // + 1: 0 is "none"
        }
    }
    void wrote_anywhere(std::uint64_t serial) {
        if (serial + 1 > anywhere_) anywhere_ = serial + 1;
    }
    // `completed`: every submission before it has run (Gpu::completed_submits).
    bool pending(std::uint64_t va, std::uint64_t bytes, std::uint64_t completed) const {
        if (!bytes) return false;
        if (anywhere_ > completed) return true;  // serial >= completed
        if (pages_.empty()) return false;
        for (std::uint64_t p = va >> kPageShift; p <= (va + bytes - 1) >> kPageShift; ++p) {
            const auto it = pages_.find(p);
            if (it != pages_.end() && it->second > completed) return true;
        }
        return false;
    }
    bool pending_anywhere(std::uint64_t completed) const { return anywhere_ > completed; }
    void retire(std::uint64_t completed) {
        if (anywhere_ <= completed) anywhere_ = 0;
        for (auto it = pages_.begin(); it != pages_.end();) it = it->second <= completed ? pages_.erase(it) : std::next(it);
    }
    std::size_t size() const { return pages_.size(); }

private:
    std::unordered_map<std::uint64_t, std::uint64_t> pages_;
    std::uint64_t anywhere_ = 0;
};

// A 64-bit content hash (multiply-fold over 8-byte words), for dedup within one recording.
inline std::uint64_t hash_bytes(const void* data, std::size_t n) {
    const auto* p = static_cast<const std::uint8_t*>(data);
    std::uint64_t h = 0x9e3779b97f4a7c15ull ^ n;
    while (n >= 8) {
        std::uint64_t w;
        std::memcpy(&w, p, 8);
        h = (h ^ w) * 0xbf58476d1ce4e5b9ull;
        h ^= h >> 31;
        p += 8;
        n -= 8;
    }
    std::uint64_t t = 0;
    std::memcpy(&t, p, n);
    h = (h ^ t) * 0x94d049bb133111ebull;
    return h ^ (h >> 29);
}

// One recording's snapshots by guest base, size and content: a repeat binds the first copy.
class SnapshotDedup {
public:
    struct Where {
        int chunk;
        std::uint64_t offset;
    };
    bool find(std::uint64_t base, std::uint64_t bytes, std::uint64_t hash, Where& out) const {
        const auto it = map_.find(Key{base, bytes, hash});
        if (it == map_.end()) return false;
        out = it->second;
        return true;
    }
    void add(std::uint64_t base, std::uint64_t bytes, std::uint64_t hash, Where at) { map_.emplace(Key{base, bytes, hash}, at); }
    void reset() { map_.clear(); }

private:
    struct Key {
        std::uint64_t base, bytes, hash;
        bool operator==(const Key& o) const { return base == o.base && bytes == o.bytes && hash == o.hash; }
    };
    struct KeyHash {
        std::size_t operator()(const Key& k) const { return static_cast<std::size_t>(k.hash ^ (k.base * 0x9e3779b97f4a7c15ull) ^ k.bytes); }
    };
    std::unordered_map<Key, Where, KeyHash> map_;
};

// The chunks each submission slot's recording fills, one after another. A
// slot's chunks go back to the free list only when the slot retires - its
// fence has signalled, so nothing the GPU runs reads them any more.
template <class Chunk>
class SlotRing {
public:
    struct Held {
        Chunk chunk;
        std::uint64_t used = 0;
    };
    SlotRing(int slots, std::uint64_t chunk_bytes) : held_(static_cast<std::size_t>(slots)), chunk_bytes_(chunk_bytes) {}
    // A place for `bytes` (rounded up to `align`, a power of two) in `slot`'s
    // last chunk, else at the start of a free chunk, else of one `make` makes
    // while fewer than `cap` exist. False when there is none.
    template <class Make>
    bool alloc(int slot, std::uint64_t bytes, std::uint64_t align, std::uint64_t cap, Make&& make, int& index, std::uint64_t& offset) {
        const std::uint64_t need = (bytes + align - 1) & ~(align - 1);
        if (need > chunk_bytes_) return false;
        std::vector<Held>& held = held_[static_cast<std::size_t>(slot)];
        if (held.empty() || held.back().used + need > chunk_bytes_) {
            Held next;
            if (!free_.empty()) {
                next.chunk = free_.back();
                free_.pop_back();
            } else if (made_ >= cap || !make(next.chunk)) {
                return false;
            } else {
                ++made_;
            }
            held.push_back(next);
        }
        index = static_cast<int>(held.size()) - 1;
        offset = held.back().used;
        held.back().used += need;
        return true;
    }
    void retire(int slot) {
        std::vector<Held>& held = held_[static_cast<std::size_t>(slot)];
        for (const Held& h : held) free_.push_back(h.chunk);
        held.clear();
    }
    const std::vector<Held>& held(int slot) const { return held_[static_cast<std::size_t>(slot)]; }
    std::uint64_t made() const { return made_; }
    std::size_t free_count() const { return free_.size(); }

private:
    std::vector<std::vector<Held>> held_;
    std::vector<Chunk> free_;
    std::uint64_t chunk_bytes_;
    std::uint64_t made_ = 0;
};

// Whether a constant binding is copied as it is recorded, or why it stays
// where the GPU reads it when it runs. The rule that matters: never over a
// range a submission that has not run yet writes on the GPU's timeline - the
// binding left in place would see that write, a copy taken now would not -
// unless the range lies inside a copy version, whose bytes are what both read.
enum SnapOutcome { kSnapTaken, kSnapDispatch, kSnapLarger, kSnapGpuWritten, kSnapUnmapped };
struct SnapshotAsk {
    std::uint64_t bytes = 0, max_bytes = 0;
    bool dispatch = false;        // a dispatch's binding (BBHOST_BBG_CS_CB)
    bool dispatch_on = false;     // BBHOST_BBG_CB_SNAPSHOT_CS
    bool inside_version = false;  // inside a pending copy version
    bool gpu_pending = false;     // GpuWrites::pending over the range
    bool mapped = false;          // the whole range is guest memory the CPU can read
};
inline int snapshot_refusal(const SnapshotAsk& q) {
    if (q.dispatch && !q.dispatch_on) return kSnapDispatch;
    if (!q.bytes || q.bytes > q.max_bytes) return kSnapLarger;
    if (q.gpu_pending && !q.inside_version) return kSnapGpuWritten;
    if (!q.mapped) return kSnapUnmapped;
    return kSnapTaken;
}

// The guest bytes a buffer store through a V# can reach, as host_gpu_dispatch
// reckons a store's: the 40-bit base, num_records times the stride - or
// num_records bytes when the stride is 0.
struct GuestRange {
    std::uint64_t base, bytes;
};
inline GuestRange vsharp_store_range(const std::uint32_t* v) {
    const std::uint64_t base = v[0] | (static_cast<std::uint64_t>(v[1] & 0xff) << 32);
    const std::uint32_t stride = (v[1] >> 16) & 0x3fff;
    return {base, stride ? std::uint64_t{v[2]} * stride : v[2]};
}

}  // namespace bbg
