#pragma once

/// math
#include <DirectXMath.h>
#include <Quaternion.h>
#include <Vector3.h>

namespace OriGine {

/// <summary>
/// 4x4 行列
/// </summary>
struct Matrix4x4 {
    float m[4][4];

    ///
    ///	演算子
    ///
    float* operator[](int index) { return m[index]; }
    const float* operator[](int index) const { return m[index]; }

    Matrix4x4 operator+(const Matrix4x4& another) const;
    Matrix4x4 operator-(const Matrix4x4& another) const;
    Matrix4x4 operator*(const Matrix4x4& another) const;
    Matrix4x4 operator*(const float& scalar) const;

    Matrix4x4* operator*=(const Matrix4x4& another);

    Matrix4x4 transpose() const;
    static Matrix4x4 Transpose(const Matrix4x4& m);

    Matrix4x4 inverse() const;
    static Matrix4x4 Inverse(const Matrix4x4& m);

    static void ToFloatArray(const Matrix4x4& mat, float out[16]);
    void toFloatArray(float out[16]) const {
        return ToFloatArray(*this, out);
    }
    static void FromFloatArray(Matrix4x4& mat, const float in[16]);
    void fromFloatArray(const float in[16]) {
        FromFloatArray(*this, in);
    }

    /// <summary>
    /// Matrixから srt (Scale, Rotate, Translate) の成分を抽出
    /// </summary>
    /// <param name="mat"></param>
    /// <param name="outScale"></param>
    /// <param name="outRotate"></param>
    /// <param name="outTranslate"></param>
    static void DecomposeMatrixToComponents(
        const Matrix4x4& mat,
        Vec3f& outScale,
        Quaternion& outRotate,
        Vec3f& outTranslate);
    /// <summary>
    /// Matrixから srt (Scale, Rotate, Translate) の成分を抽出
    /// </summary>
    /// <param name="outScale"></param>
    /// <param name="outRotate"></param>
    /// <param name="outTranslate"></param>
    void decomposeMatrixToComponents(
        Vec3f& outScale,
        Quaternion& outRotate,
        Vec3f& outTranslate) const {
        DecomposeMatrixToComponents(*this, outScale, outRotate, outTranslate);
    }

    /// <summary>
    /// 行列から回転成分をQuaternionで抽出
    /// </summary>
    /// <param name="_mat">抽出元の行列</param>
    /// <returns>抽出した回転成分</returns>
    static Quaternion DecomposeMatrixToQuaternion(const Matrix4x4& _mat);
    Quaternion decomposeMatrixToQuaternion() const {
        return DecomposeMatrixToQuaternion(*this);
    }

    /// <summary>
    /// DirectX::XMMATRIX への変換
    /// </summary>
    static DirectX::XMMATRIX MatrixToXMMATRIX(const Matrix4x4& _mat) {
        return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&_mat));
    }
    DirectX::XMMATRIX matrixToXMMATRIX() const {
        return MatrixToXMMATRIX(*this);
    }

    /// <summary>
    /// DirectX::XMMATRIX からの変換
    /// </summary>
    static Matrix4x4 XMMATRIXToMatrix(const DirectX::XMMATRIX& _xmmat) {
        Matrix4x4 result{};
        XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&result), _xmmat);
        return result;
    }
    void xmmatrixToMatrix(const DirectX::XMMATRIX& _xmmat) {
        *this = XMMATRIXToMatrix(_xmmat);
    }
};

using Mat4f = Matrix4x4;

namespace MakeMatrix4x4 {
/// <summary>
/// 単位行列を作成
/// </summary>
/// <returns></returns>
const Matrix4x4 Identity();

/// <summary>
/// 平行移動行列を作成
/// </summary>
/// <param name="vec"></param>
/// <returns></returns>
Matrix4x4 Translate(const Vec3f& vec);
/// <summary>
/// 拡大縮小行列を作成
/// </summary>
Matrix4x4 Scale(const Vec3f& vec);

/// <summary>
/// X軸回転行列を作成
/// </summary>
/// <param name="radian"></param>
/// <returns></returns>
Matrix4x4 RotateX(const float& radian);
/// <summary>
/// Y軸回転行列を作成
/// </summary>
/// <param name="radian"></param>
/// <returns></returns>
Matrix4x4 RotateY(const float& radian);
/// <summary>
/// Z軸回転行列を作成
/// </summary>
Matrix4x4 RotateZ(const float& radian);
/// <summary>
/// XYZ順の回転行列を作成
/// </summary>
Matrix4x4 RotateXYZ(const Vec3f& radian);
Matrix4x4 RotateXYZ(const Matrix4x4& x, const Matrix4x4& y, const Matrix4x4& z);
/// <summary>
/// Quaternionから回転行列を作成
/// </summary>
/// <param name="q"></param>
/// <returns></returns>
Matrix4x4 RotateQuaternion(const Quaternion& q);

/// <summary>
/// 任意軸回転行列を作成
/// </summary>
/// <param name="axis">任意軸</param>
/// <param name="angle"></param>
/// <returns></returns>
Matrix4x4 RotateAxisAngle(const Vec3f& axis, float angle);
/// <summary>
/// fromベクトルからtoベクトルへの回転行列を作成
/// </summary>
Matrix4x4 RotateAxisAngle(const Vec3f& fromV, const Vec3f& toV);

/// <summary>
/// アフィン変換行列を作成
/// </summary>
Matrix4x4 Affine(const Vec3f& scale, const Vec3f& rotate, const Vec3f& translate);
/// <summary>
/// アフィン変換行列を作成
/// </summary>
/// <param name="scale"></param>
/// <param name="rotate"></param>
/// <param name="translate"></param>
/// <returns></returns>
Matrix4x4 Affine(const Vec3f& scale, const Quaternion& rotate, const Vec3f& translate);

/// <summary>
/// 透視投影行列を作成
/// </summary>
Matrix4x4 PerspectiveFov(const float& fovY, const float& aspectRatio, const float& nearClip, const float& farClip);
/// <summary>
/// 正射影行列を作成
/// </summary>
Matrix4x4 Orthographic(const float& left, const float& top, const float& right, const float& bottom, const float& nearClip, const float& farClip);
/// <summary>
/// ビューポート行列を作成
/// </summary>
Matrix4x4 ViewPort(const float& left, const float& top, const float& width, const float& height, const float& minDepth, const float& maxDepth);
} // namespace MakeMatrix4x4

/// <summary>
/// ベクトルを行列で変換
/// </summary>
/// <param name="vec"></param>
/// <param name="matrix"></param>
/// <returns></returns>
Vec3f TransformVector(const Vec3f& vec, const Matrix4x4& matrix);
// インライン関数として定義
inline Vec3f operator*(const Vec3f& vec, const Matrix4x4& matrix) {
    return TransformVector(vec, matrix);
}

/// <summary>
/// ベクトルを平行移動を無視して変換
/// </summary>
Vec3f TransformNormal(const Vec3f& v, const Matrix4x4& m);

/// <summary>
/// ワールド座標をスクリーン座標に変換
/// </summary>
Vec2f WorldToScreen(const Vec3f& _worldPos, const Matrix4x4& _vpvpvMat);

} // namespace OriGine
