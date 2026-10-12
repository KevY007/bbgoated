// host/perf/core/fault_window_core.h: how many pages a fault releases ahead
// of itself; and core/write_watch.cpp's fault path with the window set, on
// plain host memory - unset, a fault releases what upstream's look-ahead
// does and no more (bbgoated).
#include "core/write_watch.cpp"

#include "host/perf/core/fault_window_core.h"

#include <csignal>
#include <cstdint>
#include <cstdio>

#include <signal.h>
#include <sys/mman.h>
#include <ucontext.h>

namespace {
void on_segv(int sig, siginfo_t* info, void* ctx) {
    auto* uc = static_cast<ucontext_t*>(ctx);
    if (write_watch_on_fault(reinterpret_cast<std::uint64_t>(info->si_addr), (uc->uc_mcontext.gregs[REG_ERR] & 2) != 0)) {
        return;
    }
    std::signal(sig, SIG_DFL);
    std::raise(sig);
}

constexpr std::uint64_t kPage = 4096;

// `pages` pages below 2^40, where the watch's table reaches, on a 16-page
// boundary (whole 64 KiB windows); null when there is no room.
volatile std::uint8_t* map_pages(std::uint64_t pages, std::uint64_t from) {
    void* m = MAP_FAILED;
    for (std::uint64_t hint = from; m == MAP_FAILED && hint < 0x8000000000ull; hint += 0x10000000ull) {
        m = mmap(reinterpret_cast<void*>(hint), pages * kPage, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    }
    return m == MAP_FAILED ? nullptr : static_cast<volatile std::uint8_t*>(m);
}
}  // namespace

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    check(bbg::window_ahead(0x1000, 16) == 15, "a window's first page releases the 15 after it");
    check(bbg::window_ahead(0x1005, 16) == 10, "mid-window: to its end");
    check(bbg::window_ahead(0x100f, 16) == 0, "its last page: none");
    check(bbg::window_ahead(0x1000, 0) == 0 && bbg::window_ahead(0x1000, 1) == 0, "no window");
    check(bbg::window_pages_for_kb(64) == 16 && bbg::window_pages_for_kb(100) == 32 && bbg::window_pages_for_kb(4096) == 256 &&
              bbg::window_pages_for_kb(0) == 0,
          "KiB to a power-of-two page count, 4 KiB pages, at most 1 MiB");
    check(bbg::window_pages_for_kb(1) == 4 && bbg::window_pages_for_kb(1024) == 256, "16 KiB at least, 1 MiB at most");

    if (!write_watch_enabled()) {
        std::printf("write watch disabled (BBHOST_WRITE_WATCH=0); the fault path is not tested\n");
        if (!fail) std::printf("bbg_fault_window_test: ok\n");
        return fail ? 1 : 0;
    }
    struct sigaction sa{};
    sa.sa_sigaction = on_segv;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, nullptr);
    volatile std::uint8_t* const base = map_pages(32, 0x7100000000ull);
    volatile std::uint8_t* const fresh = map_pages(16, 0x7180000000ull);
    check(base && fresh, "memory below 2^40");
    if (!base || !fresh) return 1;
    const auto va = reinterpret_cast<std::uint64_t>(base);
    const auto at = [&](std::uint64_t p) { return va + p * kPage; };
    const auto faults = [] { return write_watch_stats().faults; };
    const auto ahead = [] { return write_watch_stats().ahead_pages; };

    // Unset: the first write in fresh pages releases its own page only (no
    // pages written just below it, so no look-ahead either).
    WriteWatch all;
    check(write_watch_arm_locked(va, 32 * kPage, all), "armed");
    std::uint64_t f0 = faults(), a0 = ahead();
    base[0] = 1;
    check(faults() == f0 + 1 && ahead() == a0 && write_watch_dirty(va, 32 * kPage, all) == 1, "unset: one fault, its own page");

    // 64 KiB: the first write in a window releases the armed pages to its end.
    write_watch_set_window_pages(bbg::window_pages_for_kb(64));
    WriteWatch lo, mid, hi;  // pages 0-15, 16-20, 21-31: one watch per range asked about
    check(write_watch_arm_locked(va, 16 * kPage, lo) && write_watch_arm_locked(at(16), 5 * kPage, mid) &&
              write_watch_arm_locked(at(21), 11 * kPage, hi),
          "armed again");
    f0 = faults();
    a0 = ahead();
    base[0] = 2;
    check(faults() == f0 + 1 && ahead() == a0 + 15, "the window's first page released the 15 after it");
    check(write_watch_dirty(va, 16 * kPage, lo) == 16, "every page of the window reads written");
    check(write_watch_dirty(at(16), 5 * kPage, mid) == 0 && write_watch_dirty(at(21), 11 * kPage, hi) == 0,
          "the next window is still armed and clean");
    base[9 * kPage] = 3;
    check(faults() == f0 + 1, "a write further into a released window does not fault");
    base[21 * kPage] = 4;
    check(faults() == f0 + 2 && ahead() == a0 + 25, "mid-window: one fault");
    check(write_watch_dirty(at(16), 5 * kPage, mid) == 0 && write_watch_dirty(at(21), 11 * kPage, hi) == 11,
          "mid-window: from its own page to the window's end, none before it");

    // A page never armed ends the release, as it ends upstream's look-ahead.
    WriteWatch part;
    const auto fva = reinterpret_cast<std::uint64_t>(fresh);
    check(write_watch_arm_locked(fva, 4 * kPage, part), "four of sixteen pages armed");
    f0 = faults();
    a0 = ahead();
    fresh[0] = 5;
    check(faults() == f0 + 1 && ahead() == a0 + 3 && write_watch_dirty(fva, 4 * kPage, part) == 4, "the armed run went with the fault");
    fresh[4 * kPage] = 6;
    check(faults() == f0 + 1, "the page past it was never protected");

    write_watch_set_window_pages(0);
    WriteWatch again;
    check(write_watch_arm_locked(va, 16 * kPage, again), "armed after the window is unset");
    f0 = faults();
    base[0] = 7;
    check(faults() == f0 + 1 && write_watch_dirty(va, 16 * kPage, again) == 1, "unset again: its own page only");
    munmap(const_cast<std::uint8_t*>(base), 32 * kPage);
    munmap(const_cast<std::uint8_t*>(fresh), 16 * kPage);
    if (!fail) std::printf("bbg_fault_window_test: ok\n");
    return fail ? 1 : 0;
}
