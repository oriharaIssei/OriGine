#pragma once

/// math
#include "Vector.h"
#include "Vector3.h"
#include <Matrix4x4.h>

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
    static constexpr Quaternion Inverse(const Quaternion& q);
    constexpr Quaternion inverse() const;

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

    static constexpr float NormSq(const Quaternion& q);
    constexpr float normSq() const;

    static Quaternion Normalize(const Quaternion& q);
    Quaternion normalize() const;

    static constexpr float Dot(const Quaternion& q0, const Quaternion& v);
    constexpr float dot(const Quaternion& q) const;

    Vec3f ToEulerAngles() const;

    // インライン関数として定義
    static Vec3f RotateVector(const Vec3f& vec, const Quaternion& q);
    constexpr Vec3f RotateVector(const Vec3f& vec) const {
        Quaternion r = Quaternion(vec, 0.0f);
        r            = *this * r * this->Conjugation();
        return Vec3f(r[X], r[Y], r[Z]);
    }

    inline static constexpr Quaternion RotateAxisAngle(const Vec3f& axis, float angle) {
        float halfAngle = angle / 2.0f;
        return Quaternion(
            axis * sinf(halfAngle),
            cosf(halfAngle));
    }
    inline static const Quaternion RotateAxisVector(const Vec3f& _from, const Vec3f& _to) {
        float angle = std::acosf(_from.Dot(_to));
        Vec3f axis  = _from.cross(_to).normalize();

        float halfAngle = angle / 2.0f;
        return Quaternion(
            axis * sinf(halfAngle),
            cosf(halfAngle));
    }

    inline static constexpr Quaternion FromMatrix(const Matrix4x4& _rotateMat) {
        float trace = _rotateMat.m[0][0] + _rotateMat.m[1][1] + _rotateMat.m[2][2];

        if (trace > 0.0f) {
            float s    = std::sqrt(trace + 1.0f) * 2.0f;
            float invS = 1.0f / s;

            return Quaternion(
                (_rotateMat.m[2][1] - _rotateMat.m[1][2]) * invS, // x
                (_rotateMat.m[0][2] - _rotateMat.m[2][0]) * invS, // y
                (_rotateMat.m[1][0] - _rotateMat.m[0][1]) * invS, // z
                0.25f * s // w
            );
        } else {
            if (_rotateMat.m[0][0] > _rotateMat.m[1][1] && _rotateMat.m[0][0] > _rotateMat.m[2][2]) {
                float s    = std::sqrt(1.0f + _rotateMat.m[0][0] - _rotateMat.m[1][1] - _rotateMat.m[2][2]) * 2.0f;
                float invS = 1.0f / s;

                return Quaternion(
                    0.25f * s,
                    (_rotateMat.m[0][1] + _rotateMat.m[1][0]) * invS,
                    (_rotateMat.m[0][2] + _rotateMat.m[2][0]) * invS,
                    (_rotateMat.m[2][1] - _rotateMat.m[1][2]) * invS);
            } else if (_rotateMat.m[1][1] > _rotateMat.m[2][2]) {
                float s    = std::sqrt(1.0f + _rotateMat.m[1][1] - _rotateMat.m[0][0] - _rotateMat.m[2][2]) * 2.0f;
                float invS = 1.0f / s;

                return Quaternion(
                    (_rotateMat.m[0][1] + _rotateMat.m[1][0]) * invS,
                    0.25f * s,
                    (_rotateMat.m[1][2] + _rotateMat.m[2][1]) * invS,
                    (_rotateMat.m[0][2] - _rotateMat.m[2][0]) * invS);
            } else {
                float s    = std::sqrt(1.0f + _rotateMat.m[2][2] - _rotateMat.m[0][0] - _rotateMat.m[1][1]) * 2.0f;
                float invS = 1.0f / s;

                return Quaternion(
                    (_rotateMat.m[0][2] + _rotateMat.m[2][0]) * invS,
                    (_rotateMat.m[1][2] + _rotateMat.m[2][1]) * invS,
                    0.25f * s,
                    (_rotateMat.m[1][0] - _rotateMat.m[0][1]) * invS);
            }
        }
    }
    inline static Quaternion FromEulerAngles(float pitch, float yaw, float roll) {
        // 半分の角度を計算
        float halfPitch = pitch * 0.5f;
        float halfYaw   = yaw * 0.5f;
        float halfRoll  = roll * 0.5f;

        // サインとコサインを計算
        float sinPitch = sin(halfPitch);
        float cosPitch = cos(halfPitch);
        float sinYaw   = sin(halfYaw);
        float cosYaw   = cos(halfYaw);
        float sinRoll  = sin(halfRoll);
        float cosRoll  = cos(halfRoll);

        // クォータニオンを計算
        Quaternion q;
        q[X] = cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll;
        q[Y] = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;
        q[Z] = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
        q[W] = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;

        return q;
    }
    inline static Quaternion FromEulerAngles(const Vec3f& euler) {
        return FromEulerAngles(euler[Y], euler[X], euler[Z]);
    }
    inline static constexpr Quaternion LookAt(const Vec3f& _forward, const Vec3f& up) { // Z軸を向けるべき方向にする
        Vec3f forward = Vec3f::Normalize(_forward);

        // 右ベクトルを計算（外積）
        Vec3f right = Vec3f::Normalize(Vec3f::Cross(up, forward));

        // 上ベクトルを再計算
        Vec3f newUp = Vec3f::Cross(forward, right);

        // 回転行列を作成
        Matrix4x4 lookAtMatrix = {
            right[X], newUp[X], forward[X], 0.0f,
            right[Y], newUp[Y], forward[Y], 0.0f,
            right[Z], newUp[Z], forward[Z], 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f};

        // 行列からクォータニオンに変換
        return FromMatrix(lookAtMatrix);
    };
};

constexpr Quaternion operator*(float scalar, const Quaternion& q);
constexpr Quaternion operator/(float scalar, const Quaternion& q);

Quaternion Slerp(const Quaternion& q0, const Quaternion& v, float t);
