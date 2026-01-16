#include "Interpolation.h"

using namespace OriGine;

Quaternion OriGine::SlerpByDeltaTime(const Quaternion& _q0, const Quaternion& _v, float _deltaTime, float _speed) {
    float alpha = 1.0f - std::exp(-_speed * _deltaTime);
    return Slerp(_q0, _v, alpha);
}

float OriGine::LerpAngle(float _current, float _target, float _t) {
    // 角度差を [-pi, +pi] に正規化
    float diff = std::fmod(_target - _current, 2.0f * kPi);
    if (diff > kPi) {
        diff -= 2.0f * kPi;
    } else if (diff < -kPi) {
        diff += 2.0f * kPi;
    }

    // 補間
    return _current + diff * _t;
}
