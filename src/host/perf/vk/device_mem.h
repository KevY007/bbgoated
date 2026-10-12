#pragma once
// bbgoated: where the params ring and the page tables live (BBHOST_BBG_PARAMS_MEM,
// BBHOST_BBG_PT_MEM), and copies recorded into a submission's upload buffer -
// the buffer shadow's, which runs before the submission's own work.
#include "host/gpu_internal.h"

namespace gpu {
enum BbgParamsMem { kParamsUpstream = 0, kParamsRebar = 1, kParamsDevice = 2 };
int bbg_params_mem();
bool bbg_pt_rebar();
// DEVICE_LOCAL | HOST_VISIBLE | HOST_COHERENT among `type_bits` - resizable
// BAR, or the 256 MiB window without it - or UINT32_MAX. Says which once.
std::uint32_t bbg_rebar_type(std::uint32_t type_bits);
// BBHOST_BBG_PARAMS_MEM=device: the ring's device-local twin, or null.
extern VkBuffer g_bbg_params_twin;
bool bbg_params_twin_create_locked(VkDeviceSize bytes);
// A copy that runs before this submission's work (adjacent spans merge).
void bbg_presubmit_add_locked(VkBuffer src, VkBuffer dst, VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize bytes);
// A source of such copies, asked at each submission (true: for a static).
bool bbg_presubmit_register(void (*fn)());
// submit_locked, before shadow_end_uploads_locked: the params twin's span, the
// registered sources', all recorded into the upload buffer.
void bbg_presubmit_locked();
}  // namespace gpu
