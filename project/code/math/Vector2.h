#pragma once

#include <cmath>
#include <Vector.h>

//=====================================
// x&yをもつ単位
//=====================================

template <typename valueType = float>
struct Vector2 final
    : Vector<2, valueType> {

    using Vector<2, valueType>::v;

    // アクセサメソッド
    valueType& x() { return this->v[0]; }
    const valueType& x() const { return this->v[0]; }
    valueType& y() { return this->v[1]; }
    const valueType& y() const { return this->v[1]; }

    // コンストラクタ
    Vector2(valueType xValue, valueType yValue)
        : Vector<2, valueType>() {
        this->v[0] = xValue;
        this->v[1] = yValue;
    }
    Vector2(int X, int Y)
        : Vector<2, valueType>() {
        this->v[0] = static_cast<valueType>(X);
        this->v[1] = static_cast<valueType>(Y);
    }
    Vector2(const valueType* x_ptr, const valueType* y_ptr)
        : Vector<2, valueType>() {
        this->v[0] = *x_ptr;
        this->v[1] = *y_ptr;
    }
    Vector2(const valueType* ptr)
        : Vector<2, valueType>() {
        this->v[0] = ptr[0];
        this->v[1] = ptr[1];
    }
    Vector2()
        : Vector<2, valueType>() {
        this->v[0] = 0.0f;
        this->v[1] = 0.0f;
    }

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

    static Vector2 Normalize(const Vector2& v) {
        valueType length = v.length();
        if (length == 0) {
            return v;
        }
        Vector2 result = v;
        return (result / length);
    }

    Vector2 operator+(const Vector2& other) const {
        return Vector2(this->v[0] + other.v[0], this->v[1] + other.v[1]);
    }
    void operator+=(const Vector2& other) {
        this->v[0] += other.v[0];
        this->v[1] += other.v[1];
    }

    Vector2 operator-(const Vector2& other) const {
        return Vector2(this->v[0] - other.v[0], this->v[1] - other.v[1]);
    }
    Vector2 operator-() const { return Vector2(-this->v[0], -this->v[1]); }
    void operator-=(const Vector2& other) {
        this->v[0] -= other.v[0];
        this->v[1] -= other.v[1];
    }

    Vector2 operator*(const valueType& scalar) const {
        return Vector2(this->v[0] * scalar, this->v[1] * scalar);
    }
    void operator*=(const valueType& scalar) {
        this->v[0] *= scalar;
        this->v[1] *= scalar;
    }

    Vector2 operator/(const valueType& scalar) const {
        if (scalar != 0) {
            return Vector2(this->v[0] / scalar, this->v[1] / scalar);
        } else {
            // ゼロで割る場合の処理を追加
            return Vector2(0.0f, 0.0f);
        }
    }
    void operator/=(const valueType& scalar) {
        if (scalar != 0) {
            this->v[0] /= scalar;
            this->v[1] /= scalar;
        } else {
            // ゼロで割る場合の処理を追加
            this->v[0] = 0;
            this->v[1] = 0;
        }
    }

    // 等号演算子のオーバーロード
    bool operator==(const Vector2& other) const {
        return (this->v[0] == other.v[0] && this->v[1] == other.v[1]);
    }

    // 不等号演算子のオーバーロード
    bool operator!=(const Vector2& other) const {
        return !(*this == other);
    }
};

template <typename valueType = float>
using Vec2 = Vector2<valueType>;
// float
using Vector2f = Vector2<float>;
using Vec2f    = Vector2<float>;

template <typename valueType>
inline Vector2<valueType> operator*(const valueType& scalar, const Vector2<valueType>& vec) {
    return Vector2<valueType>(vec.v[0] * scalar, vec.v[1] * scalar);
}

template <typename valueType>
inline Vector2<valueType> Lerp(const Vector2<valueType>& start, const Vector2<valueType>& end, float time) {
    return {
        std::lerp(start.v[0], end.v[0], time),
        std::lerp(start.v[1], end.v[1], time)};
}

template <typename valueType>
inline Vector2<valueType> Lerp(const Vector2<valueType>& start, const Vector2<valueType>& end, double time) {
    return {
        std::lerp<valueType>(start.v[0], end.v[0], time),
        std::lerp<valueType>(start.v[1], end.v[1], time)};
}
