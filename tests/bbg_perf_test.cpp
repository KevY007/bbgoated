// host/perf/core/perf_plan.h: which variables a BBHOST_BBG_PERF list and a
// [perf] section set, and which source wins (bbgoated).
#include "host/perf/core/perf_plan.h"

#include <cstdio>
#include <set>
#include <string>

static std::set<std::string> g_env;
static bool present(const std::string& v) { return g_env.count(v) != 0; }

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    check(bbg::perf_var("cb_snapshot") == "BBHOST_BBG_CB_SNAPSHOT", "short name gets the prefix, upper case");
    check(bbg::perf_var("BBHOST_PASS_BARRIERS") == "BBHOST_PASS_BARRIERS", "upstream names are kept");
    const bbg::PerfPresets presets = {{"gpu-memory", {{"PARAMS_MEM", "rebar"}, {"CB_SNAPSHOT", "rebar"}}},
                                      {"barriers", {{"BBHOST_PASS_BARRIERS", "lazy"}, {"CB_SNAPSHOT", "device"}}}};
    std::vector<std::string> unknown;
    // direct items beat presets; earlier presets beat later ones; the environment beats everything
    g_env = {"BBHOST_BBG_PARAMS_MEM"};
    auto plan = bbg::perf_plan("gpu-memory, barriers, CB_SNAPSHOT=off, nosuch", {}, presets, present, &unknown);
    auto value = [&](const char* var) -> std::string {
        for (const auto& p : plan) if (p.var == var) return p.value;
        return "<unset>";
    };
    check(value("BBHOST_BBG_CB_SNAPSHOT") == "off", "a direct item beats both presets");
    check(value("BBHOST_BBG_PARAMS_MEM") == "<unset>", "a variable already in the environment is left alone");
    check(value("BBHOST_PASS_BARRIERS") == "lazy", "a preset may set an upstream switch");
    check(unknown.size() == 1 && unknown[0] == "nosuch", "unknown preset reported");
    // [perf]: its keys after BBHOST_BBG_PERF's direct items, its preset last
    g_env.clear();
    unknown.clear();
    plan = bbg::perf_plan("PARAMS_MEM=device", {{"params_mem", "rebar"}, {"frames_ahead", "1"}, {"preset", "gpu-memory"}}, presets, present,
                          &unknown);
    check(value("BBHOST_BBG_PARAMS_MEM") == "device", "BBHOST_BBG_PERF beats [perf]");
    check(value("BBHOST_BBG_FRAMES_AHEAD") == "1", "[perf] key applies");
    check(value("BBHOST_BBG_CB_SNAPSHOT") == "rebar", "[perf] preset applies to what is still unset");
    check(bbg::perf_unquote("\"rebar\"") == "rebar" && bbg::perf_unquote("1") == "1", "TOML values unquoted");
    if (!fail) std::printf("bbg_perf_test: ok\n");
    return fail ? 1 : 0;
}
