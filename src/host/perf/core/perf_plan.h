#pragma once
// bbgoated performance switches, the part without a process: the variables a
// BBHOST_BBG_PERF list and a [perf] section ask for, most binding first
// (docs/bbgoated/SWITCHES.md). host/perf/bbg_perf.cpp applies the result.
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace bbg {

struct PerfSet {
    std::string var;     // the full environment variable (BBHOST_...)
    std::string value;
    std::string source;  // "BBHOST_BBG_PERF", "[perf]", "preset <name>"
};
// A preset's name and the variables it sets (short names get the prefix).
using PerfPresets = std::map<std::string, std::vector<std::pair<std::string, std::string>>>;

// "cb_snapshot" / "CB_SNAPSHOT" -> "BBHOST_BBG_CB_SNAPSHOT"; a name that
// already starts with BBHOST_ is an upstream switch and is kept.
inline std::string perf_var(const std::string& name) {
    std::string up;
    for (char c : name) up += (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : (c == '-' ? '_' : c);
    return up.rfind("BBHOST_", 0) == 0 ? up : "BBHOST_BBG_" + up;
}

inline std::string perf_trim(const std::string& s) {
    std::size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t')) ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t')) --b;
    return s.substr(a, b - a);
}

// A TOML value as config_value gives it: "x" -> x, 'x' -> x, others as they are.
inline std::string perf_unquote(const std::string& raw) {
    const std::string v = perf_trim(raw);
    if (v.size() >= 2 && (v.front() == '"' || v.front() == '\'') && v.back() == v.front()) return v.substr(1, v.size() - 2);
    return v;
}

inline std::vector<std::string> perf_items(const std::string& list) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : list + ",") {
        if (c != ',') {
            cur += c;
            continue;
        }
        const std::string t = perf_trim(cur);
        if (!t.empty()) out.push_back(t);
        cur.clear();
    }
    return out;
}

// In order: BBHOST_BBG_PERF's NAME=VALUE items, [perf]'s keys, the presets
// BBHOST_BBG_PERF names, then the presets [perf]'s `preset` names. A variable
// already in the environment (`present`) or earlier in the list is skipped:
// the most explicit source wins. Unknown preset names go to `unknown`.
inline std::vector<PerfSet> perf_plan(const std::string& perf_env, const std::map<std::string, std::string>& config_perf,
                                      const PerfPresets& presets, bool (*present)(const std::string&),
                                      std::vector<std::string>* unknown) {
    std::vector<PerfSet> out;
    auto add = [&](const std::string& name, const std::string& value, const std::string& source) {
        const std::string var = perf_var(name);
        if (present && present(var)) return;
        for (const PerfSet& p : out) if (p.var == var) return;
        out.push_back({var, value, source});
    };
    std::vector<std::string> env_presets, cfg_presets;
    for (const std::string& item : perf_items(perf_env)) {
        const std::size_t eq = item.find('=');
        if (eq == std::string::npos) env_presets.push_back(item);
        else add(perf_trim(item.substr(0, eq)), perf_trim(item.substr(eq + 1)), "BBHOST_BBG_PERF");
    }
    for (const auto& [key, value] : config_perf) {
        if (key == "preset") cfg_presets = perf_items(value);
        else add(key, value, "[perf]");
    }
    auto expand = [&](const std::vector<std::string>& names) {
        for (const std::string& n : names) {
            auto it = presets.find(n);
            if (it == presets.end()) {
                if (unknown) unknown->push_back(n);
                continue;
            }
            for (const auto& [var, value] : it->second) add(var, value, "preset " + n);
        }
    };
    expand(env_presets);
    expand(cfg_presets);
    return out;
}

}  // namespace bbg
