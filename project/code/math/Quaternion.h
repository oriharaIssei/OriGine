#pragma once

#include "Vector.h"
#include "Vector3.h"

/// <summary>
/// 四元数
/// </summary>
struct Quaternion final : public Vector<4, float> {
    using Vector<4, float>::v;
    using Vector<4, float>::operator[];
    using Vector<4, float>::operator+;
    using Vector<4, float>::operator+=;
    using Vector<4, float>::operator-;
    using Vector<4, float>::operator-=;
    using Vector<4, float>::operator*;
    using Vector<4, float>::operator*=;
    using Vector<4, float>::operator/;
    using Vector<4, float>::operator/=;
    using Vector<4, float>::operator=;
    using Vector<4, float>::operator==;
    using Vector<4, float>::operator!=;

    Quaternion() {}
    Quaternion(const Vector<4, float>& vec) : Vector<4, float>(vec) {}
    Quaternion(float _x, float _y, float _z, float _w)
        : Vector<4, float>(_x, _y, _z, _w) {}
    Quaternion(const Vec3f& v, float _w)
        : Vector<4, float>(v[X], v[Y], v[Z], _w) {}
    Quaternion(const Quaternion& q)
        : Vector<4, float>(q[X], q[Y], q[Z], q[W]) {}

    static Quaternion Identity() {
        return Quaternion(
            0.0f,
            0.0f,
            0.0f,
            1.0f);
    }
    static Quaternion Inverse(const Quaternion& q);
    Quaternion inverse() const;

    static Quaternion Conjugation(const Quaternion& q) {
        return Quaternion(
            -q[X],
            -q[Y],
            -q[Z],
            q[W]);
    }
    Quaternion Conjugation() const {
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

    static float Dot(const Quaternion& q0, const Quaternion& q1);
    float dot(const Quaternion& q) const;

    static Quaternion RotateAxisAngle(const Vec3f& axis, float angle);
    void Show();

    Vec3f ToEulerAngles() const;

    // インライン関数として定義
    friend inline Vec3f RotateVector(const Vec3f& v, const Quaternion& q) {
        Quaternion r = Quaternion(v, 0.0f);
        r            = q * r * q.Conjugation();
        return Vec3f(r[X], r[Y], r[Z]);
    }
};

Quaternion operator*(float scalar, const Quaternion& q);
Quaternion operator/(float scalar, const Quaternion& q);

Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t);
