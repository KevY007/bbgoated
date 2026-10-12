#pragma once
// bbgoated (BBHOST_BBG_PASS_BREAKS=1): the census lines' text. host/perf/census.cpp
// counts the callers; tools/bbg_sites.py reads the lines and names the offsets.
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bbg {

// "<tag> all=<n>/flip <offset>=<n>/flip ...": every count divided by `flips`,
// the 16 sites with the most first (ties by offset), each a hex offset from
// `base` - the image base, as in the "cmd census:" lines.
inline std::string census_line(const char* tag, const std::unordered_map<std::uintptr_t, std::uint64_t>& counts, std::uintptr_t base,
                               double flips) {
    std::vector<std::pair<std::uint64_t, std::uintptr_t>> v;
    std::uint64_t all = 0;
    for (const auto& [site, n] : counts) {
        v.emplace_back(n, site);
        all += n;
    }
    std::sort(v.begin(), v.end(), [](const auto& a, const auto& b) { return a.first != b.first ? a.first > b.first : a.second < b.second; });
    char b[64];
    std::snprintf(b, sizeof(b), " all=%.1f/flip", static_cast<double>(all) / flips);
    std::string out = std::string(tag) + b;
    for (std::size_t i = 0; i < v.size() && i < 16; ++i) {
        std::snprintf(b, sizeof(b), " %llx=%.2f/flip", static_cast<unsigned long long>(v[i].second - base), static_cast<double>(v[i].first) / flips);
        out += b;
    }
    return out;
}

// The flips a report covers: those since the last report, at least one.
inline double census_flips(std::uint64_t flip, std::uint64_t from) { return static_cast<double>(flip > from ? flip - from : 1); }

}  // namespace bbg
