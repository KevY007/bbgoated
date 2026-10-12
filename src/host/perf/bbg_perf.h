#pragma once
// bbgoated performance switches (docs/bbgoated/SWITCHES.md). Each is an
// environment variable BBHOST_BBG_<NAME>, off when unset; BBHOST_BBG_PERF and
// bbgoated.toml's [perf] fill the ones the environment leaves unset, once
// main() has the configuration (perf_init). Read them lazily - in a
// function-local static or a perf hook - never in a static initializer.
#include <cstdint>
#include <string>

namespace bbg {
// The switch names and the upstream switches a preset may set are in
// host/perf/core/perf_switches.h (header-only, so the tests read them too).
std::string perf_value(const char* name);  // "" when unset
bool perf_on(const char* name);            // set and not 0/off/false/no
std::uint64_t perf_uint(const char* name, std::uint64_t def);
bool perf_register_hook(void (*fn)());      // run by perf_init, after the presets
void perf_init();
std::string perf_report();  // "bbg perf: VAR=value ..." or "" when no bbgoated switch is set
// The switches' periodic lines (bbg ...): due at flip 16 - inside a smoke
// boot's 18 flips - and then every 300 flips, as the 300-flip report. `next`
// is the caller's own (start it at 0).
bool perf_report_due(std::uint64_t& next);
}  // namespace bbg
