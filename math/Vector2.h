#pragma once

/// math
#include "Vector.h"
#include <cmath>

/// externals
#include <imgui/imgui.h>

namespace OriGine {

/// <summary>
/// 2次元ベクトル
/// </summary>
template <typename valueType = float>
struct Vector2 final
    : Vector<2, valueType> {
    using Vector<2, valueType>::dim;
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
    using Vector<2, valueType>::length;
    using Vector<2, valueType>::Length;
    using Vector<2, valueType>::lengthSq;
    using Vector<2, valueType>::LengthSq;
    using Vector<2, valueType>::dot;
    using Vector<2, valueType>::Dot;
    using Vector<2, valueType>::normalize;
    using Vector<2, valueType>::Normalize;

    // コンストラクタ
    constexpr Vector2() : Vector<2, valueType>({valueType(0), valueType(0)}) {}
    constexpr Vector2(valueType xValue, valueType yValue)
        : Vector<2, valueType>({xValue, yValue}) {}
    constexpr Vector2(const valueType* xPtr, const valueType* yPtr)
        : Vector<2, valueType>({*xPtr, *yPtr}) {}
    constexpr Vector2(const valueType* ptr)
        : Vector<2, valueType>({ptr[0], ptr[1]}) {}

    /// <summary>
    /// 外積を計算 (2Dの外積はスカラー)
    /// </summary>
    constexpr valueType cross(const Vector2& another) const { return (this->v[X] * another.v[Y]) - (this->v[Y] * another.v[X]); }
    /// <summary>
    /// 外積を計算 (static)
    /// </summary>
    /// <param name="v">ベクトル1</param>
    /// <param name="another">ベクトル2</param>
    /// <returns>外積(行列式)</returns>
    static constexpr valueType Cross(const Vector2& v, const Vector2& another) { return (v.v[X] * another.v[Y]) - (v.v[Y] * another.v[X]); }

#ifdef _DEBUG
    constexpr Vector2& operator=(const ImVec2& another) {
        this->v[X] = another.x;
        this->v[Y] = another.y;
        return *this;
    }
    /// <summary>
    /// ImVec2へ変換
    /// </summary>
    /// <returns>ImVec2</returns>
    ImVec2 toImVec2() const {
        ImVec2 result;
        result.x = this->v[X];
        result.y = this->v[Y];
        return result;
    }
#endif // DEBUG
};

template <int dim, typename valueType>
inline Vector2<valueType> operator*(const valueType& scalar, const Vector2<valueType>& vec) {
    Vector2<valueType> result;
    for (int i = 0; i < dim; i++) {
        result[i] = vec[i] * scalar;
    }
    return result;
}

template <int dim, typename valueType>
inline Vector2<valueType> operator*(const Vector2<valueType>& vec, const Vector2<valueType>& another) {
    Vector2<valueType> result;
    for (int i = 0; i < dim; i++) {
        result[i] = vec[i] * another[i];
    }
    return result;
}
template <int dim, typename valueType>
inline Vector2<valueType>* operator*=(Vector2<valueType>& vec, const Vector2<valueType>& another) {
    for (int i = 0; i < dim; i++) {
        vec[i] *= another[i];
    }
    return &vec;
}

template <int dim, typename valueType>
inline Vector2<valueType> operator/(const Vector2<valueType>& vec, const Vector2<valueType>& another) {
    Vector2<valueType> result;
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
inline Vector2<valueType>* operator/=(Vector2<valueType>& vec, const Vector2<valueType>& another) {
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
/// <typeparam name="valueType"></typeparam>
/// <param name="v"></param>
/// <param name="normal"></param>
/// <returns></returns>
template <typename valueType>
inline Vector2<valueType> Reflect(const Vector2<valueType>& v, const Vector2<valueType>& normal) {
    return v - 2.0f * (v.dot(normal)) * normal;
}

//=========== using ===========//
template <typename valueType>
using Vec2 = Vector2<valueType>;

using Vector2f = Vector2<float>;
using Vec2f    = Vector2<float>;

using Vector2d = Vector2<double>;
using Vec2d    = Vector2<double>;

} // namespace OriGine
