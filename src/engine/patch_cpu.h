#pragma once
// A patch manifest's cpu_vendor (engine/patch_manifest.cpp): a patch for one
// CPU vendor's machines only - Kyo's Intel tone-map fix, say, which Intel's
// rounding needs and the PS4's AMD CPU never did.
#include <string>

// true when want is empty (every CPU) or names the host's vendor.
bool patch_cpu_vendor_ok(const std::string& want, const std::string& have);

// CPUID leaf 0's vendor string: "GenuineIntel", "AuthenticAMD", ...
std::string host_cpu_vendor();
