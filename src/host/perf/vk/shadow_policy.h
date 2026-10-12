#pragma once
// bbgoated (BBHOST_BBG_SHADOW_*): the buffer shadow's rules for pages the game
// keeps rewriting, and an audit of what still binds from host memory. Under g.mu.
#include <cstdint>

namespace gpu {
enum ShadowHot { kShadowHotUpstream = 0, kShadowHotFrames = 1, kShadowHotNever = 2 };
int shadow_hot_rule();      // BBHOST_BBG_SHADOW_HOT
bool shadow_policy_any();   // a hot rule, ping-pong or retry is set
// A copy of page `p` of chunk `chunk`: true when bbgoated's rules send it to
// host memory (upstream's own rule is kept by the caller when hot is upstream).
bool shadow_policy_copy_locked(std::uint32_t chunk, std::uint32_t p, std::uint64_t now_ms);
// That copy was not made after all (the live share had no room for it): the
// rules forget it, as upstream takes it off its own count.
void shadow_policy_copy_refused_locked(std::uint32_t chunk, std::uint32_t p);
void shadow_policy_gpu_wrote_locked(std::uint32_t chunk, std::uint32_t p);
void shadow_policy_went_dynamic_locked(std::uint32_t chunk, std::uint32_t p, bool rewritten, std::uint64_t now_ms);
std::uint64_t shadow_retry_ms();  // BBHOST_BBG_SHADOW_RETRY_S, in ms; 0 off
bool shadow_policy_retry_due_locked(std::uint32_t chunk, std::uint32_t p, std::uint64_t now_ms);
void shadow_policy_retried_locked(std::uint32_t chunk, std::uint32_t p);
std::uint64_t shadow_floor_bytes();  // BBHOST_BBG_SHADOW_FLOOR_MB
enum ShadowAuditWhy {
    kAuditNotMirrorable, kAuditNoMirror, kAuditDynamic, kAuditGpuWritten, kAuditNoRoom, kAuditNoImport, kAuditUnwatchable, kAuditRewritten,
    kAuditWhyCount
};
bool shadow_audit_on();  // BBHOST_BBG_SHADOW_AUDIT=1
void shadow_audit_host_locked(std::uint64_t va, std::uint64_t bytes, int why);
void shadow_audit_mirror_locked();  // a bind from a mirror: the line's other half, and its clock
}  // namespace gpu
