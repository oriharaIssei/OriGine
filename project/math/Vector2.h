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
    constexpr Vector2() : Vector<2, valueType>({valueType(0), valueType(0)}) {}
    constexpr Vector2(valueType _xValue, valueType _yValue)
        : Vector<2, valueType>({_xValue, _yValue}) {}
    constexpr Vector2(const valueType* _xPtr, const valueType* _yPtr)
        : Vector<2, valueType>({*_xPtr, *_yPtr}) {}
    constexpr Vector2(const valueType* _ptr)
        : Vector<2, valueType>({_ptr[0], _ptr[1]}) {}

    /// <summary>
    /// ベクトルの長さを計算
    /// </summary>
    /// <returns>ベクトルの長さ</returns>
    constexpr valueType length() const { return std::sqrt(v[X] * v[X] + v[Y] * v[Y]); }
    /// <summary>
    /// ベクトルの長さを計算 (static)
    /// </summary>
    /// <param name="_v">ベクトル</param>
    /// <returns>ベクトルの長さ</returns>
    static constexpr valueType Length(const Vector2& _v) { return std::sqrt(_v.v[X] * _v.v[X] + _v.v[Y] * _v.v[Y]); }

    /// <summary>
    /// ベクトルの長さの2乗を計算
    /// </summary>
    constexpr valueType lengthSq() const { return (this->v[X] * this->v[X] + this->v[Y] * this->v[Y]); }
    /// <summary>
    /// ベクトルの長さの2乗を計算 (static)
    /// </summary>
    /// <param name="_v1">ベクトル</param>
    /// <returns>長さの2乗</returns>
    static constexpr valueType LengthSq(const Vector2& _v1) { return (_v1.v[X] * _v1.v[X] + _v1.v[Y] * _v1.v[Y]); }

    /// <summary>
    /// 内積を計算
    /// </summary>
    constexpr valueType dot() const { return v[X] * v[X] + v[Y] * v[Y]; }
    /// <summary>
    /// 内積を計算 (static)
    /// </summary>
    /// <param name="_v">ベクトル</param>
    /// <returns>内積</returns>
    static constexpr valueType Dot(const Vector2& _v) { return _v.v[X] * _v.v[X] + _v.v[Y] * _v.v[Y]; }

    /// <summary>
    /// 外積を計算 (2Dの外積はスカラー)
    /// </summary>
    constexpr valueType cross(const Vector2& _another) const { return (this->v[X] * _another.v[Y]) - (this->v[Y] * _another.v[X]); }
    /// <summary>
    /// 外積を計算 (static)
    /// </summary>
    /// <param name="_v">ベクトル1</param>
    /// <param name="_another">ベクトル2</param>
    /// <returns>外積(行列式)</returns>
    static constexpr valueType Cross(const Vector2& _v, const Vector2& _another) { return (_v.v[X] * _another.v[Y]) - (_v.v[Y] * _another.v[X]); }

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
    constexpr Vector2& operator=(const ImVec2& _another) {
        this->v[X] = _another.x;
        this->v[Y] = _another.y;
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

/// <summary>
/// 反射ベクトルを計算
/// </summary>
/// <typeparam name="valueType"></typeparam>
/// <param name="_v"></param>
/// <param name="_normal"></param>
/// <returns></returns>
template <typename valueType>
inline Vector2<valueType> Reflect(const Vector2<valueType>& _v, const Vector2<valueType>& _normal) {
    return _v - 2.0f * (_v.dot(_normal)) * _normal;
}

//=========== using ===========//
template <typename valueType>
using Vec2 = Vector2<valueType>;

using Vector2f = Vector2<float>;
using Vec2f    = Vector2<float>;

using Vector2d = Vector2<double>;
using Vec2d    = Vector2<double>;

} // namespace OriGine
