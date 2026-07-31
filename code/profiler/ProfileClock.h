#pragma once

/// stl
#include <cstdint>

namespace OriGine::Profiling::Clock {

/// <summary>
/// 現在時刻を QueryPerformanceCounter の生のtick値として取得する
/// </summary>
/// <returns>QPCのtick値</returns>
int64_t NowTicks();

/// <summary>
/// QueryPerformanceCounterの周波数(1秒あたりのtick数)を取得する
/// </summary>
/// <returns>周波数[tick/sec]</returns>
int64_t FrequencyTicks();

/// <summary>
/// tick差分をミリ秒に変換する
/// </summary>
/// <param name="_deltaTicks">tickの差分</param>
/// <returns>ミリ秒</returns>
double TicksToMilliseconds(int64_t _deltaTicks);

} // namespace OriGine::Profiling::Clock
