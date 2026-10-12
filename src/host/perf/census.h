#pragma once
// bbgoated (BBHOST_BBG_PASS_BREAKS=1): who ends render passes and who opens
// transfer batches, by caller - "bbg pass ends:" and "bbg transfer batches:"
// lines of <image offset>=<per flip>, at flip 16 and then every 300 flips;
// tools/bbg_sites.py names the offsets. Under g.mu.
namespace bbg {
void census_pass_end_locked(void* site);
void census_batch_open_locked(void* site);
}  // namespace bbg
