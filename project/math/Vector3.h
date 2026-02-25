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
struct Vector3 final
    : Vector<3, valueType> {
    using Vector<3, valueType>::dim;
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
    using Vector<3, valueType>::length;
    using Vector<3, valueType>::Length;
    using Vector<3, valueType>::lengthSq;
    using Vector<3, valueType>::LengthSq;
    using Vector<3, valueType>::dot;
    using Vector<3, valueType>::Dot;
    using Vector<3, valueType>::normalize;
    using Vector<3, valueType>::Normalize;

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
};


template <int dim, typename valueType>
inline Vector3<valueType> operator*(const valueType& scalar, const Vector3<valueType>& vec) {
    Vector3<valueType> result;
    for (int i = 0; i < dim; i++) {
        result[i] = vec[i] * scalar;
    }
    return result;
}

template <int dim, typename valueType>
inline Vector3<valueType> operator*(const Vector3<valueType>& vec, const Vector3<valueType>& another) {
    Vector3<valueType> result;
    for (int i = 0; i < dim; i++) {
        result[i] = vec[i] * another[i];
    }
    return result;
}
template <int dim, typename valueType>
inline Vector3<valueType>* operator*=(Vector3<valueType>& vec, const Vector3<valueType>& another) {
    for (int i = 0; i < dim; i++) {
        vec[i] *= another[i];
    }
    return &vec;
}

template <int dim, typename valueType>
inline Vector3<valueType> operator/(const Vector3<valueType>& vec, const Vector3<valueType>& another) {
    Vector3<valueType> result;
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
inline Vector3<valueType>* operator/=(Vector3<valueType>& vec, const Vector3<valueType>& another) {
    for (int i = 0; i < dim; i++) {
        if (another[i] != 0) {
            vec[i] /= another[i];
        } else {
            vec[i] = 0;
        }
    }
    return &vec;
}

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

constexpr Vec3f kZeroVec3f = {0.f, 0.f, 0.f};
constexpr Vec3f kOneVec3f  = {1.f, 1.f, 1.f};

} // namespace OriGine
