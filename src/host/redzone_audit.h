#pragma once
// bbgoated's red-zone audit (Windows). The game's leaf functions keep locals
// in the 128 bytes below rsp that the System V ABI promises nothing touches;
// a Windows exception builds its frame there, and bbhost's texture write
// watch takes one on the first write to each watched page. A fault inside one
// of those functions' stores can come back with the locals overwritten - the
// ribbon crash bbhost fixed by rewriting its two functions (decomp/sfx/ribbons.cpp).
// BBHOST_REDZONE_AUDIT=1 counts the write faults that land in the known
// red-zone store sites (Supermedo PR #2's list, redzone_sites.inc) and reports
// them every 30 s: evidence for whether any more hardening is needed. Off by
// default; it only observes.
#include <cstdint>

int redzone_site_count();
// The index of the site holding this image offset (guest rip - slide), or -1.
int redzone_site_index(std::uint64_t image_offset);
// BBHOST_REDZONE_AUDIT=1: the vectored handler (ahead of the write watch's) and the reporter.
void redzone_audit_install();
