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
/// <param name="_start">開始値</param>
/// <param name="_end">終了値</param>
/// <param name="_deltaTime">1フレームの経過時間</param>
/// <param name="_speed">補間速度</param>
/// <returns>補間後の値</returns>
template <typename T>
T LerpByDeltaTime(const T& _start, const T& _end, float _deltaTime, float _speed) {
    float alpha = 1.0f - std::exp(-_speed * _deltaTime);
    return _start + (_end - _start) * alpha;
}

/// <summary>
/// Frame依存しない球面線形補間
/// </summary>
/// <param name="_q0">開始時の回転</param>
/// <param name="_v">終了時の回転</param>
/// <param name="_deltaTime">1フレームの経過時間</param>
/// <param name="_speed">補間速度</param>
/// <returns>補間後のクォータニオン</returns>
Quaternion SlerpByDeltaTime(const Quaternion& _q0, const Quaternion& _v, float _deltaTime, float _speed);

/// <summary>
/// 角度を最短経路で線形補間 (ラジアン)
/// </summary>
/// <param name="_current">現在の角度</param>
/// <param name="_target">目標角度</param>
/// <param name="_t">補間係数</param>
/// <returns>補間後の角度</returns>
float LerpAngle(float _current, float _target, float _t);

/// <summary>
/// Frame依存しない角度の最短経路での線形補間 (ラジアン)
/// </summary>
/// <param name="_current"></param>
/// <param name="_target"></param>
/// <param name="_deltaTime"></param>
/// <param name="_speed"></param>
/// <returns></returns>
float LerpAngleByDeltaTime(float _current, float _target, float _deltaTime, float _speed);

} // namespace OriGine
