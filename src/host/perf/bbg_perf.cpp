// See bbg_perf.h.
#include "host/perf/bbg_perf.h"

#include "core/config.h"
#include "hle/modules.h"
#include "host/perf/core/perf_plan.h"
#include "host/perf/core/perf_switches.h"
#include "log.h"

#include <cstdlib>
#include <map>
#include <vector>

namespace bbg {

namespace {
std::vector<void (*)()>& hooks() {
    static std::vector<void (*)()> h;
    return h;
}
const PerfPresets& presets();  // below (Task 21 fills it)
bool g_applied = false;        // perf_init set a variable (BBHOST_BBG_PERF, [perf])
bool in_env(const std::string& var) { return std::getenv(var.c_str()) != nullptr; }
void set_env(const std::string& var, const std::string& value) {
#if defined(_WIN32)
    _putenv_s(var.c_str(), value.c_str());
#else
    setenv(var.c_str(), value.c_str(), 1);
#endif
}
bool allowed(const std::string& var) {
    if (var.rfind("BBHOST_BBG_", 0) == 0) return true;
    for (const char* v : kLateUpstream) if (var == v) return true;
    return false;
}
std::string lower(const char* s) {
    std::string out;
    for (; *s; ++s) out += (*s >= 'A' && *s <= 'Z') ? static_cast<char>(*s - 'A' + 'a') : *s;
    return out;
}
}  // namespace

std::string perf_value(const char* name) {
    const char* e = std::getenv(perf_var(name).c_str());
    return e ? std::string(e) : std::string();
}

bool perf_on(const char* name) {
    const std::string v = perf_value(name);
    return !v.empty() && v != "0" && v != "off" && v != "false" && v != "no";
}

std::uint64_t perf_uint(const char* name, std::uint64_t def) {
    const std::string v = perf_value(name);
    if (v.empty()) return def;
    char* end = nullptr;
    const unsigned long long n = std::strtoull(v.c_str(), &end, 10);
    return end && *end == 0 ? static_cast<std::uint64_t>(n) : def;
}

bool perf_register_hook(void (*fn)()) {
    hooks().push_back(fn);
    return true;
}

bool perf_report_due(std::uint64_t& next) {
    const std::uint64_t flip = hle_video_flip_count();
    if (!next) next = 16;
    if (flip < next) return false;
    next = flip + 300;
    return true;
}

std::string perf_report() {
    std::string out;
    for (int i = 0; i < kSwitchCount; ++i) {
        const std::string v = perf_value(kSwitches[i].name);
        if (!v.empty()) out += " " + perf_var(kSwitches[i].name) + "=" + v;
    }
    // The upstream switches a preset may set, beside bbgoated's own. Set by
    // hand with none of bbgoated's, they are upstream's and make no bbg line:
    // with every BBHOST_BBG_* unset the log is the base's.
    if (!out.empty() || g_applied) {
        for (const char* var : kLateUpstream) {
            if (const char* e = std::getenv(var)) out += std::string(" ") + var + "=" + e;
        }
    }
    return out.empty() ? std::string() : "bbg perf:" + out;
}

void perf_init() {
    static bool done = false;
    if (done) return;
    done = true;
    std::map<std::string, std::string> cfg;
    for (int i = 0; i < kSwitchCount; ++i) {
        const std::string v = perf_unquote(config_value("perf." + lower(kSwitches[i].name)));
        if (!v.empty()) cfg[lower(kSwitches[i].name)] = v;
    }
    if (const std::string p = perf_unquote(config_value("perf.preset")); !p.empty()) cfg["preset"] = p;
    const char* env = std::getenv("BBHOST_BBG_PERF");
    std::vector<std::string> unknown;
    for (const PerfSet& p : perf_plan(env ? env : "", cfg, presets(), in_env, &unknown)) {
        if (!allowed(p.var)) {
            host_log("bbg perf: %s=%s (%s) is read before the configuration is: set it in the environment", p.var.c_str(), p.value.c_str(),
                     p.source.c_str());
            continue;
        }
        set_env(p.var, p.value);
        g_applied = true;
    }
    for (const std::string& n : unknown) host_log("bbg perf: no preset named \"%s\"", n.c_str());
    if (const std::string r = perf_report(); !r.empty()) host_log("%s", r.c_str());
    for (void (*fn)() : hooks()) fn();
}

namespace {
const PerfPresets& presets() {
    static const PerfPresets p = {};  // Task 21 replaces this with host/perf/core/perf_presets.h's table
    return p;
}
}  // namespace

}  // namespace bbg
