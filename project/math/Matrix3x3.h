#pragma once

/// math
#include "math/Vector2.h"

/// <summary>
/// 3x3 行列（2D アフィン行列）
/// </summary>
struct Matrix3x3 {
    float m[3][3];

    ///
    /// 演算子
    ///
    float* operator[](int index) { return m[index]; }
    const float* operator[](int index) const { return m[index]; }

    Matrix3x3 operator+(const Matrix3x3& another) const;
    Matrix3x3 operator-(const Matrix3x3& another) const;
    Matrix3x3 operator*(const Matrix3x3& another) const;
    Matrix3x3 operator*(const float& scalar) const;

    Matrix3x3& operator*=(const Matrix3x3& another);

    Matrix3x3 transpose() const;
    static Matrix3x3 Transpose(const Matrix3x3& mat);

    Matrix3x3 inverse() const;
    static Matrix3x3 Inverse(const Matrix3x3& mat);

    static void ToFloatArray(const Matrix3x3& mat, float out[9]);
    void toFloatArray(float out[9]) const {
        return ToFloatArray(*this, out);
    }
    static void FromFloatArray(Matrix3x3& mat, const float in[9]);
    void fromFloatArray(const float in[9]) {
        FromFloatArray(*this, in);
    }

    /// <summary>
    /// 行列から SRT (Scale, Rotate, Translate) を抽出
    /// </summary>
    static void Decompose2D(
        const Matrix3x3& mat,
        Vec2f& outScale,
        float& outRotate,
        Vec2f& outTranslate);

    void decompose2D(
        Vec2f& outScale,
        float& outRotate,
        Vec2f& outTranslate) const {
        Decompose2D(*this, outScale, outRotate, outTranslate);
    }
};

namespace MakeMatrix3x3 {
/// <summary>
/// 単位行列を作成
/// </summary>
/// <returns></returns>
inline Matrix3x3 Identity() {
    return Matrix3x3{
        1, 0, 0,
        0, 1, 0,
        0, 0, 1};
}

inline Matrix3x3 Translate(const Vec2f& v) {
    return Matrix3x3{
        1, 0, 0,
        0, 1, 0,
        v[X], v[Y], 1};
}

inline Matrix3x3 Scale(const Vec2f& v) {
    return Matrix3x3{
        v[X], 0, 0,
        0, v[Y], 0,
        0, 0, 1};
}

inline Matrix3x3 Rotate(float rad) {
    float c = cosf(rad);
    float s = sinf(rad);
    return Matrix3x3{
        c, s, 0,
        -s, c, 0,
        0, 0, 1};
}

inline Matrix3x3 Affine(const Vec2f& scale, float rotate, const Vec2f& trans) {
    return Scale(scale) * Rotate(rotate) * Translate(trans);
}
}
