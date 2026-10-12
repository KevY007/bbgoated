// See census.h.
#include "host/perf/census.h"

#include "core/portable.h"
#include "hle/modules.h"
#include "host/perf/bbg_perf.h"
#include "host/perf/core/census_core.h"
#include "log.h"

#include <cstdint>
#include <unordered_map>

namespace bbg {
namespace {
bool on() {
    static const bool v = perf_on("PASS_BREAKS");
    return v;
}
std::unordered_map<std::uintptr_t, std::uint64_t> g_pass, g_batch;
std::uint64_t g_next = 0, g_from_flip = 0;

void maybe_report() {
    if (!perf_report_due(g_next)) return;
    const std::uint64_t flip = hle_video_flip_count();
    const double flips = census_flips(flip, g_from_flip);
    host_log("%s", census_line("bbg pass ends:", g_pass, host_image_base(), flips).c_str());
    host_log("%s", census_line("bbg transfer batches:", g_batch, host_image_base(), flips).c_str());
    g_pass.clear();
    g_batch.clear();
    g_from_flip = flip;
}
}  // namespace

void census_pass_end_locked(void* site) {
    if (!on()) return;
    ++g_pass[reinterpret_cast<std::uintptr_t>(site)];
    maybe_report();
}

void census_batch_open_locked(void* site) {
    if (!on()) return;
    ++g_batch[reinterpret_cast<std::uintptr_t>(site)];
    maybe_report();
}
}  // namespace bbg
