// host/perf/core/frame_pacing_core.h: which flip's GPU work the command
// processor waits for at FRAMES_AHEAD=k; and host/perf/frame_pacing.cpp's
// guard, against stand-ins for the renderer's counters, the clock and the
// switch: it waits only while that work is not done, and gives up after
// 250 ms, so a GPU taken for stuck cannot hold the command processor
// (bbgoated).
#include "host/perf/frame_pacing.cpp"

#include <chrono>
#include <cstdio>
#include <thread>

// The renderer's completed submissions: g_completed, one more each time it
// is asked while g_step is 1.
static std::uint64_t g_completed = 0, g_step = 0;
static int g_polls = 0;
std::uint64_t host_gpu_submissions_completed() {
    ++g_polls;
    g_completed += g_step;
    return g_completed;
}
std::uint64_t host_clock_monotonic_ns() {
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
}
void host_sleep_us(std::uint64_t usec) { std::this_thread::sleep_for(std::chrono::microseconds(usec)); }
std::uint64_t hle_video_flip_count() { return 0; }
namespace bbg {
std::uint64_t perf_uint(const char*, std::uint64_t) { return 1; }  // BBHOST_BBG_FRAMES_AHEAD=1
bool perf_report_due(std::uint64_t&) { return false; }
}  // namespace bbg

static double ms_since(std::chrono::steady_clock::time_point t0) {
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
}

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    bbg::FrameNeeds n;
    check(n.push(10, 1) == 0, "the first flip has nothing before it");
    check(n.push(21, 1) == 10, "k=1: flip N waits for flip N-1's work");
    check(n.push(33, 1) == 21, "k=1 again");
    bbg::FrameNeeds m;
    m.push(5, 2);
    m.push(9, 2);
    check(m.push(14, 2) == 5, "k=2: flip N waits for flip N-2's work");
    check(m.push(20, 0) == 0, "k=0: off");
    check(m.push(20, 9) == 0, "k past the ring: off");
    bbg::FrameNeeds q;
    q.push(7, 1);
    check(q.push(7, 1) == 0, "no submission since flip N-1: its need may be the recording under way, never waited for");

    // The guard, at k=1 (the stand-in switch).
    { bbg::FramesAheadGuard first(10, true); }
    check(g_polls == 0, "the first flip does not wait");
    g_completed = 10;
    { bbg::FramesAheadGuard done(20, true); }
    check(g_polls == 1, "flip N-1's work has run: one look, no wait");
    auto t0 = std::chrono::steady_clock::now();
    { bbg::FramesAheadGuard stuck(30, true); }  // needs 20; the GPU stays at 10
    const double stuck_ms = ms_since(t0);
    check(stuck_ms >= 250 && stuck_ms < 5000, "a GPU that never gets there is given up on after 250 ms");
    g_completed = 25;
    g_step = 1;
    const int polls = g_polls;
    t0 = std::chrono::steady_clock::now();
    { bbg::FramesAheadGuard late(40, true); }  // needs 30: there at the fifth look
    check(g_polls - polls == 5 && ms_since(t0) < 250, "the wait ends as soon as flip N-1's work has run");
    const int before = g_polls;
    { bbg::FramesAheadGuard request(50, false); }  // a game thread's request, maybe ahead of the commands
    { bbg::FramesAheadGuard nothing(0, true); }
    check(g_polls == before, "a game thread's request and a flip with no GPU work never wait");
    if (!fail) std::printf("bbg_frame_pacing_test: ok\n");
    return fail ? 1 : 0;
}
