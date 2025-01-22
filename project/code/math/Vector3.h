#pragma once

#include "Vector.h"
#include "Vector2.h"

#include <cmath>

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
    Vector3(valueType xValue, valueType yValue, valueType zValue)
        : Vector<3, valueType>({xValue, yValue, zValue}) {}
    Vector3(int X, int Y, int Z)
        : Vector<3, valueType>({(valueType)X, (valueType)Y, (valueType)Z}) {}
    Vector3(const Vector2<valueType>& xy, const valueType& z)
        : Vector<3, valueType>({xy[X], xy[Y], z}) {}
    Vector3(const valueType& x, const Vector2<valueType>& yz)
        : Vector<3, valueType>({x, yz[X], yz[Y]}) {}
    Vector3(const valueType* x_ptr, const valueType* y_ptr, const valueType* z_ptr)
        : Vector<3, valueType>({*x_ptr, *y_ptr, *z_ptr}) {}
    Vector3(const valueType* ptr)
        : Vector<3, valueType>({ptr[0], ptr[1], ptr[2]}) {}
    Vector3()
        : Vector<3, valueType>({0, 0, 0}) {}

    // ベクトルの長さ
    valueType length() const { return std::sqrt(v[X] * v[X] + v[Y] * v[Y] + v[Z] * v[Z]); }
    static valueType Length(const Vector3& v) { return std::sqrt(v.v[X] * v.v[X] + v.v[Y] * v.v[Y] + v.v[Z] * v.v[Z]); }

    // ベクトルの長さの二乗
    valueType lengthSq() const { return (v[X] * v[X] + v[Y] * v[Y] + v[Z] * v[Z]); }
    static valueType LengthSq(const Vector3& v) { return (v.v[X] * v.v[X] + v.v[Y] * v.v[Y] + v.v[Z] * v.v[Z]); }

    // 内積
    valueType dot() const { return v[X] * v[X] + v[Y] * v[Y] + v[Z] * v[Z]; }
    static valueType Dot(const Vector3& v) { return v.v[X] * v.v[X] + v.v[Y] * v.v[Y] + v.v[Z] * v.v[Z]; }
    // 外積
    Vector3 cross(const Vector3& another) const {
        return Vector3(
            v[Y] * another.v[Z] - v[Z] * another.v[Y],
            v[Z] * another.v[X] - v[X] * another.v[Z],
            v[X] * another.v[Y] - v[Y] * another.v[X]);
    }
    static Vector3 Cross(const Vector3& v, const Vector3& another) {
        return v.cross(another);
    }

    // 正規化
    Vector3 normalize() const {
        valueType len = length();
        if (len == 0) return *this;
        return (*this / len);
    }
    static Vector3 Normalize(const Vector3& v) {
        valueType len = v.length();
        if (len == 0) return v;
        return (v / len);
    }
};

//=========== using ===========//
template <typename valueType>
using Vec3 = Vector3<valueType>;

using Vector3f = Vector3<float>;
using Vec3f    = Vector3<float>;

using Vector3d = Vector3<double>;
using Vec3d    = Vector3<double>;

//=========== DirectionVector ===========//
static const Vec3f axisX = Vec3f(1.0f, 0.0f, 0.0f);
static const Vec3f axisY = Vec3f(0.0f, 1.0f, 0.0f);
static const Vec3f axisZ = Vec3f(0.0f, 0.0f, 1.0f);
