// host/perf/core/pcores_core.h: the performance cores' CPU sets (bbgoated).
#include "host/perf/core/pcores_core.h"

#include <cstdio>
#include <vector>

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    std::vector<bbg::CpuSet> hybrid;
    for (std::uint32_t i = 0; i < 8; ++i) hybrid.push_back({0x100 + i, 1});
    for (std::uint32_t i = 0; i < 12; ++i) hybrid.push_back({0x200 + i, 0});
    const auto p = bbg::performance_cpu_sets(hybrid);
    check(p.size() == 8 && p.front() == 0x100 && p.back() == 0x107, "8 P-cores of 20 (Core Ultra 7 265KF)");
    std::vector<bbg::CpuSet> uniform = {{1, 0}, {2, 0}, {3, 0}};
    check(bbg::performance_cpu_sets(uniform).empty(), "one class: nothing to choose");
    check(bbg::performance_cpu_sets({}).empty(), "no information: nothing");
    const std::vector<bbg::CpuSet> three = {{1, 0}, {2, 1}, {3, 2}, {4, 2}};
    const auto top = bbg::performance_cpu_sets(three);
    check(top.size() == 2 && top[0] == 3 && top[1] == 4, "three classes: only the highest");
    // The switch reads as every BBHOST_BBG_* one does (perf_on): set and not 0, off, false or no.
    check(!bbg::pcores_wanted(nullptr) && !bbg::pcores_wanted("") && !bbg::pcores_wanted("0") && !bbg::pcores_wanted("off") &&
              !bbg::pcores_wanted("false") && !bbg::pcores_wanted("no"),
          "unset, empty, 0, off, false, no: off");
    check(bbg::pcores_wanted("1") && bbg::pcores_wanted("on") && bbg::pcores_wanted("true"), "1, on, true: on");
    if (!fail) std::printf("bbg_pcores_test: ok\n");
    return fail ? 1 : 0;
}
