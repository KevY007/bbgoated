// host/perf/core/tex_upload_core.h: where a texture upload's commands go
// (bbgoated, BBHOST_BBG_TEX_UPLOAD).
#include "host/perf/core/tex_upload_core.h"

#include <cstdio>

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    check(bbg::tex_upload_mode("") == bbg::kUploadInPlace && bbg::tex_upload_mode("0") == bbg::kUploadInPlace &&
              bbg::tex_upload_mode("stream") == bbg::kUploadStream && bbg::tex_upload_mode("early") == bbg::kUploadEarly &&
              bbg::tex_upload_mode("Early") == bbg::kUploadInPlace,
          "the switch's values; anything else is upstream's");
    bbg::UploadAsk a;
    a.first = true;
    check(bbg::upload_route(a) == bbg::kUploadInPlace, "unset: in place, as upstream");
    a.mode = bbg::kUploadStream;
    check(bbg::upload_route(a) == bbg::kUploadStream, "stream: recorder ops at the same place");
    a.first = false;
    a.keep = a.profiling = a.gpu_pending = true;
    check(bbg::upload_route(a) == bbg::kUploadStream, "stream: whatever the upload, the order is upstream's");
    a = bbg::UploadAsk{};
    a.mode = bbg::kUploadEarly;
    a.first = true;
    check(bbg::upload_route(a) == bbg::kUploadEarly, "early: a new image's first upload, nothing in its way");
    a.first = false;
    check(bbg::upload_route(a) == bbg::kUploadInPlace, "first_only: a re-upload could come after draws of this recording that sampled the image");
    a.first = true;
    a.gpu_pending = true;
    check(bbg::upload_route(a) == bbg::kUploadInPlace, "gpu_written_source: a pending GPU write over the source bytes keeps it in place");
    a.gpu_pending = false;
    a.keep = true;
    check(bbg::upload_route(a) == bbg::kUploadInPlace, "a GPU-written surface or the untile compare keeps it in place");
    a.keep = false;
    a.profiling = true;
    check(bbg::upload_route(a) == bbg::kUploadInPlace, "profiling keeps it in place: its timestamps bracket the upload in the stream");
    a.profiling = false;
    check(bbg::upload_why(a) == bbg::kWhyEarly, "why: early");
    a.first = false;
    check(bbg::upload_why(a) == bbg::kWhyReupload, "why: a re-upload");
    a.first = true;
    a.gpu_pending = true;
    a.keep = true;
    check(bbg::upload_why(a) == bbg::kWhyKept, "why: kept comes first (GPU-written, compare)");
    a.keep = false;
    check(bbg::upload_why(a) == bbg::kWhyPending, "why: a pending GPU write");
    if (!fail) std::printf("bbg_tex_upload_test: ok\n");
    return fail ? 1 : 0;
}
