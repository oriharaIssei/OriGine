#pragma once

#include "Vector.h"
#include "Vector2.h"

#include <cmath>

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
    constexpr Vector3(valueType xValue, valueType yValue, valueType zValue)
        : Vector<3, valueType>({xValue, yValue, zValue}) {}
    constexpr Vector3(int _x, int _y, int _z)
        : Vector<3, valueType>({(valueType)_x, (valueType)_y, (valueType)_z}) {}
    constexpr Vector3(const Vector2<valueType>& xy, const valueType& z)
        : Vector<3, valueType>({xy[X], xy[Y], z}) {}
    constexpr Vector3(const valueType& x, const Vector2<valueType>& yz)
        : Vector<3, valueType>({x, yz[X], yz[Y]}) {}
    constexpr Vector3(const valueType* x_ptr, const valueType* y_ptr, const valueType* z_ptr)
        : Vector<3, valueType>({*x_ptr, *y_ptr, *z_ptr}) {}
    constexpr Vector3(const valueType* ptr)
        : Vector<3, valueType>({ptr[0], ptr[1], ptr[2]}) {}

    /// <summary>
    /// ベクトルの長さを計算
    /// </summary>
    /// <returns>ベクトルの長さ</returns>
    constexpr valueType length() const { return std::sqrt(v[X] * v[X] + v[Y] * v[Y] + v[Z] * v[Z]); }
    static constexpr valueType Length(const Vector3& v) { return std::sqrt(v.v[X] * v.v[X] + v.v[Y] * v.v[Y] + v.v[Z] * v.v[Z]); }
    /// <summary>
    /// ベクトルの長さの二乗を計算
    /// </summary>
    constexpr valueType lengthSq() const { return (v[X] * v[X] + v[Y] * v[Y] + v[Z] * v[Z]); }
    static constexpr valueType LengthSq(const Vector3& v) { return (v.v[X] * v.v[X] + v.v[Y] * v.v[Y] + v.v[Z] * v.v[Z]); }

    /// <summary>
    /// 内積を計算
    /// </summary>
    constexpr valueType dot() const { return v[X] * v[X] + v[Y] * v[Y] + v[Z] * v[Z]; }
    constexpr valueType dot(const Vector3<valueType>& vec) const {
        return v[X] * vec.v[X] + v[Y] * vec.v[Y] + v[Z] * vec.v[Z];
    }
    static constexpr valueType Dot(const Vector3& v) { return v.v[X] * v.v[X] + v.v[Y] * v.v[Y] + v.v[Z] * v.v[Z]; }
    static constexpr valueType Dot(const Vector3& v, const Vector3& vec) {
        return v.dot(vec);
    }

    /// <summary>
    /// 外積を計算
    /// </summary>
    /// <param name="another"></param>
    /// <returns></returns>
    constexpr Vector3 cross(const Vector3& another) const {
        return Vector3(
            v[Y] * another.v[Z] - v[Z] * another.v[Y],
            v[Z] * another.v[X] - v[X] * another.v[Z],
            v[X] * another.v[Y] - v[Y] * another.v[X]);
    }
    static constexpr Vector3 Cross(const Vector3& v, const Vector3& another) {
        return v.cross(another);
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
    static constexpr Vector3 Normalize(const Vector3& v) {
        valueType len = v.length();
        if (len == 0)
            return v;
        return (v / len);
    }
};

/// <summary>
/// 反射ベクトルを計算
/// </summary>
template <typename valueType>
inline Vector3<valueType> Reflect(const Vector3<valueType>& v, const Vector3<valueType>& normal) {
    return v - 2.0f * (v.dot(normal)) * normal;
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
