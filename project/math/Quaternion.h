#pragma once

/// math
#include "Vector.h"
#include "Vector3.h"

struct Matrix4x4;

/// <summary>
/// 四元数
/// </summary>
struct Quaternion final
    : public Vector<4, float> {
    using Vector<4, float>::v;
    using Vector<4, float>::operator[];
    using Vector<4, float>::operator+;
    using Vector<4, float>::operator+=;
    using Vector<4, float>::operator-;
    using Vector<4, float>::operator-=;
    /* using Vector<4, float>::operator*;
     using Vector<4, float>::operator*=;*/
    using Vector<4, float>::operator/;
    using Vector<4, float>::operator/=;
    using Vector<4, float>::operator=;
    using Vector<4, float>::operator==;
    using Vector<4, float>::operator!=;

    constexpr Quaternion() : Vector<4, float>() {}
    constexpr Quaternion(const Vector<4, float>& vec) : Vector<4, float>(vec) {}
    constexpr Quaternion(float _x, float _y, float _z, float _w)
        : Vector<4, float>(_x, _y, _z, _w) {}
    constexpr Quaternion(const Vec3f& v, float _w)
        : Vector<4, float>(v[X], v[Y], v[Z], _w) {}
    constexpr Quaternion(const Quaternion& q)
        : Vector<4, float>(q[X], q[Y], q[Z], q[W]) {}

    constexpr Quaternion operator*(const Quaternion& q2) const {
        Vec3f v1 = Vec3f(this->v[X], this->v[Y], this->v[Z]);
        Vec3f v2 = Vec3f(q2[X], q2[Y], q2[Z]);
        // 修正: ドット積はv1とv2の間で計算する
        float dot  = Vec3f::Dot(v1, v2);
        float newW = (this->v[W] * q2[W]) - dot;

        Vector3f cross{};
        cross[X] = v1[Y] * v2[Z] - v1[Z] * v2[Y];
        cross[Y] = v1[Z] * v2[X] - v1[X] * v2[Z];
        cross[Z] = v1[X] * v2[Y] - v1[Y] * v2[X];

        Vector3f vector = cross + (v1 * q2[W]) + (v2 * this->v[W]);
        return Quaternion(vector[X], vector[Y], vector[Z], newW);
    }

    Quaternion* operator*=(const Quaternion& _q) {
        Quaternion q = *this * _q;
        *this        = q;
        return this;
    }

    constexpr Quaternion operator*(float scalar) const {
        return Quaternion(
            v[X] * scalar,
            v[Y] * scalar,
            v[Z] * scalar,
            v[W] * scalar);
    }
    Quaternion* operator*=(float scalar) {
        Quaternion q = *this * scalar;
        *this        = q;
        return this;
    }

    static constexpr Quaternion Identity() {
        return Quaternion(
            0.0f,
            0.0f,
            0.0f,
            1.0f);
    }
    static Quaternion Inverse(const Quaternion& q);
    Quaternion inverse() const;

    static constexpr Quaternion Conjugation(const Quaternion& q) {
        return Quaternion(
            -q[X],
            -q[Y],
            -q[Z],
            q[W]);
    }
    constexpr Quaternion Conjugation() const {
        return Quaternion(
            -this->v[X],
            -this->v[Y],
            -this->v[Z],
            this->v[W]);
    }

    static float Norm(const Quaternion& q);
    float norm() const;

    static float NormSq(const Quaternion& q);
    float normSq() const;

    static Quaternion Normalize(const Quaternion& q);
    Quaternion normalize() const;

    static float Dot(const Quaternion& q0, const Quaternion& v);
    float dot(const Quaternion& q) const;

    Vec3f ToEulerAngles() const;
    float ToPitch() const;
    float ToYaw() const;
    float ToRoll() const;

    static Vec3f RotateVector(const Vec3f& vec, const Quaternion& q);
    Vec3f RotateVector(const Vec3f& vec) const;

    static Quaternion RotateAxisAngle(const Vec3f& axis, float angle);
    static const Quaternion RotateAxisVector(const Vec3f& _from, const Vec3f& _to);

    static const Quaternion FromNormalVector(const Vec3f& _normal, const Vec3f& _up);

    static Quaternion FromMatrix(const Matrix4x4& _rotateMat);
    static Quaternion FromEulerAngles(float pitch, float yaw, float roll);
    static Quaternion FromEulerAngles(const Vec3f& euler);

    static Quaternion LookAt(const Vec3f& _forward, const Vec3f& up);
};

constexpr Quaternion operator*(float scalar, const Quaternion& q);
constexpr Quaternion operator/(float scalar, const Quaternion& q);

Quaternion Slerp(const Quaternion& q0, const Quaternion& v, float t);
