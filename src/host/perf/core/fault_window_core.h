#pragma once
// bbgoated (BBHOST_BBG_FAULT_WINDOW_KB): a write-watch fault releases the armed
// pages from its own to the end of its window (bbport's unprotect window,
// report E C5). Pages are 4 KiB; the window is a power of two of them.
#include <cstdint>

namespace bbg {
// The pages after `page` up to its window's end. The write watch's fault
// handler calls it, on whatever thread faulted (a guest thread's stack and
// thread pointer are the guest's): no stack protector, always inlined, as the
// handler's own helpers are (core/write_watch.cpp).
__attribute__((no_stack_protector, always_inline)) inline std::uint64_t window_ahead(std::uint64_t page, std::uint32_t window_pages) {
    if (window_pages < 2) return 0;
    return (page | (window_pages - 1)) - page;
}
inline std::uint32_t window_pages_for_kb(std::uint64_t kb) {
    if (!kb) return 0;
    std::uint32_t pages = 4;  // 16 KiB at least
    while (pages < 256 && pages * 4ull < kb) pages <<= 1;
    return pages;
}
}  // namespace bbg
