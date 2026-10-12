// host/perf/core/census_core.h: the census lines BBHOST_BBG_PASS_BREAKS=1 logs
// - who ends render passes and who opens transfer batches, per flip - in the
// form tools/bbg_sites.py reads (bbgoated).
#include "host/perf/core/census_core.h"

#include <cstdint>
#include <cstdio>
#include <string>
#include <unordered_map>

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    const std::uintptr_t base = 0x400000;
    std::unordered_map<std::uintptr_t, std::uint64_t> counts;
    check(bbg::census_line("bbg pass ends:", counts, base, 16) == "bbg pass ends: all=0.0/flip", "no sites: the total alone");
    counts = {{base + 0x10, 30}, {base + 0x2a0, 60}, {base + 0x30, 30}};
    check(bbg::census_line("bbg pass ends:", counts, base, 30) == "bbg pass ends: all=4.0/flip 2a0=2.00/flip 10=1.00/flip 30=1.00/flip",
          "most first, ties by offset; hex offsets from the image base; per flip");
    counts.clear();
    for (std::uintptr_t i = 0; i < 20; ++i) counts[base + 0x100 + i] = 100 + i;
    const std::string top = bbg::census_line("bbg transfer batches:", counts, base, 1);
    check(top.rfind("bbg transfer batches: all=2190.0/flip 113=119.00/flip 112=118.00/flip ", 0) == 0, "the total counts every site");
    check(top.find(" 104=104.00/flip") != std::string::npos && top.find(" 103=") == std::string::npos, "16 sites at most");
    check(bbg::census_flips(316, 16) == 300 && bbg::census_flips(16, 16) == 1 && bbg::census_flips(5, 16) == 1,
          "a report covers the flips since the last, at least one");
    if (!fail) std::printf("bbg_census_test: ok\n");
    return fail ? 1 : 0;
}
