// bbgoated: a vertex range staged once in a recording's uploads serves every
// later draw of that recording that stages it, or a range inside it, again
// (buffer_shadow.cpp shadow_stage_locked, BBHOST_VERTEX_STAGE_SHARE). Every
// copy in a slot's uploads runs before the submission's draws, so a second
// copy of the same bytes would read what the first one did. "The same bytes"
// is the same source as well as the same guest range: a GX copy-back between
// two draws gives the range new contents in a copy version, which the later
// draw reads through another buffer, and that draw copies again. The game
// binds a rewritten ring's whole record to each of many draws: at the Astral
// Clocktower that was ~400 copies and ~35 MiB a frame.
#pragma once

#include <cstdint>
#include <vector>

template <class Buffer>
struct StagedRange {
    std::uint64_t va = 0;          // the guest range copied
    std::uint64_t bytes = 0;
    Buffer src{};                  // the buffer it was read through
    std::uint64_t src_offset = 0;  // and where in it
    Buffer chunk{};                // the staging chunk holding the copy
    std::uint64_t offset = 0;      // where the copy starts in it
};

// The copy that holds all of [va, va + bytes), read through `src` at
// `src_offset` like this range would be, if one staged earlier in the
// recording does: its chunk, and the offset of va in it.
template <class Buffer>
bool stage_share_find(const std::vector<StagedRange<Buffer>>& seen, std::uint64_t va, std::uint64_t bytes, const Buffer& src,
                      std::uint64_t src_offset, Buffer* chunk, std::uint64_t* offset) {
    for (const StagedRange<Buffer>& s : seen) {
        if (va < s.va || bytes > s.bytes || va - s.va > s.bytes - bytes) continue;
        if (!(s.src == src) || src_offset != s.src_offset + (va - s.va)) continue;
        *chunk = s.chunk;
        *offset = s.offset + (va - s.va);
        return true;
    }
    return false;
}
