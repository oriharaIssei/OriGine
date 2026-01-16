#pragma once

/// math
#include "Vector.h"
#include "Vector2.h"
#include "Vector3.h"
#include <cmath>

namespace OriGine {

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
    constexpr Vector4(valueType _xValue, valueType _yValue, valueType _zValue, valueType _wValue)
        : Vector<4, valueType>({_xValue, _yValue, _zValue, _wValue}) {}
    constexpr Vector4(const Vector2<valueType>& _xy, const Vector2<valueType>& _zw)
        : Vector<4, valueType>({_xy[X], _xy[Y], _zw[X], _zw[Y]}) {}
    constexpr Vector4(valueType _x, const Vector2<valueType>& _yz, valueType _w)
        : Vector<4, valueType>({_x, _yz[X], _yz[Y], _w}) {}
    constexpr Vector4(const Vector3<valueType>& _xyz, valueType _w)
        : Vector<4, valueType>({_xyz[X], _xyz[Y], _xyz[Z], _w}) {}
    constexpr Vector4(const Vector2<valueType>& _xy, valueType _z, valueType _w)
        : Vector<4, valueType>({_xy[X], _xy[Y], _z, _w}) {}
    constexpr Vector4(valueType _x, valueType _y, const Vector2<valueType>& _zw)
        : Vector<4, valueType>({_x, _y, _zw[X], _zw[Y]}) {}
    constexpr Vector4(valueType _x, const Vector3<valueType>& _yzw)
        : Vector<4, valueType>({_x, _yzw[X], _yzw[Y], _yzw[Z]}) {}
    constexpr Vector4(const valueType* _xPtr, const valueType* _yPtr, const valueType* _zPtr, const valueType* _wPtr)
        : Vector<4, valueType>({*_xPtr, *_yPtr, *_zPtr, *_wPtr}) {}
    constexpr Vector4(const valueType* _ptr)
        : Vector<4, valueType>({_ptr[0], _ptr[1], _ptr[2], _ptr[3]}) {}

    /// <summary>
    /// ベクトルの長さを計算
    /// </summary>
    constexpr valueType length() const { return std::sqrt(v[X] * v[X] + v[Y] * v[Y] + v[Z] * v[Z] + v[W] * v[W]); }
    /// <summary>
    /// ベクトルの長さを計算 (static)
    /// </summary>
    /// <param name="_v">ベクトル</param>
    /// <returns>ベクトルの長さ</returns>
    static constexpr valueType Length(const Vector4& _v) { return std::sqrt(_v.v[X] * _v.v[X] + _v.v[Y] * _v.v[Y] + _v.v[Z] * _v.v[Z] + _v.v[W] * _v.v[W]); }

    /// <summary>
    /// ベクトルの長さの二乗を計算
    /// </summary>
    /// <returns>長さの二乗</returns>
    constexpr valueType lengthSq() const { return (v[X] * v[X] + v[Y] * v[Y] + v[Z] * v[Z] + v[W] * v[W]); }
    /// <summary>
    /// ベクトルの長さの二乗を計算 (static)
    /// </summary>
    /// <param name="_v">ベクトル</param>
    /// <returns>長さの二乗</returns>
    static constexpr valueType LengthSq(const Vector4& _v) { return (_v.v[X] * _v.v[X] + _v.v[Y] * _v.v[Y] + _v.v[Z] * _v.v[Z] + _v.v[W] * _v.v[W]); }

    /// <summary>
    /// 内積を計算
    /// </summary>
    /// <param name="_another">対象ベクトル</param>
    /// <returns>内積</returns>
    constexpr valueType dot(const Vector4& _another) const { return (v[X] * _another.v[X]) + (v[Y] * _another.v[Y]) + (v[Z] * _another.v[Z]) + (v[W] * _another.v[W]); }
    /// <summary>
    /// 内積を計算 (static)
    /// </summary>
    /// <param name="_v">ベクトル1</param>
    /// <param name="_another">ベクトル2</param>
    /// <returns>内積</returns>
    static constexpr valueType Dot(const Vector4& _v, const Vector4& _another) { return (_v.v[X] * _another.v[X]) + (_v.v[Y] * _another.v[Y]) + (_v.v[Z] * _another.v[Z]) + (_v.v[W] * _another.v[W]); }

    /// <summary>
    /// 正規化
    /// </summary>
    /// <returns>正規化後ベクトル</returns>
    constexpr Vector4 normalize() const {
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
constexpr Vec4f kWhite = Vec4f(1.f, 1.f, 1.f, 1.f);
constexpr Vec4f kBlack = Vec4f(0.f, 0.f, 0.f, 1.f);
constexpr Vec4f kRed   = Vec4f(1.f, 0.f, 0.f, 1.f);
constexpr Vec4f kGreen = Vec4f(0.f, 1.f, 0.f, 1.f);
constexpr Vec4f kBlue  = Vec4f(0.f, 0.f, 1.f, 1.f);

} // namespace OriGine
