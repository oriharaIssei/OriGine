#include "Interpolation.h"

using namespace OriGine;

Quaternion OriGine::SlerpByDeltaTime(const Quaternion& _q0, const Quaternion& _v, float _deltaTime, float _speed) {
    float alpha = 1.0f - std::exp(-_speed * _deltaTime);
    return Slerp(_q0, _v, alpha);
}

float OriGine::LerpAngle(float _current, float _target, float _t) {
    // 角度差を [-pi, +pi] に正規化
    float diff = std::fmod(_target - _current, kTau);
    if (diff > kPi) {
        diff -= kTau;
    } else if (diff < -kPi) {
        diff += kTau;
    }

    // 補間
    return _current + diff * _t;
}

float OriGine::LerpAngleByDeltaTime(float _current, float _target, float _deltaTime, float _speed) {
    float alpha = 1.0f - std::exp(-_speed * _deltaTime);

    // 角度差を [-pi, +pi] に正規化
    float diff = std::fmod(_target - _current, kTau);
    if (diff > kPi) {
        diff -= kTau;
    } else if (diff < -kPi) {
        diff += kTau;
    }

    // 補間
    return _current + diff * alpha;
}
