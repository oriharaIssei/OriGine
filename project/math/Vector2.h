#pragma once

/// math
#include "Vector.h"
#include <cmath>

/// externals
#include <imgui/imgui.h>

namespace OriGine {

//=====================================
// 2次元(x&y)をもつ単位
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
    constexpr Vector2() : Vector<2, valueType>({0, 0}) {}
    constexpr Vector2(valueType xValue, valueType yValue)
        : Vector<2, valueType>({xValue, yValue}) {}
    constexpr Vector2(const valueType* x_ptr, const valueType* y_ptr)
        : Vector<2, valueType>({*x_ptr, *y_ptr}) {}
    constexpr Vector2(const valueType* ptr)
        : Vector<2, valueType>({ptr[0], ptr[1]}) {}

    /// <summary>
    /// ベクトルの長さを計算
    /// </summary>
    /// <returns>ベクトルの長さ</returns>
    constexpr valueType length() const { return std::sqrt(v[X] * v[X] + v[Y] * v[Y]); }
    static constexpr valueType Length(const Vector2& v) { return std::sqrt(v.v[X] * v.v[X] + v.v[Y] * v.v[Y]); }

    /// <summary>
    /// ベクトルの長さの2乗を計算
    /// </summary>
    constexpr valueType lengthSq() const { return (this->v[X] * this->v[X] + this->v[Y] * this->v[Y]); }
    static constexpr valueType LengthSq(const Vector2& v1) { return (v1.v[X] * v1.v[X] + v1.v[Y] * v1.v[Y]); }

    /// <summary>
    /// 内積を計算
    /// </summary>
    constexpr valueType dot() const { return v[X] * v[X] + v[Y] * v[Y]; }
    static constexpr valueType Dot(const Vector2& _v) { return _v.v[X] * _v.v[X] + _v.v[Y] * _v.v[Y]; }

    /// <summary>
    /// 外積を計算
    /// </summary>
    constexpr valueType cross(const Vector2& another) const { return (this->v[X] * another.v[Y]) - (this->v[Y] * another.v[X]); }
    static constexpr valueType Cross(const Vector2& _v, const Vector2& another) { return (_v.v[X] * another.v[Y]) - (_v.v[Y] * another.v[X]); }

    /// <summary>
    /// 正規化
    /// </summary>
    /// <returns>正規化済みベクトル</returns>
    constexpr Vector2 normalize() const {
        valueType length = this->length();
        if (length == 0) {
            return *this;
        }
        Vector2 result = *this;
        return (result / length);
    }
    /// <summary>
    /// 正規化(static)
    /// </summary>
    /// <param name="_v">正規化 前</param>
    /// <returns>正規化 後</returns>
    static constexpr Vector2 Normalize(const Vector2& _v) {
        valueType length = _v.length();
        if (length == 0) {
            return _v;
        }
        Vector2 result = _v;
        return (result / length);
    }

#ifdef _DEBUG
    constexpr Vector2& operator=(const ImVec2& another) {
        this->v[X] = another.x;
        this->v[Y] = another.y;
        return *this;
    }
    ImVec2 toImVec2() const {
        ImVec2 result;
        result.x = this->v[X];
        result.y = this->v[Y];
        return result;
    }
#endif // DEBUG
};

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
