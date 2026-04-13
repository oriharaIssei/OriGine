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
    constexpr Vector3(valueType xValue, valueType yValue, valueType zValue)
        : Vector<3, valueType>({xValue, yValue, zValue}) {}
    constexpr Vector3(int x, int y, int z)
        : Vector<3, valueType>({(valueType)x, (valueType)y, (valueType)z}) {}
    constexpr Vector3(const Vector2<valueType>& xy, const valueType& z)
        : Vector<3, valueType>({xy[X], xy[Y], z}) {}
    constexpr Vector3(const valueType& x, const Vector2<valueType>& yz)
        : Vector<3, valueType>({x, yz[X], yz[Y]}) {}
    constexpr Vector3(const valueType* x, const valueType* y, const valueType* z)
        : Vector<3, valueType>({*x, *y, *z}) {}
    constexpr Vector3(const valueType* ptr)
        : Vector<3, valueType>({ptr[0], ptr[1], ptr[2]}) {}

    /// <summary>
    /// 外積を計算
    /// </summary>
    /// <param name="another">対象ベクトル</param>
    /// <returns>外積結果ベクトル</returns>
    constexpr Vector3 cross(const Vector3& another) const {
        return Vector3(
            v[Y] * another.v[Z] - v[Z] * another.v[Y],
            v[Z] * another.v[X] - v[X] * another.v[Z],
            v[X] * another.v[Y] - v[Y] * another.v[X]);
    }
    /// <summary>
    /// 外積を計算 (static)
    /// </summary>
    /// <param name="v">ベクトル1</param>
    /// <param name="another">ベクトル2</param>
    /// <returns>外積結果ベクトル</returns>
    static constexpr Vector3 Cross(const Vector3& v, const Vector3& another) {
        return v.cross(another);
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
inline Vector3<valueType> Reflect(const Vector3<valueType>& v, const Vector3<valueType>& normal, float restitution = 1.f) {
    float vn = v.dot(normal);

    if (vn >= 0.0f) {
        return v;
    }

    return v - (1.0f + restitution) * vn * normal;
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
