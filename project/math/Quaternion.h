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
    /* using Vector<4, float>::operator*;
     using Vector<4, float>::operator*=;*/
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

    Quaternion operator*(const Quaternion& q2) const {
        Vec3f v1  = Vec3f(this->v[X], this->v[Y], this->v[Z]);
        Vec3f v2 = Vec3f(q2[X], q2[Y], q2[Z]);

        float dot  = Vec3f::Dot(v1 - v2);
        float newW = (this->v[W] * q2[W]) - dot;

        Vector3f cross{};
        cross[X] = v1[Y] * v2[Z] - v1[Z] * v2[Y];
        cross[Y] = v1[Z] * v2[X] - v1[X] * v2[Z];
        cross[Z] = v1[X] * v2[Y] - v1[Y] * v2[X];

        Vector3f vector = cross + (v1 * q2[W]) + (v2 * this->v[W]);
        return Quaternion(vector[X], vector[Y], vector[Z], newW);
    }

    // Quaternion operator*(const Quaternion& _q) const {
    //     Quaternion q{
    //         v[W] * _q[X] + v[X] * _q[W] + v[Y] * _q[Z] - v[Z] * _q[Y], // X'
    //         v[W] * _q[Y] + v[Y] * _q[W] + v[Z] * _q[X] - v[X] * _q[Z], // Y'
    //         v[W] * _q[Z] + v[Z] * _q[W] + v[X] * _q[Y] - v[Y] * _q[X], // Z'
    //         v[W] * _q[W] - v[X] * _q[X] - v[Y] * _q[Y] - v[Z] * _q[Z]  // W'
    //     };

    //    return q;
    //}

    Quaternion* operator*=(const Quaternion& _q) {
        Quaternion q = *this * _q;
        *this        = q;
        return this;
    }

    Quaternion operator*(float scalar) const {
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

    static float Dot(const Quaternion& q0, const Quaternion& v);
    float dot(const Quaternion& q) const;

    static Quaternion RotateAxisAngle(const Vec3f& axis, float angle);

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

Quaternion Slerp(const Quaternion& q0, const Quaternion& v, float t);
