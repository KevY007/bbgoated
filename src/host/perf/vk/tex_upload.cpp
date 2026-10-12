// See tex_upload.h.
#include "host/perf/vk/tex_upload.h"

#include "host/perf/bbg_perf.h"
#include "host/perf/core/tex_upload_core.h"
#include "host/perf/vk/cb_ring.h"
#include "log.h"

namespace gpu {
namespace {
int mode() {
    static const int m = bbg::tex_upload_mode(bbg::perf_value("TEX_UPLOAD"));
    return m;
}
std::uint64_t g_early = 0, g_stream = 0, g_no_cmd = 0, g_next = 0;
std::uint64_t g_kept[bbg::kWhyPending + 1] = {};  // in place under early, by why (upload_why)
void report() {
    if (!bbg::perf_report_due(g_next)) return;
    using ull = unsigned long long;
    std::uint64_t in_place = g_no_cmd;
    for (std::uint64_t n : g_kept) in_place += n;
    host_log("bbg tex upload (%s): early %llu stream %llu in place %llu (re-uploads %llu, GPU-written or compared %llu, GPU writes pending %llu, "
             "profiling %llu, no upload buffer %llu)",
             mode() == bbg::kUploadEarly ? "early" : "stream", static_cast<ull>(g_early), static_cast<ull>(g_stream), static_cast<ull>(in_place),
             static_cast<ull>(g_kept[bbg::kWhyReupload]), static_cast<ull>(g_kept[bbg::kWhyKept]), static_cast<ull>(g_kept[bbg::kWhyPending]),
             static_cast<ull>(g_kept[bbg::kWhyProfile]), static_cast<ull>(g_no_cmd));
}
}  // namespace

UploadCmds upload_cmds_for_locked(std::uint64_t base, std::uint64_t bytes, bool first, bool keep) {
    UploadCmds u;
    if (mode() == bbg::kUploadInPlace) return u;
    bbg::UploadAsk a;
    a.mode = mode();
    a.first = first;
    a.keep = keep;
    a.profiling = g.profile;
    a.gpu_pending = a.mode == bbg::kUploadEarly && first && cb_ring_gpu_write_pending_locked(base, bytes);
    switch (bbg::upload_route(a)) {
        case bbg::kUploadStream:
            u.mode = UploadCmds::kStream;
            ++g_stream;
            break;
        case bbg::kUploadEarly:
            // The slot's upload buffer, begun if it is not: it runs before
            // this submission's work, and its opening barrier and the image
            // barrier's ALL_COMMANDS source order it after the earlier ones.
            if ((u.early_cmd = shadow_upload_cmd_for_bbg_locked())) {
                u.mode = UploadCmds::kEarly;
                ++g_early;
            } else {
                ++g_no_cmd;
            }
            break;
        default:
            ++g_kept[bbg::upload_why(a)];
            break;
    }
    report();
    return u;
}

}  // namespace gpu
