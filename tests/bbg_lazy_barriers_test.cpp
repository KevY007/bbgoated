// host/perf/core/lazy_barriers_core.h: when render.cpp's lazy pass barriers
// are on - upstream's BBHOST_PASS_BARRIERS decides when it is set, and
// BBHOST_BBG_LAZY_BARRIERS_NV=1 turns them on for NVIDIA when it is not
// (bbgoated).
#include "host/perf/core/lazy_barriers_core.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <initializer_list>

namespace {
constexpr std::uint32_t kNvidia = 0x10de, kAmd = 0x1002, kIntel = 0x8086, kNoDevice = 0;
bool on(const char* pass_barriers, bool nv, std::uint32_t vendor) {
    return bbg::lazy_barriers_on(bbg::pass_barriers_env(pass_barriers), nv, vendor);
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
    check(!on(nullptr, false, kNvidia), "nothing set: off");
    check(on(nullptr, true, kNvidia), "the switch on an NVIDIA device: on");
    check(!on(nullptr, true, kAmd) && !on(nullptr, true, kIntel), "the switch elsewhere: off (AMD's compressed targets are the known failure)");
    check(!on(nullptr, true, kNoDevice), "the switch before the device is known: off for now");
    check(on("lazy", false, kAmd) && on("lazy", true, kNvidia), "BBHOST_PASS_BARRIERS=lazy: on, whatever the vendor");
    check(!on("full", true, kNvidia) && !on("1", true, kNvidia), "any other BBHOST_PASS_BARRIERS value: off, the switch notwithstanding");
    check(on("", true, kNvidia) && !on("", false, kNvidia), "BBHOST_PASS_BARRIERS empty: as if unset");
    // With the switch unset, upstream's rule exactly: on only for "lazy".
    bool same = true;
    for (const char* e : {static_cast<const char*>(nullptr), "", "lazy", "full", "0", "1", "LAZY", "lazy "}) {
        for (std::uint32_t vendor : {kNoDevice, kNvidia, kAmd, kIntel}) same = same && on(e, false, vendor) == (e && std::strcmp(e, "lazy") == 0);
    }
    check(same, "switch unset: upstream's truth table for every BBHOST_PASS_BARRIERS value");
    if (!fail) std::printf("bbg_lazy_barriers_test: ok\n");
    return fail ? 1 : 0;
}
