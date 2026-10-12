// bbgoated (BBHOST_BBG_FAULT_WINDOW_KB): sets the write watch's fault window
// once presets are applied (host/perf/bbg_perf.h): write_watch_install runs
// before the configuration is read, and its handler must not read switches.
#include "core/write_watch.h"
#include "host/perf/bbg_perf.h"
#include "host/perf/core/fault_window_core.h"
#include "log.h"

namespace {
void configure() {
    const std::uint32_t pages = bbg::window_pages_for_kb(bbg::perf_uint("FAULT_WINDOW_KB", 0));
    if (!pages) return;
    write_watch_set_window_pages(pages);
    host_log("bbg: a write-watch fault releases the armed pages to the end of its %u KiB window (BBHOST_BBG_FAULT_WINDOW_KB)", pages * 4);
}
const bool g_registered = bbg::perf_register_hook(&configure);
}  // namespace
