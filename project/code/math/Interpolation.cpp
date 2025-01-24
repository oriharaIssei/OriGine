#include "Interpolation.h"

Vec3f CubicSpline(const Vec3f& previousPoint, const Vec3f& previousTangent, const Vec3f& nextPoint, const Vec3f& nextTangent, float interpolationValue) {
    float t  = interpolationValue;
    float t2 = t * t;
    float t3 = t2 * t;

    return (2.f * t3 - 3.f * t2 + 1.f) * previousPoint +
           (t3 - 2.f * t2 + t) * previousTangent +
           (-2.f * t3 + 3.f * t2) * nextPoint +
           (t3 - t2) * nextTangent;
}

Quaternion CubicSpline(const Quaternion& previousPoint, const Quaternion& previousTangent, const Quaternion& nextPoint, const Quaternion& nextTangent, float interpolationValue) {
    float t2 = interpolationValue * interpolationValue;
    float t3 = t2 * interpolationValue;

    return Quaternion::Normalize(
        (2.f * t3 - 3.f * t2 + 1.f) * previousPoint +
        (t3 - 2.f * t2 + interpolationValue) * previousTangent +
        (-2.f * t3 + 3.f * t2) * nextPoint +
        (t3 - t2) * nextTangent);
}
