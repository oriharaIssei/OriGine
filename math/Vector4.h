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
    constexpr Vector4(const valueType* xPtr, const valueType* yPtr, const valueType* zPtr, const valueType* wPtr)
        : Vector<4, valueType>({*xPtr, *yPtr, *zPtr, *wPtr}) {}
    constexpr Vector4(const valueType* ptr)
        : Vector<4, valueType>({ptr[0], ptr[1], ptr[2], ptr[3]}) {}

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
