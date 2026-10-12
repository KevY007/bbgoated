#pragma once
// bbgoated (BBHOST_BBG_TEX_UPLOAD=stream|early, host/perf/vk/tex_upload.cpp),
// the part without Vulkan: where a texture upload's commands go. In place is
// upstream's; stream records the same commands at the same place in the
// stream as recorder ops; early records them into the submission's upload
// buffer, which runs before all of its work.
#include <string>

namespace bbg {

enum UploadMode { kUploadInPlace, kUploadStream, kUploadEarly };

inline int tex_upload_mode(const std::string& v) { return v == "stream" ? kUploadStream : v == "early" ? kUploadEarly : kUploadInPlace; }

struct UploadAsk {
    int mode = kUploadInPlace;
    bool first = false;        // the surface's first upload: its image is new, nothing recorded before used it
    bool keep = false;         // a GPU-written surface, or the BBHOST_GPU_UNTILE=2 compare
    bool profiling = false;    // BBHOST_GPU_PROFILE: its timestamps bracket the upload in the stream
    bool gpu_pending = false;  // a submission that has not run yet writes the source bytes on the GPU
};

// Why an upload under early goes where it goes, the first reason that holds.
// Early moves the upload ahead of everything recorded before it in this
// submission, so it is only for an image nothing recorded so far has used: a
// re-upload could follow draws of this recording that sampled the old
// contents, and they would see the new ones.
enum UploadWhy { kWhyEarly, kWhyKept, kWhyProfile, kWhyReupload, kWhyPending };
inline int upload_why(const UploadAsk& a) {
    if (a.keep) return kWhyKept;
    if (a.profiling) return kWhyProfile;
    if (!a.first) return kWhyReupload;
    if (a.gpu_pending) return kWhyPending;
    return kWhyEarly;
}

// The route, before the upload buffer is asked for (early falls back to in
// place when there is none).
inline int upload_route(const UploadAsk& a) {
    if (a.mode != kUploadEarly) return a.mode == kUploadStream ? kUploadStream : kUploadInPlace;
    return upload_why(a) == kWhyEarly ? kUploadEarly : kUploadInPlace;
}

}  // namespace bbg
