#pragma once
// bbgoated: the buffer shadow's alternative rules for pages the game keeps
// rewriting (BBHOST_BBG_SHADOW_HOT / _PINGPONG / _RETRY_S), after bbport's
// memory module (report E §4). Pure: host/perf/vk/shadow_policy.cpp keeps one
// of these per page that was ever copied.
#include <cstdint>

namespace bbg {
struct PagePolicy {
    std::uint64_t flips = 0;       // bit k: copied at flip (at - k)
    std::uint64_t at = 0;          // the flip bit 0 stands for (+ 1; 0: never copied)
    std::uint32_t bounces = 0;     // re-copies after a GPU write, in the window
    std::uint64_t bounce_ms = 0;   // the window's start
    bool gpu_since_copy = false;   // a GPU write over the page since its last copy
    std::uint64_t dynamic_ms = 0;  // when it went to host memory
    bool dynamic_rewritten = false;
};

// bbport's hot rule: a copy at `flip`; true when the page was copied in
// `need` of the last `window` flips (this one counted once).
inline bool frames_rule_copy(PagePolicy& s, std::uint64_t flip, unsigned need = 3, unsigned window = 60) {
    const std::uint64_t f = flip + 1;
    if (!s.at || f >= s.at + 64) s.flips = 0;
    else if (f > s.at) s.flips <<= (f - s.at);
    if (f >= s.at) s.at = f;
    s.flips |= 1;
    const std::uint64_t mask = window >= 64 ? ~0ull : ((1ull << window) - 1);
    return static_cast<unsigned>(__builtin_popcountll(s.flips & mask)) >= need;
}

// Ping-pong: a copy after a GPU write over the page is a bounce; `limit`
// bounces within `window_ms` send it to host memory.
inline bool pingpong_copy(PagePolicy& s, std::uint64_t now_ms, unsigned limit, std::uint64_t window_ms = 8000) {
    if (!s.gpu_since_copy) return false;
    s.gpu_since_copy = false;
    if (now_ms - s.bounce_ms >= window_ms) {
        s.bounce_ms = now_ms;
        s.bounces = 0;
    }
    return ++s.bounces >= limit;
}

// A page sent to host memory for being rewritten may be copied again after `retry_ms`.
inline bool retry_due(const PagePolicy& s, std::uint64_t now_ms, std::uint64_t retry_ms) {
    return retry_ms && s.dynamic_rewritten && now_ms - s.dynamic_ms >= retry_ms;
}
}  // namespace bbg
