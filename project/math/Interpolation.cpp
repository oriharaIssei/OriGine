#include "Interpolation.h"

using namespace OriGine;

Quaternion OriGine::SlerpByDeltaTime(const Quaternion& q0, const Quaternion& v, float deltaTime, float speed) {
    float alpha = 1.0f - std::exp(-speed * deltaTime);
    return Slerp(q0, v, alpha);
}

float OriGine::LerpAngle(float current, float target, float t) {
    // 角度差を [-pi, +pi] に正規化
    float diff = std::fmod(target - current, kTau);
    if (diff > kPi) {
        diff -= kTau;
    } else if (diff < -kPi) {
        diff += kTau;
    }

    // 補間
    return current + diff * t;
}

float OriGine::LerpAngleByDeltaTime(float current, float target, float deltaTime, float speed) {
    float alpha = 1.0f - std::exp(-speed * deltaTime);

    // 角度差を [-pi, +pi] に正規化
    float diff = std::fmod(target - current, kTau);
    if (diff > kPi) {
        diff -= kTau;
    } else if (diff < -kPi) {
        diff += kTau;
    }

    // 補間
    return current + diff * alpha;
}
