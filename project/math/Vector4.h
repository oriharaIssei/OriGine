#pragma once

#include "Vector.h"
#include "Vector2.h"
#include "Vector3.h"

#include <cmath>

/// <summary>
/// 四次元ベクトル
/// </summary>
/// <typeparam name="valueType"></typeparam>
template <typename valueType = float>
struct Vector4 final
    : Vector<4, valueType> {
    using Vector<4, valueType>::v;
    using Vector<4, valueType>::operator[];
    using Vector<4, valueType>::operator+;
    using Vector<4, valueType>::operator+=;
    using Vector<4, valueType>::operator-;
    using Vector<4, valueType>::operator-=;
    using Vector<4, valueType>::operator*;
    using Vector<4, valueType>::operator*=;
    using Vector<4, valueType>::operator/;
    using Vector<4, valueType>::operator/=;
    using Vector<4, valueType>::operator=;
    using Vector<4, valueType>::operator==;
    using Vector<4, valueType>::operator!=;

    // コンストラクタ
    constexpr Vector4() : Vector<4, valueType>({0, 0, 0, 0}) {}
    constexpr Vector4(valueType xValue, valueType yValue, valueType zValue, valueType wValue)
        : Vector<4, valueType>({xValue, yValue, zValue, wValue}) {}
    constexpr Vector4(const Vector2<valueType>& xy, const Vector2<valueType>& zw)
        : Vector<4, valueType>({xy[X], xy[Y], zw[X], zw[Y]}) {}
    constexpr Vector4(valueType x, const Vector2<valueType>& yz, valueType w)
        : Vector<4, valueType>({x, yz[X], yz[Y], w}) {}
    constexpr Vector4(const Vector3<valueType>& xyz, valueType w)
        : Vector<4, valueType>({xyz[X], xyz[Y], xyz[Z], w}) {}
    constexpr Vector4(const Vector2<valueType>& xy, valueType z, valueType w)
        : Vector<4, valueType>({xy[X], xy[Y], z, w}) {}
    constexpr Vector4(valueType x, valueType y, const Vector2<valueType>& zw)
        : Vector<4, valueType>({x, y, zw[X], zw[Y]}) {}
    constexpr Vector4(valueType x, const Vector3<valueType>& yzw)
        : Vector<4, valueType>({x, yzw[X], yzw[Y], yzw[Z]}) {}
    constexpr Vector4(const valueType* x_ptr, const valueType* y_ptr, const valueType* z_ptr, const valueType* w_ptr)
        : Vector<4, valueType>({*x_ptr, *y_ptr, *z_ptr, *w_ptr}) {}
    constexpr Vector4(const valueType* ptr)
        : Vector<4, valueType>({ptr[0], ptr[1], ptr[2], ptr[3]}) {}

    /// <summary>
    /// ベクトルの長さを計算
    /// </summary>
    constexpr valueType length() const { return std::sqrt(v[X] * v[X] + v[Y] * v[Y] + v[Z] * v[Z] + v[W] * v[W]); }
    static constexpr valueType Length(const Vector4& v) { return std::sqrt(v.v[X] * v.v[X] + v.v[Y] * v.v[Y] + v.v[Z] * v.v[Z] + v.v[W] * v.v[W]); }

    /// <summary>
    /// ベクトルの長さの二乗を計算
    /// </summary>
    /// <returns></returns>
    constexpr valueType lengthSq() const { return (v[X] * v[X] + v[Y] * v[Y] + v[Z] * v[Z] + v[W] * v[W]); }
    static constexpr valueType LengthSq(const Vector4& v) { return (v.v[X] * v.v[X] + v.v[Y] * v.v[Y] + v.v[Z] * v.v[Z] + v.v[W] * v.v[W]); }

    /// <summary>
    /// 内積を計算
    /// </summary>
    /// <param name="another"></param>
    /// <returns></returns>
    constexpr valueType dot(const Vector4& another) const { return (v[X] * another.v[X]) + (v[Y] * another.v[Y]) + (v[Z] * another.v[Z]) + (v[W] * another.v[W]); }
    static constexpr valueType Dot(const Vector4& v, const Vector4& another) { return (v.v[X] * another.v[X]) + (v.v[Y] * another.v[Y]) + (v.v[Z] * another.v[Z]) + (v.v[W] * another.v[W]); }

    /// <summary>
    /// 正規化
    /// </summary>
    /// <returns></returns>
    constexpr Vector4 normalize() const {
        valueType len = length();
        if (len == 0)
            return *this;
        return (*this / len);
    }
    static constexpr Vector4 Normalize(const Vector4& v) {
        valueType len = v.length();
        if (len == 0)
            return v;
        return (v / len);
    }
};

//=========== using ===========//
template <typename valueType>
using Vec4 = Vector4<valueType>;

using Vector4f = Vector4<float>;
using Vec4f    = Vector4<float>;

using Vector4d = Vector4<double>;
using Vec4d    = Vector4<double>;

//=========== Color ===========//
constexpr Vec4f WHITE = Vec4f(1.f, 1.f, 1.f, 1.f);
constexpr Vec4f BLACK = Vec4f(0.f, 0.f, 0.f, 1.f);
constexpr Vec4f RED   = Vec4f(1.f, 0.f, 0.f, 1.f);
constexpr Vec4f GREEN = Vec4f(0.f, 1.f, 0.f, 1.f);
constexpr Vec4f BLUE  = Vec4f(0.f, 0.f, 1.f, 1.f);
