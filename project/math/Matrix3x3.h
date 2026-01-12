#pragma once

/// math
#include "math/Vector2.h"

namespace OriGine {

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

    /// <summary>
    /// 転置行列を取得
    /// </summary>
    /// <returns>転置行列</returns>
    Matrix3x3 transpose() const;
    /// <summary>
    /// 指定した行列の転置行列を取得
    /// </summary>
    /// <param name="mat">行列</param>
    /// <returns>転置行列</returns>
    static Matrix3x3 Transpose(const Matrix3x3& mat);

    /// <summary>
    /// 逆行列を取得
    /// </summary>
    /// <returns>逆行列</returns>
    Matrix3x3 inverse() const;
    /// <summary>
    /// 指定した行列の逆行列を取得
    /// </summary>
    /// <param name="mat">行列</param>
    /// <returns>逆行列</returns>
    static Matrix3x3 Inverse(const Matrix3x3& mat);

    /// <summary>
    /// float配列へ変換
    /// </summary>
    /// <param name="mat">変換元の行列</param>
    /// <param name="out">出力先の配列(要素数9)</param>
    static void ToFloatArray(const Matrix3x3& mat, float out[9]);
    /// <summary>
    /// float配列へ変換
    /// </summary>
    /// <param name="out">出力先の配列(要素数9)</param>
    void toFloatArray(float out[9]) const {
        return ToFloatArray(*this, out);
    }
    /// <summary>
    /// float配列から変換
    /// </summary>
    /// <param name="mat">出力先の行列</param>
    /// <param name="in">変換元の配列(要素数9)</param>
    static void FromFloatArray(Matrix3x3& mat, const float in[9]);
    /// <summary>
    /// float配列から変換
    /// </summary>
    /// <param name="in">変換元の配列(要素数9)</param>
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

/// <summary>
/// 平行移動行列を作成
/// </summary>
/// <param name="v">移動量</param>
/// <returns>平行移動行列</returns>
inline Matrix3x3 Translate(const Vec2f& v) {
    return Matrix3x3{
        1, 0, 0,
        0, 1, 0,
        v[X], v[Y], 1};
}

/// <summary>
/// 拡大縮小行列を作成
/// </summary>
/// <param name="v">スケーリング係数</param>
/// <returns>拡大縮小行列</returns>
inline Matrix3x3 Scale(const Vec2f& v) {
    return Matrix3x3{
        v[X], 0, 0,
        0, v[Y], 0,
        0, 0, 1};
}

/// <summary>
/// 回転行列を作成
/// </summary>
/// <param name="rad">回転角(ラジアン)</param>
/// <returns>回転行列</returns>
inline Matrix3x3 Rotate(float rad) {
    float c = cosf(rad);
    float s = sinf(rad);
    return Matrix3x3{
        c, s, 0,
        -s, c, 0,
        0, 0, 1};
}

/// <summary>
/// 2Dアフィン変換行列を作成
/// </summary>
/// <param name="scale">スケーリング</param>
/// <param name="rotate">回転(ラジアン)</param>
/// <param name="trans">平行移動</param>
/// <returns>アフィン変換行列</returns>
inline Matrix3x3 Affine(const Vec2f& scale, float rotate, const Vec2f& trans) {
    return Scale(scale) * Rotate(rotate) * Translate(trans);
}
}

using Mat3f = Matrix3x3;
} // namespace OriGine
