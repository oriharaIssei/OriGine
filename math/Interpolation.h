#pragma once

/// my math
#include "math/MathEnv.h"
#include "math/Quaternion.h"

/// math library
#include <cmath>

namespace OriGine {

/// <summary>
/// Frame依存しない線形補間
/// (参考: https://blog.pkh.me/p/41-fixing-the-iterative-damping-interpolation-in-video-games.html )
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="start">開始値</param>
/// <param name="end">終了値</param>
/// <param name="deltaTime">1フレームの経過時間</param>
/// <param name="speed">補間速度</param>
/// <returns>補間後の値</returns>
/// <remarks>
/// 毎フレーム lerp(current, target, 0.1f) のように固定の係数で補間すると、
/// 補間量がフレームレートに依存してしまう(60FPSと30FPSで追従の速さが変わる)。
/// 指数関数を使うと1フレームでの減衰率が経過時間に対して連続になり、
/// フレームレートが変動しても同じ時間で同じ距離だけ近づくようになる。
/// speedは「1秒あたりの減衰の強さ」で、値が大きいほど速く目標へ寄る。
/// alphaは常に0〜1に収まるためオーバーシュートせず、また厳密には目標値に到達しない
/// (漸近するだけなので、完全一致が必要な場合は呼び出し側で打ち切ること)。
/// </remarks>
template <typename T>
T LerpByDeltaTime(const T& start, const T& end, float deltaTime, float speed) {
    float alpha = 1.0f - std::exp(-speed * deltaTime);
    return start + (end - start) * alpha;
}

/// <summary>
/// Frame依存しない球面線形補間
/// </summary>
/// <param name="q0">開始時の回転</param>
/// <param name="v">終了時の回転</param>
/// <param name="deltaTime">1フレームの経過時間</param>
/// <param name="speed">補間速度</param>
/// <returns>補間後のクォータニオン</returns>
Quaternion SlerpByDeltaTime(const Quaternion& q0, const Quaternion& v, float deltaTime, float speed);

/// <summary>
/// 角度を最短経路で線形補間 (ラジアン)
/// </summary>
/// <param name="current">現在の角度</param>
/// <param name="target">目標角度</param>
/// <param name="t">補間係数</param>
/// <returns>補間後の角度</returns>
float LerpAngle(float current, float target, float t);

/// <summary>
/// Frame依存しない角度の最短経路での線形補間 (ラジアン)
/// </summary>
/// <param name="current"></param>
/// <param name="target"></param>
/// <param name="deltaTime"></param>
/// <param name="speed"></param>
/// <returns></returns>
float LerpAngleByDeltaTime(float current, float target, float deltaTime, float speed);

} // namespace OriGine
