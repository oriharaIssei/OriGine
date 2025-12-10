#pragma once

/// my math
#include "math/mathEnv.h"
#include "math/Quaternion.h"

/// math library
#include <cmath>

namespace OriGine {

/// <summary>
/// Frame依存しない線形補間
/// (参考: https://blog.pkh.me/p/41-fixing-the-iterative-damping-interpolation-in-video-games.html )
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="_start"></param>
/// <param name="_end"></param>
/// <param name="_deltaTime"></param>
/// <param name="speed"></param>
/// <returns></returns>
template <typename T>
T LerpByDeltaTime(const T& _start, const T& _end, float _deltaTime, float _speed) {
    float alpha = 1.0f - std::exp(-_speed * _deltaTime);
    return _start + (_end - _start) * alpha;
}

Quaternion SlerpByDeltaTime(const Quaternion& q0, const Quaternion& v, float _deltaTime, float _speed);

/// <summary>
/// 角度を最短経路で線形補間 (ラジアン)
/// </summary>
/// <param name="current"></param>
/// <param name="target"></param>
/// <param name="t"></param>
/// <returns></returns>
float LerpAngle(float current, float target, float t);

} // namespace OriGine
