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
    float* operator[](int _index) { return m[_index]; }
    const float* operator[](int _index) const { return m[_index]; }

    Matrix4x4 operator+(const Matrix4x4& _another) const;
    Matrix4x4 operator-(const Matrix4x4& _another) const;
    Matrix4x4 operator*(const Matrix4x4& _another) const;
    Matrix4x4 operator*(const float& _scalar) const;

    Matrix4x4* operator*=(const Matrix4x4& _another);

    /// <summary>
    /// 転置行列を取得
    /// </summary>
    /// <returns>転置行列</returns>
    Matrix4x4 transpose() const;
    /// <summary>
    /// 指定した行列の転置行列を取得
    /// </summary>
    /// <param name="m">行列</param>
    /// <returns>転置行列</returns>
    static Matrix4x4 Transpose(const Matrix4x4& _m);

    /// <summary>
    /// 逆行列を取得
    /// </summary>
    /// <returns>逆行列</returns>
    Matrix4x4 inverse() const;
    /// <summary>
    /// 指定した行列の逆行列を取得
    /// </summary>
    /// <param name="m">行列</param>
    /// <returns>逆行列</returns>
    static Matrix4x4 Inverse(const Matrix4x4& _m);

    /// <summary>
    /// float配列へ変換
    /// </summary>
    /// <param name="mat">変換元の行列</param>
    /// <param name="out">出力先の配列(要素数16)</param>
    static void ToFloatArray(const Matrix4x4& _mat, float _out[16]);
    /// <summary>
    /// float配列へ変換
    /// </summary>
    /// <param name="out">出力先の配列(要素数16)</param>
    void toFloatArray(float _out[16]) const {
        return ToFloatArray(*this, _out);
    }
    /// <summary>
    /// float配列から変換
    /// </summary>
    /// <param name="mat">出力先の行列</param>
    /// <param name="in">変換元の配列(要素数16)</param>
    static void FromFloatArray(Matrix4x4& _mat, const float _in[16]);
    /// <summary>
    /// float配列から変換
    /// </summary>
    /// <param name="in">変換元の配列(要素数16)</param>
    void fromFloatArray(const float _in[16]) {
        FromFloatArray(*this, _in);
    }

    /// <summary>
    /// Matrixから srt (Scale, Rotate, Translate) の成分を抽出
    /// </summary>
    /// <param name="mat"></param>
    /// <param name="outScale"></param>
    /// <param name="outRotate"></param>
    /// <param name="outTranslate"></param>
    static void DecomposeMatrixToComponents(
        const Matrix4x4& _mat,
        Vec3f& _outScale,
        Quaternion& _outRotate,
        Vec3f& _outTranslate);
    /// <summary>
    /// Matrixから srt (Scale, Rotate, Translate) の成分を抽出
    /// </summary>
    /// <param name="outScale"></param>
    /// <param name="outRotate"></param>
    /// <param name="outTranslate"></param>
    void decomposeMatrixToComponents(
        Vec3f& _outScale,
        Quaternion& _outRotate,
        Vec3f& _outTranslate) const {
        DecomposeMatrixToComponents(*this, _outScale, _outRotate, _outTranslate);
    }

    /// <summary>
    /// 行列から回転成分をQuaternionで抽出
    /// </summary>
    /// <param name="_mat">抽出元の行列</param>
    /// <returns>抽出した回転成分</returns>
    static Quaternion DecomposeMatrixToQuaternion(const Matrix4x4& _mat);
    /// <summary>
    /// 行列から回転成分をQuaternionで抽出
    /// </summary>
    /// <returns>回転成分</returns>
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
Matrix4x4 Translate(const Vec3f& _vec);
/// <summary>
/// 拡大縮小行列を作成
/// </summary>
Matrix4x4 Scale(const Vec3f& _vec);
/// <summary>
/// X軸回転行列を作成
/// </summary>
/// <param name="radian"></param>
/// <returns></returns>
Matrix4x4 RotateX(const float& _radian);
/// <summary>
/// Y軸回転行列を作成
/// </summary>
/// <param name="radian"></param>
/// <returns></returns>
Matrix4x4 RotateY(const float& _radian);
/// <summary>
/// Z軸回転行列を作成
/// </summary>
Matrix4x4 RotateZ(const float& _radian);
/// <summary>
/// XYZ順の回転行列を作成
/// </summary>
/// <param name="radian">各方向への回転角</param>
/// <returns>回転行列</returns>
Matrix4x4 RotateXYZ(const Vec3f& _radian);
/// <summary>
/// 各軸の回転行列を合成
/// </summary>
/// <param name="x">X軸回転行列</param>
/// <param name="y">Y軸回転行列</param>
/// <param name="z">Z軸回転行列</param>
/// <returns>合成した回転行列</returns>
Matrix4x4 RotateXYZ(const Matrix4x4& _x, const Matrix4x4& _y, const Matrix4x4& _z);
/// <summary>
/// Quaternionから回転行列を作成
/// </summary>
/// <param name="q"></param>
/// <returns></returns>
Matrix4x4 RotateQuaternion(const Quaternion& _q);

