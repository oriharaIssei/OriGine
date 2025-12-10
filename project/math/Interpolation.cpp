#include "Interpolation.h"

using namespace OriGine;

Quaternion OriGine::SlerpByDeltaTime(const Quaternion& q0, const Quaternion& v, float _deltaTime, float _speed) {
    float alpha = 1.0f - std::exp(-_speed * _deltaTime);
    return Slerp(q0, v, alpha);
}

float OriGine::LerpAngle(float current, float target, float t) {
    // 角度差を [-pi, +pi] に正規化
    float diff = std::fmod(target - current, 2.0f * kPi);
    if (diff > kPi) {
        diff -= 2.0f * kPi;
    } else if (diff < -kPi) {
        diff += 2.0f * kPi;
    }

    // 補間
    return current + diff * t;
}
