// host/perf/core/fault_window_core.h: how many pages a fault releases ahead of itself (bbgoated).
#include "host/perf/core/fault_window_core.h"

#include <cstdio>

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
    check(bbg::window_pages_for_kb(1) == 4 && bbg::window_pages_for_kb(16) == 4 && bbg::window_pages_for_kb(1024) == 256,
          "16 KiB at least, 1 MiB exactly");
    check(bbg::window_ahead(0x12345, 256) == 0xba, "a 1 MiB window: to its end");
    if (!fail) std::printf("bbg_fault_window_test: ok\n");
    return fail ? 1 : 0;
}
