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
    using Vector<4, valueType>::dim;
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
    using Vector<4, valueType>::length;
    using Vector<4, valueType>::Length;
    using Vector<4, valueType>::lengthSq;
    using Vector<4, valueType>::LengthSq;
    using Vector<4, valueType>::dot;
    using Vector<4, valueType>::Dot;
    using Vector<4, valueType>::normalize;
    using Vector<4, valueType>::Normalize;

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

};

template <int dim, typename valueType>
inline Vector4<valueType> operator*(const valueType& scalar, const Vector4<valueType>& vec) {
    Vector4<valueType> result;
    for (int i = 0; i < dim; i++) {
        result[i] = vec[i] * scalar;
    }
    return result;
}

template <int dim, typename valueType>
inline Vector4<valueType> operator*(const Vector4<valueType>& vec, const Vector4<valueType>& another) {
    Vector4<valueType> result;
    for (int i = 0; i < dim; i++) {
        result[i] = vec[i] * another[i];
    }
    return result;
}
template <int dim, typename valueType>
inline Vector4<valueType>* operator*=(Vector4<valueType>& vec, const Vector4<valueType>& another) {
    for (int i = 0; i < dim; i++) {
        vec[i] *= another[i];
    }
    return &vec;
}

template <int dim, typename valueType>
inline Vector4<valueType> operator/(const Vector4<valueType>& vec, const Vector4<valueType>& another) {
    Vector4<valueType> result;
    for (int i = 0; i < dim; i++) {
        if (another[i] != 0) {
            result[i] = vec[i] / another[i];
        } else {
            result[i] = 0;
        }
    }
    return result;
}
template <int dim, typename valueType>
inline Vector4<valueType>* operator/=(Vector4<valueType>& vec, const Vector4<valueType>& another) {
    for (int i = 0; i < dim; i++) {
        if (another[i] != 0) {
            vec[i] /= another[i];
        } else {
            vec[i] = 0;
        }
    }
    return &vec;
}

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
