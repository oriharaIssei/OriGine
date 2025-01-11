#pragma once

#include <Vector2.h>
#include <Vector3.h>
#include <cmath>

template <typename valueType = float>
struct Vector4 final
    : Vector<4, valueType> {
    using Vector<4, valueType>::v;

    // アクセサメソッド
    valueType& x() { return this->v[0]; }
    const valueType& x() const { return this->v[0]; }
    valueType& y() { return this->v[1]; }
    const valueType& y() const { return this->v[1]; }
    valueType& z() { return this->v[2]; }
    const valueType& z() const { return this->v[2]; }
    valueType& w() { return this->v[3]; }
    const valueType& w() const { return this->v[3]; }

    // コンストラクタ
    Vector4(valueType xValue, valueType yValue, valueType zValue, valueType wValue)
        : Vector<4, valueType>({xValue, yValue, zValue, wValue}) {}
    Vector4(int X, int Y, int Z, int W)
        : Vector<4, valueType>({(valueType)X, (valueType)Y, (valueType)Z, (valueType)W}) {}
    Vector4(const valueType* x_ptr, const valueType* y_ptr, const valueType* z_ptr, const valueType* w_ptr)
        : Vector<4, valueType>({*x_ptr, *y_ptr, *z_ptr, *w_ptr}) {}
    Vector4(const valueType* ptr)
        : Vector<4, valueType>({ptr[0], ptr[1], ptr[2], ptr[3]}) {}
    Vector4(const Vector2<valueType>& xy, const Vector2<valueType>& zw)
        : Vector<4, valueType>({xy.x(), xy.y(), zw.x(), zw.y()}) {}
    Vector4(valueType x, const Vector2<valueType>& yz, valueType w)
        : Vector<4, valueType>({x, yz.x(), yz.y(), w}) {}
    Vector4(const Vector3<valueType>& xyz, valueType w)
        : Vector<4, valueType>({xyz.x(), xyz.y(), xyz.z(), w}) {}
    Vector4(const Vector2<valueType>& xy, valueType z, valueType w)
        : Vector<4, valueType>({xy.x(), xy.y(), z, w}) {}
    Vector4(valueType x, valueType y, const Vector2<valueType>& zw)
        : Vector<4, valueType>({x, y, zw.x(), zw.y()}) {}
    Vector4(valueType x, const Vector3<valueType>& yzw)
        : Vector<4, valueType>({x, yzw.x(), yzw.y(), yzw.z()}) {}
    Vector4()
        : Vector<4, valueType>({valueType(0), valueType(0), valueType(0), valueType(0)}) {}

    // ベクトルの長さ
    template <typename resultType = valueType>
    resultType length() const { return static_cast<resultType>(sqrt(static_cast<resultType>(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]))); }
    template <typename resultType = valueType>
    static resultType Length(const Vector4& v) { return static_cast<resultType>(sqrt(static_cast<resultType>(v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2] + v.v[3] * v.v[3]))); }

    // ベクトルの長さの二乗
    valueType lengthSq() const { return (v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]); }
    static valueType LengthSq(const Vector4& v) { return (v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2] + v.v[3] * v.v[3]); }

    // 内積
    valueType dot(const Vector4& another) const {
        return (v[0] * another.v[0]) + (v[1] * another.v[1]) + (v[2] * another.v[2]) + (v[3] * another.v[3]);
    }
    static valueType Dot(const Vector4& v, const Vector4& another) {
        return (v.v[0] * another.v[0]) + (v.v[1] * another.v[1]) + (v.v[2] * another.v[2]) + (v.v[3] * another.v[3]);
    }

    // 正規化
    Vector4 normalize() const {
        valueType len = length();
        if (len == 0) return *this;
        return (*this / len);
    }
    static Vector4 Normalize(const Vector4& v) {
        valueType len = v.length();
        if (len == 0) return v;
        return (v / len);
    }

    // 演算子オーバーロード (+, -, *, /, 等)
    Vector4 operator+(const Vector4& other) const {
        return Vector4(v[0] + other.v[0], v[1] + other.v[1], v[2] + other.v[2], v[3] + other.v[3]);
    }
    void operator+=(const Vector4& other) {
        v[0] += other.v[0];
        v[1] += other.v[1];
        v[2] += other.v[2];
        v[3] += other.v[3];
    }

    Vector4 operator-(const Vector4& other) const {
        return Vector4(v[0] - other.v[0], v[1] - other.v[1], v[2] - other.v[2], v[3] - other.v[3]);
    }
    Vector4 operator-() const { return Vector4(-v[0], -v[1], -v[2], -v[3]); }
    void operator-=(const Vector4& other) {
        v[0] -= other.v[0];
        v[1] -= other.v[1];
        v[2] -= other.v[2];
        v[3] -= other.v[3];
    }

    Vector4 operator*(const valueType& scalar) const {
        return Vector4(v[0] * scalar, v[1] * scalar, v[2] * scalar, v[3] * scalar);
    }
    void operator*=(const valueType& scalar) {
        v[0] *= scalar;
        v[1] *= scalar;
        v[2] *= scalar;
        v[3] *= scalar;
    }

    Vector4 operator/(const valueType& scalar) const {
        if (scalar != 0) {
            return Vector4(v[0] / scalar, v[1] / scalar, v[2] / scalar, v[3] / scalar);
        } else {
            return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }
    void operator/=(const valueType& scalar) {
        if (scalar != 0) {
            v[0] /= scalar;
            v[1] /= scalar;
            v[2] /= scalar;
            v[3] /= scalar;
        } else {
            v[0] = 0;
            v[1] = 0;
            v[2] = 0;
            v[3] = 0;
        }
    }

    // 等号演算子のオーバーロード
    bool operator==(const Vector4& other) const {
        return (v[0] == other.v[0] && v[1] == other.v[1] && v[2] == other.v[2] && v[3] == other.v[3]);
    }
    bool operator!=(const Vector4& other) const {
        return !(*this == other);
    }
};

template <typename valueType = float>
using Vec4     = Vector4<valueType>;
using Vector4f = Vector4<float>;
using Vec4f    = Vector4<float>;

template <typename valueType>
inline Vector4<valueType> operator*(const valueType& scalar, const Vector4<valueType>& vec) {
    return Vector4<valueType>(vec.v[0] * scalar, vec.v[1] * scalar, vec.v[2] * scalar, vec.v[3] * scalar);
}

template <typename valueType, typename timeType>
inline Vector4<valueType> Lerp(const Vector4<valueType>& start, const Vector4<valueType>& end, timeType time) {
    return {
        std::lerp(start.v[0], end.v[0], time),
        std::lerp(start.v[1], end.v[1], time),
        std::lerp(start.v[2], end.v[2], time),
        std::lerp(start.v[3], end.v[3], time)};
}