/// <summary>
/// 任意軸回転行列を作成
/// </summary>
/// <param name="axis">任意軸</param>
/// <param name="angle"></param>
/// <returns></returns>
Matrix4x4 RotateAxisAngle(const Vec3f& _axis, float _angle);
/// <summary>
/// fromベクトルからtoベクトルへの回転行列を作成
/// </summary>
Matrix4x4 RotateAxisAngle(const Vec3f& _fromV, const Vec3f& _toV);

/// <summary>
/// アフィン変換行列を作成
/// </summary>
Matrix4x4 Affine(const Vec3f& _scale, const Vec3f& _rotate, const Vec3f& _translate);
/// <summary>
/// アフィン変換行列を作成
/// </summary>
/// <param name="scale"></param>
/// <param name="rotate"></param>
/// <param name="translate"></param>
/// <returns></returns>
Matrix4x4 Affine(const Vec3f& _scale, const Quaternion& _rotate, const Vec3f& _translate);

/// <summary>
/// 透視投影行列を作成
/// </summary>
Matrix4x4 PerspectiveFov(const float& _fovY, const float& _aspectRatio, const float& _nearClip, const float& _farClip);
/// <summary>
/// 正射影行列を作成
/// </summary>
Matrix4x4 Orthographic(const float& _left, const float& _top, const float& _right, const float& _bottom, const float& _nearClip, const float& _farClip);
/// <summary>
/// ビューポート行列を作成
/// </summary>
Matrix4x4 ViewPort(const float& _left, const float& _top, const float& _width, const float& _height, const float& _minDepth, const float& _maxDepth);
} // namespace MakeMatrix4x4

/// <summary>
/// ベクトルを行列で変換
/// </summary>
/// <param name="vec"></param>
/// <param name="matrix"></param>
/// <returns></returns>
Vec3f TransformVector(const Vec3f& _vec, const Matrix4x4& _matrix);
// インライン関数として定義
inline Vec3f operator*(const Vec3f& _vec, const Matrix4x4& _matrix) {
    return TransformVector(_vec, _matrix);
}

/// <summary>
/// ベクトルを平行移動を無視して変換
/// </summary>
Vec3f TransformNormal(const Vec3f& _v, const Matrix4x4& _m);

/// <summary>
/// ワールド座標をスクリーン座標に変換
/// </summary>
Vec2f WorldToScreen(const Vec3f& _worldPos, const Matrix4x4& _vpvpvMat);

/// <summary>
/// スクリーン座標をワールド座標に変換
/// </summary>
/// <param name="_screenPos"></param>
/// <param name="_depth"></param>
/// <param name="_invVpvpMat"></param>
/// <returns></returns>
Vec3f ScreenToWorld(const Vec2f& _screenPos, const float& _depth, const Matrix4x4& _invVpvpMat);

} // namespace OriGine
