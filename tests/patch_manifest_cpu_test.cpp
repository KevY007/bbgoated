// engine/patch_cpu.cpp: a patch manifest's cpu_vendor gate (bbgoated).
#include "engine/patch_cpu.h"

#include <cstdio>
#include <string>

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    check(patch_cpu_vendor_ok("", "AuthenticAMD"), "no vendor: applies everywhere");
    check(patch_cpu_vendor_ok("GenuineIntel", "GenuineIntel"), "matching vendor applies");
    check(!patch_cpu_vendor_ok("GenuineIntel", "AuthenticAMD"), "cpu_vendor_mismatch_skips");
    const std::string v = host_cpu_vendor();
    check(v.size() == 12, "host_cpu_vendor is CPUID leaf 0's 12-character vendor string");
    if (!fail) std::printf("patch_manifest_cpu_test: ok (%s)\n", v.c_str());
    return fail ? 1 : 0;
}
