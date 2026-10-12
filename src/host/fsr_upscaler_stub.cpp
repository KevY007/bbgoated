// SPDX-License-Identifier: GPL-3.0-or-later
// bbgoated: the FSR entry points dlaa.cpp and gpu.cpp call, for a build without the
// FSR provider (-DBBGOATED_FSR=OFF, or before Task 6 of the Phase 2 plan). FSR reports
// itself unavailable, and the temporal path keeps DLSS or the game's own picture.
#include "host/fsr_upscaler.h"
#include "log.h"

namespace gpu {

bool fsr_upscale_locked(VkPhysicalDevice, VkDevice, VkCommandBuffer, const NgxbImage&, const NgxbImage&,
                        const DlssFgGuides&, uint64_t, uint64_t) {
    static bool said = false;
    if (!said) {
        said = true;
        host_log("fsr: this build has no FSR provider (-DBBGOATED_FSR=OFF); FSR modes render unchanged");
    }
    return false;
}

void fsr_retire_locked(uint64_t) {}

void fsr_shutdown_locked() {}

}  // namespace gpu
