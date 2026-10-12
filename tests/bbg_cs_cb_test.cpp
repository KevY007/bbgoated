// host/perf/core/cs_cb_core.h: a dispatch's constant buffers bound as storage
// buffers, in a descriptor set of their own (bbgoated, BBHOST_BBG_CS_CB).
#include "host/perf/core/cs_cb_core.h"

#include <cstdio>

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    gcn::TranslateOptions o;
    o.stage = gcn::Stage::Compute;
    o.rsrc1 = 0x1234;
    o.rsrc2 = 0x10;
    o.descriptor_set = 0;
    bbg::cs_cb_options(o);
    check(o.cb_ssbo, "options: constant buffers through storage buffers");
    check(!o.cb_no_fallback, "options: the page-table walk stays the fallback");
    check(o.cb_descriptor_set == static_cast<int>(bbg::kCsCbSet) && bbg::kCsCbSet == 1, "options: in set 1, not the shared set 0");
    check(o.cb_binding_base == gcn::kBindingStorageBuffer0, "options: from binding 112");
    check(o.stage == gcn::Stage::Compute && o.rsrc1 == 0x1234 && o.rsrc2 == 0x10 && o.descriptor_set == 0, "options: the rest as given");

    const std::vector<std::uint32_t> b = bbg::cs_cb_set_bindings();
    bool contiguous = b.size() == gcn::kMaxBuffers;
    for (std::size_t i = 0; contiguous && i < b.size(); ++i) contiguous = b[i] == gcn::kBindingStorageBuffer0 + i;
    check(contiguous, "set bindings: one storage buffer for each binding a translation can make, 112 to 127");

    std::vector<gcn::BufferBinding> buffers(3);
    for (std::uint32_t i = 0; i < 3; ++i) buffers[i].binding = gcn::kBindingStorageBuffer0 + i;
    std::vector<bbg::CbWrite> w = bbg::cs_cb_writes(buffers, 3);
    check(w.size() == 3 && w[0].binding == 112 && w[0].info == 0 && w[2].binding == 114 && w[2].info == 2,
          "writes: each binding the translation made takes its resolved info, in order");
    w = bbg::cs_cb_writes(buffers, 2);
    check(w.size() == 2, "writes: never past the infos resolved");
    check(bbg::cs_cb_binds(true, 1) && !bbg::cs_cb_binds(true, 0) && !bbg::cs_cb_binds(false, 3),
          "binds: set 1 only under the switch, for a program with constant buffers");
    if (!fail) std::printf("bbg_cs_cb_test: ok\n");
    return fail ? 1 : 0;
}
