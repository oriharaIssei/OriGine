#pragma once

/// math
#include "Vector.h"
#include "Vector3.h"

namespace OriGine {
/// 前方宣言
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
    /// <summary>
    /// 逆クォータニオンを取得
    /// </summary>
    /// <param name="q">クォータニオン</param>
    /// <returns>逆クォータニオン</returns>
    static Quaternion Inverse(const Quaternion& q);
    /// <summary>
    /// 逆クォータニオンを取得
    /// </summary>
    /// <returns>逆クォータニオン</returns>
    Quaternion inverse() const;

    /// <summary>
    /// 共役クォータニオンを取得
    /// </summary>
    /// <param name="q">クォータニオン</param>
    /// <returns>共役クォータニオン</returns>
    static constexpr Quaternion Conjugation(const Quaternion& q) {
        return Quaternion(
            -q[X],
            -q[Y],
            -q[Z],
            q[W]);
    }
    /// <summary>
    /// 共役クォータニオンを取得
    /// </summary>
    /// <returns>共役クォータニオン</returns>
    constexpr Quaternion Conjugation() const {
        return Quaternion(
            -this->v[X],
            -this->v[Y],
            -this->v[Z],
            this->v[W]);
    }

    /// <summary>
    /// ノルム(長さ)を取得
    /// </summary>
    static float Norm(const Quaternion& q);
    /// <summary>
    /// ノルム(長さ)を取得
    /// </summary>
    float norm() const;

    /// <summary>
    /// ノルムの2乗を取得
    /// </summary>
    static float NormSq(const Quaternion& q);
    /// <summary>
    /// ノルムの2乗を取得
    /// </summary>
    float normSq() const;

    /// <summary>
    /// 正規化
    /// </summary>
    static Quaternion Normalize(const Quaternion& q);
    /// <summary>
    /// 正規化
    /// </summary>
    Quaternion normalize() const;

    /// <summary>
    /// 内積
    /// </summary>
    static float Dot(const Quaternion& q0, const Quaternion& v);
    /// <summary>
    /// 内積
    /// </summary>
    float dot(const Quaternion& q) const;

    /// <summary>
    /// オイラー角へ変換
    /// </summary>
    Vec3f ToEulerAngles() const;
    /// <summary>
    /// ピッチ (x軸回りの回転角度)を取得
    /// </summary>
    /// <returns></returns>
    float ToPitch() const;
    /// <summary>
    /// ヨー (y軸回りの回転角度)を取得
    /// </summary>
    /// <returns></returns>
    float ToYaw() const;
    /// <summary>
    /// ロール (z軸回りの回転角度)を取得
    /// </summary>
    /// <returns></returns>
    float ToRoll() const;

    /// <summary>
    /// ベクトルを回転
    /// </summary>
    static Vec3f RotateVector(const Vec3f& vec, const Quaternion& q);
    /// <summary>
    /// ベクトルを回転
    /// </summary>
    Vec3f RotateVector(const Vec3f& vec) const;

    /// <summary>
    /// 任意軸回りの回転からクォータニオンを作成
    /// </summary>
    static Quaternion RotateAxisAngle(const Vec3f& axis, float angle);
    /// <summary>
    /// 2つのベクトルの間の回転を作成
    /// </summary>
    static const Quaternion RotateAxisVector(const Vec3f& _from, const Vec3f& _to);

    /// <summary>
    /// 法線ベクトルと上方向ベクトルからクォータニオンを作成
    /// </summary>
    static const Quaternion FromNormalVector(const Vec3f& _normal, const Vec3f& _up);

    /// <summary>
    /// 行列からクォータニオンを作成
    /// </summary>
    static Quaternion FromMatrix(const Matrix4x4& _rotateMat);
    /// <summary>
    /// オイラー角からクォータニオンを作成
    /// </summary>
    static Quaternion FromEulerAngles(float pitch, float yaw, float roll);
    /// <summary>
    /// オイラー角からクォータニオンを作成
    /// </summary>
    static Quaternion FromEulerAngles(const Vec3f& euler);

    /// <summary>
    /// 前方向と上方向からクォータニオンを作成
    /// </summary>
    static Quaternion LookAt(const Vec3f& _forward, const Vec3f& up);
};

constexpr Quaternion operator*(float scalar, const Quaternion& q);
constexpr Quaternion operator/(float scalar, const Quaternion& q);

/// <summary>
/// 球面線形補間 (Slerp)
/// </summary>
/// <param name="q0">開始時の回転</param>
/// <param name="v">終了時の回転</param>
/// <param name="t">補間係数(0-1)</param>
/// <returns>補間されたクォータニオン</returns>
Quaternion Slerp(const Quaternion& q0, const Quaternion& v, float t);

using Quatf = Quaternion;
} // namespace OriGine
