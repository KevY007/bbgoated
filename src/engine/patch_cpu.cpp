// See patch_cpu.h.
#include "engine/patch_cpu.h"

#include <cpuid.h>
#include <cstring>

bool patch_cpu_vendor_ok(const std::string& want, const std::string& have) { return want.empty() || want == have; }

std::string host_cpu_vendor() {
    unsigned a = 0, b = 0, c = 0, d = 0;
    if (!__get_cpuid(0, &a, &b, &c, &d)) return {};
    char v[13] = {};
    std::memcpy(v, &b, 4);  // the vendor string is EBX, EDX, ECX
    std::memcpy(v + 4, &d, 4);
    std::memcpy(v + 8, &c, 4);
    return v;
}
