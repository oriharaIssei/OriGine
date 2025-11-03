#include "Interpolation.h"

Quaternion SlerpByDeltaTime(const Quaternion& q0, const Quaternion& v, float _deltaTime, float _speed) {
    float alpha = 1.0f - std::exp(-_speed * _deltaTime);
    return Slerp(q0, v, alpha);
}
