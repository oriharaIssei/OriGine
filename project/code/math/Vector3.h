#pragma once

#include "Vector2.h"

#include <cmath>

struct Quaternion;
template <typename valueType = float>
struct Vector3 final
    : Vector<3, valueType> {
    using Vector<3, valueType>::v;

    // アクセサメソッド
    valueType& x() { return this->v[0]; }
    const valueType& x() const { return this->v[0]; }
    valueType& y() { return this->v[1]; }
    const valueType& y() const { return this->v[1]; }
    valueType& z() { return this->v[2]; }
    const valueType& z() const { return this->v[2]; }

    // コンストラクタ
    Vector3(valueType xValue, valueType yValue, valueType zValue)
        : Vector<3, valueType>({xValue, yValue, zValue}) {}
    Vector3(int X, int Y, int Z)
        : Vector<3, valueType>({(valueType)X, (valueType)Y, (valueType)Z}) {}
    Vector3(const Vector2<valueType>& xy, const valueType& z)
        : Vector<3, valueType>({xy.x(), xy.y(), z}) {}
    Vector3(const valueType& x, const Vector2<valueType>& yz)
        : Vector<3, valueType>({x, yz.x(), yz.y()}) {}
    Vector3(const valueType* x_ptr, const valueType* y_ptr, const valueType* z_ptr)
        : Vector<3, valueType>({*x_ptr, *y_ptr, *z_ptr}) {}
    Vector3(const valueType* ptr)
        : Vector<3, valueType>({ptr[0], ptr[1], ptr[2]}) {}
    Vector3()
        : Vector<3, valueType>({valueType(0), valueType(0), valueType(0)}) {}

    // ベクトルの長さ
    template <typename resultType = valueType>
    resultType length() const { return static_cast<resultType>(sqrt(static_cast<resultType>(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]))); }
    template <typename resultType = valueType>
    static resultType Length(const Vector3& v) { return static_cast<resultType>(sqrt(static_cast<resultType>(v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2]))); }

    // ベクトルの長さの二乗
    valueType lengthSq() const { return (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]); }
    static valueType LengthSq(const Vector3& v) { return (v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2]); }

    // 内積
    valueType dot() const {
        return (v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]);
    }
    static valueType Dot(const Vector3& v) {
        return (v.v[0] * v.v[0]) + (v.v[1] * v.v[1]) + (v.v[2] * v.v[2]);
    }
    valueType dot(const Vector3& another) const {
        return (v[0] * another.v[0]) + (v[1] * another.v[1]) + (v[2] * another.v[2]);
    }
    static valueType Dot(const Vector3& v, const Vector3& another) {
        return (v.v[0] * another.v[0]) + (v.v[1] * another.v[1]) + (v.v[2] * another.v[2]);
    }

    // 外積
    Vector3 cross(const Vector3& another) const {
        return Vector3(
            v[1] * another.v[2] - v[2] * another.v[1],
            v[2] * another.v[0] - v[0] * another.v[2],
            v[0] * another.v[1] - v[1] * another.v[0]);
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

    // 演算子オーバーロード（+ - * / 等）
    Vector3 operator+(const Vector3& other) const {
        return Vector3(v[0] + other.v[0], v[1] + other.v[1], v[2] + other.v[2]);
    }
    void operator+=(const Vector3& other) {
        v[0] += other.v[0];
        v[1] += other.v[1];
        v[2] += other.v[2];
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3(v[0] - other.v[0], v[1] - other.v[1], v[2] - other.v[2]);
    }
    Vector3 operator-() const { return Vector3(-v[0], -v[1], -v[2]); }
    void operator-=(const Vector3& other) {
        v[0] -= other.v[0];
        v[1] -= other.v[1];
        v[2] -= other.v[2];
    }

    Vector3 operator*(const valueType& scalar) const {
        return Vector3(v[0] * scalar, v[1] * scalar, v[2] * scalar);
    }
    void operator*=(const valueType& scalar) {
        v[0] *= scalar;
        v[1] *= scalar;
        v[2] *= scalar;
    }

    Vector3 operator/(const valueType& scalar) const {
        if (scalar != 0) {
            return Vector3(v[0] / scalar, v[1] / scalar, v[2] / scalar);
        } else {
            return Vector3(0.0f, 0.0f, 0.0f);
        }
    }
    void operator/=(const valueType& scalar) {
        if (scalar != 0) {
            v[0] /= scalar;
            v[1] /= scalar;
            v[2] /= scalar;
        } else {
            v[0] = 0;
            v[1] = 0;
            v[2] = 0;
        }
    }

    bool operator==(const Vector3& other) const {
        return (v[0] == other.v[0] && v[1] == other.v[1] && v[2] == other.v[2]);
    }
    bool operator!=(const Vector3& other) const {
        return !(*this == other);
    }
};

template <typename valueType = float>
using Vec3 = Vector3<valueType>;

using Vector3f = Vector3<float>;
using Vec3f    = Vector3<float>;

template <typename valueType>
inline Vector3<valueType> operator*(const valueType& scalar, const Vector3<valueType>& vec) {
    return Vector3<valueType>(vec.v[0] * scalar, vec.v[1] * scalar, vec.v[2] * scalar);
}

template <typename valueType>
inline Vector3<valueType> Lerp(const Vector3<valueType>& start, const Vector3<valueType>& end, float time) {
    return {
        std::lerp(start.v[0], end.v[0], time),
        std::lerp(start.v[1], end.v[1], time),
        std::lerp(start.v[2], end.v[2], time)};
}

template <typename valueType>
inline Vector3<valueType> Lerp(const Vector3<valueType>& start, const Vector3<valueType>& end, double time) {
    return {
        std::lerp<valueType>(start.v[0], end.v[0], time),
        std::lerp<valueType>(start.v[1], end.v[1], time),
        std::lerp<valueType>(start.v[2], end.v[2], time)};
}
