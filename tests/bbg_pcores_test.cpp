// host/perf/core/pcores_core.h: the performance cores' CPU sets (bbgoated).
#include "host/perf/core/pcores_core.h"

#include <cstdio>

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
    const std::vector<bbg::CpuSet> three = {{10, 0}, {11, 2}, {12, 1}, {13, 2}};
    const auto t = bbg::performance_cpu_sets(three);
    check(t.size() == 2 && t[0] == 11 && t[1] == 13, "three classes (low-power E-cores too): the highest only, in order");
    if (!fail) std::printf("bbg_pcores_test: ok\n");
    return fail ? 1 : 0;
}
