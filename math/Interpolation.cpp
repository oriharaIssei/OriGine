#include "Interpolation.h"

using namespace OriGine;

Quaternion OriGine::SlerpByDeltaTime(const Quaternion& q0, const Quaternion& v, float deltaTime, float speed) {
    float alpha = 1.0f - std::exp(-speed * deltaTime);
    return Slerp(q0, v, alpha);
}

float OriGine::LerpAngle(float current, float target, float t) {
    // 角度差を [-pi, +pi] に正規化
    // 例えば350度から10度へ回る場合、単純な差は-340度となり、ほぼ1周する遠回りになる。
    // 差を1周(kTau)で割った余りにしたうえで半周を超える分を折り返すことで、
    // 必ず近い方向(この例では+20度)へ回るようにする
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
    // 例えば350度から10度へ回る場合、単純な差は-340度となり、ほぼ1周する遠回りになる。
    // 差を1周(kTau)で割った余りにしたうえで半周を超える分を折り返すことで、
    // 必ず近い方向(この例では+20度)へ回るようにする
    float diff = std::fmod(target - current, kTau);
    if (diff > kPi) {
        diff -= kTau;
    } else if (diff < -kPi) {
        diff += kTau;
    }

    // 補間
    return current + diff * alpha;
}
