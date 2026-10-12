// host/perf/core/cb_ring_core.h: pending GPU writes, the content hash, the
// per-recording dedup of constant snapshots, the per-slot ring and when a
// binding is copied (bbgoated).
#include "host/perf/core/cb_ring_core.h"

#include <cstdio>
#include <cstring>

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    bbg::GpuWrites w;
    w.wrote(0x10010, 16, 5);  // recorded into submission 5
    check(w.pending(0x10000, 64, 5), "pending_until_retired: submission 5 not run while completed == 5");
    check(!w.pending(0x10000, 64, 6), "pending_until_retired: run once completed passes 5");
    check(!w.pending(0x20000, 4, 5), "another 64 KiB page is not pending");
    check(!w.pending(0x10000, 0, 5), "an empty range is never pending");
    w.wrote(0x1fff0, 0x20, 7);  // straddles pages 1 and 2
    check(w.pending(0x20000, 4, 7), "serial_is_recording: the straddled page carries the later serial");
    check(w.pending(0x10000, 4, 6), "serial_is_recording: page 1 keeps its later write (7), not the first (5)");
    w.retire(7);
    check(w.size() == 2 && w.pending(0x20000, 4, 7), "retire keeps what has not run");
    w.retire(8);
    check(w.size() == 0, "retire drops what has run");
    // A store whose range is not known (an untraced V#) could write anywhere.
    w.wrote_anywhere(9);
    check(w.pending(0x7000000000ull, 4, 9), "anywhere_until_retired: any range is pending while submission 9 has not run");
    check(!w.pending(0x7000000000ull, 4, 10), "anywhere_until_retired: none once it has");
    w.retire(10);
    check(!w.pending_anywhere(9), "anywhere_until_retired: retire forgets it");
    w.wrote(0x100000000ull, (64ull << 20) + 1, 11);
    check(w.pending_anywhere(11) && w.size() == 0, "a range past the tracked length counts as anywhere, without a page each");
    w.retire(12);
    w.wrote(0x100000000ull, 64ull << 20, 12);
    check(!w.pending_anywhere(12) && w.size() == 1024 && w.pending(0x103fffffcull, 4, 12), "64 MiB is tracked by its pages");
    w.retire(13);

    const char a[] = "constants-a", b[] = "constants-b";
    check(bbg::hash_bytes(a, sizeof(a)) != bbg::hash_bytes(b, sizeof(b)), "hash differs on content");
    check(bbg::hash_bytes(a, 4) != bbg::hash_bytes(a, 5), "hash differs on length");
    bbg::SnapshotDedup d;
    bbg::SnapshotDedup::Where at{};
    d.add(0x1000, 256, 42, {1, 512});
    check(d.find(0x1000, 256, 42, at) && at.chunk == 1 && at.offset == 512, "dedup_by_content: same range and bytes share");
    check(!d.find(0x1000, 256, 43, at), "dedup_by_content: other bytes do not");
    check(!d.find(0x1040, 256, 42, at), "dedup_by_content: other base does not");
    check(!d.find(0x1000, 128, 42, at), "dedup_by_content: other length does not");
    d.reset();
    check(!d.find(0x1000, 256, 42, at), "reset forgets");

    // The ring: a chunk is filled by one slot's recording and reused only
    // once that slot has retired.
    struct Chunk {
        int id = 0;
    };
    int next_id = 1;
    auto make = [&](Chunk& c) {
        c.id = next_id++;
        return true;
    };
    bbg::SlotRing<Chunk> ring(4, 1024);
    int index = -1;
    std::uint64_t offset = 0;
    check(ring.alloc(0, 100, 256, 8, make, index, offset) && index == 0 && offset == 0, "the first place is the start of a chunk");
    check(ring.alloc(0, 300, 256, 8, make, index, offset) && index == 0 && offset == 256, "the next is aligned after it, in the same chunk");
    check(ring.alloc(0, 600, 256, 8, make, index, offset) && index == 1 && offset == 0 && ring.made() == 2,
          "what does not fit starts another chunk");
    check(ring.alloc(1, 16, 256, 8, make, index, offset) && ring.held(1)[0].chunk.id == 3, "ring_reuse_after_retire: another slot never takes a held chunk");
    ring.retire(0);
    check(ring.held(0).empty() && ring.free_count() == 2, "ring_reuse_after_retire: a retired slot's chunks are free");
    check(ring.alloc(2, 16, 256, 8, make, index, offset) && ring.made() == 3 && ring.held(2)[0].chunk.id != 3 && offset == 0,
          "ring_reuse_after_retire: a free chunk is taken again, from its start");
    check(!ring.alloc(3, 2000, 256, 8, make, index, offset), "a binding larger than a chunk has no place");
    bbg::SlotRing<Chunk> full(2, 1024);
    check(full.alloc(0, 1024, 256, 1, make, index, offset), "a chunk while under the cap");
    check(!full.alloc(1, 16, 256, 1, make, index, offset), "ring full: no chunk free and the cap reached");
    auto refuse = [](Chunk&) { return false; };
    bbg::SlotRing<Chunk> none(2, 1024);
    check(!none.alloc(0, 16, 256, 8, refuse, index, offset) && none.made() == 0, "a chunk that cannot be made is not counted");

    // When a constant binding is copied as recorded (Review Focus 1).
    bbg::SnapshotAsk q;
    q.bytes = 256;
    q.max_bytes = 64 << 10;
    q.mapped = true;
    check(bbg::snapshot_refusal(q) == bbg::kSnapTaken, "a mapped constant buffer is copied");
    q.gpu_pending = true;
    check(bbg::snapshot_refusal(q) == bbg::kSnapGpuWritten, "pending_until_retired: a range the GPU still writes stays where the GPU reads it");
    q.inside_version = true;
    check(bbg::snapshot_refusal(q) == bbg::kSnapTaken, "inside a copy version the version's bytes are the source, whatever is pending");
    q.inside_version = q.gpu_pending = false;
    q.bytes = (64 << 10) + 4;
    check(bbg::snapshot_refusal(q) == bbg::kSnapLarger, "larger than the limit stays");
    q.bytes = 0;
    check(bbg::snapshot_refusal(q) == bbg::kSnapLarger, "an empty range stays");
    q.bytes = 256;
    q.mapped = false;
    check(bbg::snapshot_refusal(q) == bbg::kSnapUnmapped, "unmapped stays");
    q.mapped = true;
    q.dispatch = true;
    check(bbg::snapshot_refusal(q) == bbg::kSnapDispatch, "a dispatch's stays without BBHOST_BBG_CB_SNAPSHOT_CS");
    q.dispatch_on = true;
    check(bbg::snapshot_refusal(q) == bbg::kSnapTaken, "a dispatch's is copied with it");

    // A store's range, as host_gpu_dispatch reckons it from the V#.
    const std::uint32_t raw[4] = {0x12345678u, 0x0000009au, 0x100u, 0};
    const bbg::GuestRange r0 = bbg::vsharp_store_range(raw);
    check(r0.base == 0x9a12345678ull && r0.bytes == 0x100, "store_range: a raw buffer's num_records are bytes");
    const std::uint32_t strided[4] = {0x1000u, 0xc010ff01u, 10u, 0};  // stride 16 in [29:16]; [15:8] and [31:30] are not the base's
    const bbg::GuestRange r1 = bbg::vsharp_store_range(strided);
    check(r1.base == 0x0100001000ull && r1.bytes == 160, "store_range: records times the stride, the base's 40 bits");
    if (!fail) std::printf("bbg_cb_ring_test: ok\n");
    return fail ? 1 : 0;
}
