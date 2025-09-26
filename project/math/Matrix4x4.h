#pragma once

#include <DirectXMath.h>
#include <Quaternion.h>
#include <Vector3.h>

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

    static Quaternion DecomposeMatrixToQuaternion(const Matrix4x4& _mat);
    Quaternion decomposeMatrixToQuaternion() const {
        return DecomposeMatrixToQuaternion(*this);
    }

    static DirectX::XMMATRIX MatrixToXMMATRIX(const Matrix4x4& _mat) {
        return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&_mat));
    }
    DirectX::XMMATRIX matrixToXMMATRIX() const {
        return MatrixToXMMATRIX(*this);
    }

    static Matrix4x4 XMMATRIXToMatrix(const DirectX::XMMATRIX& _xmmat) {
        Matrix4x4 result{};
        XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&result), _xmmat);
        return result;
    }
    void xmmatrixToMatrix(const DirectX::XMMATRIX& _xmmat) {
        *this = XMMATRIXToMatrix(_xmmat);
    }
};

namespace MakeMatrix {
const Matrix4x4 Identity();

Matrix4x4 Translate(const Vec3f& vec);
Matrix4x4 Scale(const Vec3f& vec);

Matrix4x4 RotateX(const float& radian);
Matrix4x4 RotateY(const float& radian);
Matrix4x4 RotateZ(const float& radian);
Matrix4x4 RotateXYZ(const Vec3f& radian);
Matrix4x4 RotateXYZ(const Matrix4x4& x, const Matrix4x4& y, const Matrix4x4& z);

Matrix4x4 RotateQuaternion(const Quaternion& q);

Matrix4x4 RotateAxisAngle(const Vec3f& axis, float angle);
Matrix4x4 RotateAxisAngle(const Vec3f& fromV, const Vec3f& toV);

Matrix4x4 Affine(const Vec3f& scale, const Vec3f& rotate, const Vec3f& translate);
Matrix4x4 Affine(const Vec3f& scale, const Quaternion& rotate, const Vec3f& translate);

Matrix4x4 PerspectiveFov(const float& fovY, const float& aspectRatio, const float& nearClip, const float& farClip);
Matrix4x4 Orthographic(const float& left, const float& top, const float& right, const float& bottom, const float& nearClip, const float& farClip);
Matrix4x4 ViewPort(const float& left, const float& top, const float& width, const float& height, const float& minDepth, const float& maxDepth);
} // namespace MakeMatrix

Vec3f TransformVector(const Vec3f& vec, const Matrix4x4& matrix);
// インライン関数として定義
inline Vec3f operator*(const Vec3f& vec, const Matrix4x4& matrix) {
    return TransformVector(vec, matrix);
}

Vec3f TransformNormal(const Vec3f& v, const Matrix4x4& m);

Vec2f WorldToScreen(const Vec3f& _worldPos, const Matrix4x4& _vpvpvMat);
