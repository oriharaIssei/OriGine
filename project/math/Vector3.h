#pragma once

/// math
#include "Vector.h"
#include "Vector2.h"
#include <cmath>

namespace OriGine {

/// <summary>
/// 3次元ベクトル
/// </summary>
/// <typeparam name="valueType"></typeparam>
template <typename valueType = float>
struct Vector3 final : Vector<3, valueType> {
    using Vector<3, valueType>::v;
    using Vector<3, valueType>::operator[];
    using Vector<3, valueType>::operator+;
    using Vector<3, valueType>::operator+=;
    using Vector<3, valueType>::operator-;
    using Vector<3, valueType>::operator-=;
    using Vector<3, valueType>::operator*;
    using Vector<3, valueType>::operator*=;
    using Vector<3, valueType>::operator/;
    using Vector<3, valueType>::operator/=;
    using Vector<3, valueType>::operator=;
    using Vector<3, valueType>::operator==;
    using Vector<3, valueType>::operator!=;

    // コンストラクタ
    constexpr Vector3() : Vector<3, valueType>({0, 0, 0}) {}
    constexpr Vector3(valueType _xValue, valueType _yValue, valueType _zValue)
        : Vector<3, valueType>({_xValue, _yValue, _zValue}) {}
    constexpr Vector3(int _x, int _y, int _z)
        : Vector<3, valueType>({(valueType)_x, (valueType)_y, (valueType)_z}) {}
    constexpr Vector3(const Vector2<valueType>& _xy, const valueType& _z)
        : Vector<3, valueType>({_xy[X], _xy[Y], _z}) {}
    constexpr Vector3(const valueType& _x, const Vector2<valueType>& _yz)
        : Vector<3, valueType>({_x, _yz[X], _yz[Y]}) {}
    constexpr Vector3(const valueType* _x, const valueType* _y, const valueType* _z)
        : Vector<3, valueType>({*_x, *_y, *_z}) {}
    constexpr Vector3(const valueType* _ptr)
        : Vector<3, valueType>({_ptr[0], _ptr[1], _ptr[2]}) {}

    /// <summary>
    /// ベクトルの長さを計算
    /// </summary>
    /// <returns>ベクトルの長さ</returns>
    constexpr valueType length() const { return std::sqrt(v[X] * v[X] + v[Y] * v[Y] + v[Z] * v[Z]); }
    /// <summary>
    /// ベクトルの長さを計算 (static)
    /// </summary>
    /// <param name="v">ベクトル</param>
    /// <returns>ベクトルの長さ</returns>
    static constexpr valueType Length(const Vector3& _v) { return std::sqrt(_v.v[X] * _v.v[X] + _v.v[Y] * _v.v[Y] + _v.v[Z] * _v.v[Z]); }
    /// <summary>
    /// ベクトルの長さの二乗を計算
    /// </summary>
    constexpr valueType lengthSq() const { return (v[X] * v[X] + v[Y] * v[Y] + v[Z] * v[Z]); }
    /// <summary>
    /// ベクトルの長さの二乗を計算 (static)
    /// </summary>
    /// <param name="v">ベクトル</param>
    /// <returns>長さの二乗</returns>
    static constexpr valueType LengthSq(const Vector3& _v) { return (_v.v[X] * _v.v[X] + _v.v[Y] * _v.v[Y] + _v.v[Z] * _v.v[Z]); }

    /// <summary>
    /// 内積を計算
    /// </summary>
    constexpr valueType dot() const { return v[X] * v[X] + v[Y] * v[Y] + v[Z] * v[Z]; }
    /// <summary>
    /// 指定のベクトルとの内積を計算
    /// </summary>
    /// <param name="vec">対象ベクトル</param>
    /// <returns>内積</returns>
    constexpr valueType dot(const Vector3<valueType>& _vec) const {
        return v[X] * _vec.v[X] + v[Y] * _vec.v[Y] + v[Z] * _vec.v[Z];
    }
    /// <summary>
    /// ベクトルの内積を計算 (static)
    /// </summary>
    /// <param name="v">対象ベクトル</param>
    /// <returns>内積</returns>
    static constexpr valueType Dot(const Vector3& _v) { return _v.v[X] * _v.v[X] + _v.v[Y] * _v.v[Y] + _v.v[Z] * _v.v[Z]; }
    /// <summary>
    /// 2つのベクトルの内積を計算 (static)
    /// </summary>
    /// <param name="v">ベクトル1</param>
    /// <param name="vec">ベクトル2</param>
    /// <returns>内積</returns>
    static constexpr valueType Dot(const Vector3& _v, const Vector3& _vec) {
        return _v.dot(_vec);
    }

    /// <summary>
    /// 外積を計算
    /// </summary>
    /// <param name="another">対象ベクトル</param>
    /// <returns>外積結果ベクトル</returns>
    constexpr Vector3 cross(const Vector3& _another) const {
        return Vector3(
            v[Y] * _another.v[Z] - v[Z] * _another.v[Y],
            v[Z] * _another.v[X] - v[X] * _another.v[Z],
            v[X] * _another.v[Y] - v[Y] * _another.v[X]);
    }
    /// <summary>
    /// 外積を計算 (static)
    /// </summary>
    /// <param name="v">ベクトル1</param>
    /// <param name="another">ベクトル2</param>
    /// <returns>外積結果ベクトル</returns>
    static constexpr Vector3 Cross(const Vector3& _v, const Vector3& _another) {
        return _v.cross(_another);
    }

    /// <summary>
    /// 正規化
    /// </summary>
    constexpr Vector3 normalize() const {
        valueType len = length();
        if (len == 0)
            return *this;
        return (*this / len);
    }
    /// <summary>
    /// 正規化 (static)
    /// </summary>
    /// <param name="v">正規化前ベクトル</param>
    /// <returns>正規化後ベクトル</returns>
    static constexpr Vector3 Normalize(const Vector3& _v) {
        valueType len = _v.length();
        if (len == 0)
            return _v;
        return (_v / len);
    }
};

/// <summary>
/// 反射ベクトルを計算
/// </summary>
template <typename valueType>
inline Vector3<valueType> Reflect(const Vector3<valueType>& _v, const Vector3<valueType>& _normal) {
    return _v - 2.0f * (_v.dot(_normal)) * _normal;
}

//=========== using ===========//
template <typename valueType>
using Vec3 = Vector3<valueType>;

using Vector3f = Vector3<float>;
using Vec3f    = Vector3<float>;

using Vector3d = Vector3<double>;
using Vec3d    = Vector3<double>;

//=========== DirectionVector ===========//
constexpr Vec3f axisX = Vec3f(1.0f, 0.0f, 0.0f);
constexpr Vec3f axisY = Vec3f(0.0f, 1.0f, 0.0f);
constexpr Vec3f axisZ = Vec3f(0.0f, 0.0f, 1.0f);

} // namespace OriGine
