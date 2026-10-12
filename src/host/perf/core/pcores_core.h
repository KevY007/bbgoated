#pragma once
// bbgoated (BBHOST_BBG_PCORES): the CPU sets of the highest efficiency class -
// a hybrid CPU's performance cores - or none when every set shares one class.
#include <algorithm>
#include <cstdint>
#include <string_view>
#include <vector>

namespace bbg {
// BBHOST_BBG_PCORES's value, read as every bbgoated switch is (bbg::perf_on):
// set and not 0, off, false or no. core/host_clock.cpp reads the environment
// itself, without the perf glue.
inline bool pcores_wanted(const char* value) {
    if (!value || !*value) return false;
    const std::string_view v(value);
    return v != "0" && v != "off" && v != "false" && v != "no";
}

struct CpuSet {
    std::uint32_t id;
    std::uint8_t efficiency_class;
};
inline std::vector<std::uint32_t> performance_cpu_sets(const std::vector<CpuSet>& sets) {
    if (sets.empty()) return {};
    std::uint8_t lo = 255, hi = 0;
    for (const CpuSet& s : sets) {
        lo = std::min(lo, s.efficiency_class);
        hi = std::max(hi, s.efficiency_class);
    }
    if (lo == hi) return {};
    std::vector<std::uint32_t> out;
    for (const CpuSet& s : sets) {
        if (s.efficiency_class == hi) out.push_back(s.id);
    }
    return out;
}
}  // namespace bbg
