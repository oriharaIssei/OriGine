#pragma once

#include "Vector.h"

#include <cmath>
#include <Vector.h>

//=====================================
// x&yをもつ単位
//=====================================

template <typename valueType = float>
struct Vector2 final
    : Vector<2, valueType> {
    using Vector<2, valueType>::v;
    using Vector<2, valueType>::operator[];
    using Vector<2, valueType>::operator+;
    using Vector<2, valueType>::operator+=;
    using Vector<2, valueType>::operator-;
    using Vector<2, valueType>::operator-=;
    using Vector<2, valueType>::operator*;
    using Vector<2, valueType>::operator*=;
    using Vector<2, valueType>::operator/;
    using Vector<2, valueType>::operator/=;
    using Vector<2, valueType>::operator=;
    using Vector<2, valueType>::operator==;
    using Vector<2, valueType>::operator!=;

    // コンストラクタ
    Vector2(valueType xValue, valueType yValue)
        : Vector<2, valueType>({xValue, yValue}) {}
    Vector2(const valueType* x_ptr, const valueType* y_ptr)
        : Vector<2, valueType>({*x_ptr, *y_ptr}) {}
    Vector2(const valueType* ptr)
        : Vector<2, valueType>({ptr[0], ptr[1]}) {}
    Vector2()
        : Vector<2, valueType>({0, 0}) {}

    // ベクトルの長さを計算
    template <typename resultType = valueType>
    resultType length() const { return static_cast<resultType>(sqrt(static_cast<resultType>(this->v[0] * this->v[0] + this->v[1] * this->v[1]))); }
    template <typename resultType = valueType>
    static resultType Length(const Vector2& v) { return static_cast<resultType>(sqrt(static_cast<resultType>(v.v[0] * v.v[0] + v.v[1] * v.v[1]))); }

    template <typename resultType = valueType>
    resultType lengthSq() const { return (this->v[0] * this->v[0] + this->v[1] * this->v[1]); }
    template <typename resultType = valueType>
    static resultType LengthSq(const Vector2& v1) { return (v1.v[0] * v1.v[0] + v1.v[1] * v1.v[1]); }

    template <typename resultType = valueType>
    resultType dot() const { return this->v[0] * this->v[0] + this->v[1] * this->v[1]; }
    template <typename resultType = valueType>
    resultType dot(const Vector2& another) const { return (this->v[0] * another.v[0]) + (this->v[1] * another.v[1]); }

    template <typename resultType = valueType>
    static resultType Dot(const Vector2& v) { return v.v[0] * v.v[0] + v.v[1] * v.v[1]; }
    template <typename resultType = valueType>
    static resultType Dot(const Vector2& v, const Vector2& another) { return (v.v[0] * another.v[0]) + (v.v[1] * another.v[1]); }

    template <typename resultType = valueType>
    resultType cross(const Vector2& another) const { return (this->v[0] * another.v[1]) - (this->v[1] * another.v[0]); }
    template <typename resultType = valueType>
    static resultType Cross(const Vector2& v, const Vector2& another) { return (v.v[0] * another.v[1]) - (v.v[1] * another.v[0]); }

    Vector2 normalize() const {
        valueType length = this->length();
        if (length == 0) {
            return *this;
        }
        Vector2 result = *this;
        return (result / length);
    }

    static Vector2 Normalize(const Vector2& _v) {
        valueType length = _v.length();
        if (length == 0) {
            return _v;
        }
        Vector2 result = _v;
        return (result / length);
    }
};

//=========== using ===========//
template <typename valueType>
using Vec2 = Vector2<valueType>;

using Vector2f = Vector2<float>;
using Vec2f    = Vector2<float>;

using Vector2d = Vector2<double>;
using Vec2d    = Vector2<double>;
