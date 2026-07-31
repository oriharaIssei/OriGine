#include "ProfileClock.h"

/// api
// Windows.h の min/max マクロ汚染を避ける
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace OriGine::Profiling::Clock {

namespace {
/// <summary>
/// QueryPerformanceFrequency の結果を一度だけ取得する (関数内staticはスレッドセーフに初期化される)
/// </summary>
int64_t QueryFrequencyOnce() {
    LARGE_INTEGER freq{};
    ::QueryPerformanceFrequency(&freq);
    return freq.QuadPart;
}
} // namespace

int64_t NowTicks() {
    LARGE_INTEGER counter{};
    ::QueryPerformanceCounter(&counter);
    return counter.QuadPart;
}

int64_t FrequencyTicks() {
    static const int64_t kFrequency = QueryFrequencyOnce();
    return kFrequency;
}

double TicksToMilliseconds(int64_t _deltaTicks) {
    const int64_t frequency = FrequencyTicks();
    if (frequency == 0) {
        return 0.0;
    }
    return static_cast<double>(_deltaTicks) * 1000.0 / static_cast<double>(frequency);
}

} // namespace OriGine::Profiling::Clock
