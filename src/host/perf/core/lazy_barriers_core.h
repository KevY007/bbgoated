#pragma once
// bbgoated (BBHOST_BBG_LAZY_BARRIERS_NV): when render.cpp's lazy pass barriers
// are on. Upstream's BBHOST_PASS_BARRIERS decides whenever it is set - "lazy"
// on, anything else off - and unset or empty, the bbgoated switch turns them
// on for an NVIDIA device: the known failure is AMD's compressed targets.
#include <cstdint>
#include <cstring>

namespace bbg {

// BBHOST_PASS_BARRIERS as lazy_barriers_on takes it: 1 "lazy", 0 any other
// value, -1 unset or empty (the bbgoated switch decides).
inline int pass_barriers_env(const char* e) { return e && *e ? (std::strcmp(e, "lazy") == 0 ? 1 : 0) : -1; }

// `vendor`: VkPhysicalDeviceProperties::vendorID, 0 before the device is made.
inline bool lazy_barriers_on(int pass_barriers, bool nv_switch, std::uint32_t vendor) {
    if (pass_barriers >= 0) return pass_barriers == 1;
    return nv_switch && vendor == 0x10de;
}

}  // namespace bbg
