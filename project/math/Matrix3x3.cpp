#include "Matrix3x3.h"

/// math
#include <DirectXMath.h>

namespace OriGine {

Matrix3x3 Matrix3x3::operator+(const Matrix3x3& _another) const {
    Matrix3x3 r{};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            r[i][j] = m[i][j] + _another[i][j];
    return r;
}

Matrix3x3 Matrix3x3::operator-(const Matrix3x3& _another) const {
    Matrix3x3 r{};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            r[i][j] = m[i][j] - _another[i][j];
    return r;
}

Matrix3x3 Matrix3x3::operator*(const Matrix3x3& _another) const {
    Matrix3x3 r{};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            r[i][j] = m[i][0] * _another[0][j] + m[i][1] * _another[1][j] + m[i][2] * _another[2][j];
    return r;
}

Matrix3x3 Matrix3x3::operator*(const float& _scalar) const {
    Matrix3x3 r{};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            r[i][j] = m[i][j] * _scalar;
    return r;
}

Matrix3x3& Matrix3x3::operator*=(const Matrix3x3& _another) {
    *this = *this * _another;
    return *this;
}

Matrix3x3 Matrix3x3::transpose() const {
    return Transpose(*this);
}

Matrix3x3 Matrix3x3::Transpose(const Matrix3x3& _mat) {
    Matrix3x3 r{};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            r[i][j] = _mat[j][i];
    return r;
}

Matrix3x3 Matrix3x3::Inverse(const Matrix3x3& _mat) {
    // 3x3 を 4x4 に埋めたテンポラリ（行優先で埋める）
    DirectX::XMFLOAT4X4 src{
        _mat[0][0], _mat[0][1], _mat[0][2], 0.0f,
        _mat[1][0], _mat[1][1], _mat[1][2], 0.0f,
        _mat[2][0], _mat[2][1], _mat[2][2], 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};

    // SIMD ロード
    DirectX::XMMATRIX mat = DirectX::XMLoadFloat4x4(&src);

    // 行列式を取得して特異かを判定
    DirectX::XMVECTOR detVec = DirectX::XMMatrixDeterminant(mat);
    float det                = DirectX::XMVectorGetX(detVec);
    if (std::fabs(det) < 1e-6f) {
        // 特異（逆行列が存在しない） -> ゼロ行列を返す（既存実装と合わせる）
        return Matrix3x3{};
    }

    // 逆行列を計算
    DirectX::XMMATRIX invMat = DirectX::XMMatrixInverse(&detVec, mat);

    // 結果を XMFLOAT4X4 に格納
    DirectX::XMFLOAT4X4 out;
    DirectX::XMStoreFloat4x4(&out, invMat);

    // 上位 3x3 を取り出して返す
    Matrix3x3 r{};
    r[0][0] = out.m[0][0];
    r[0][1] = out.m[0][1];
    r[0][2] = out.m[0][2];
    r[1][0] = out.m[1][0];
    r[1][1] = out.m[1][1];
    r[1][2] = out.m[1][2];
    r[2][0] = out.m[2][0];
    r[2][1] = out.m[2][1];
    r[2][2] = out.m[2][2];

    return r;
}

void Matrix3x3::ToFloatArray(const Matrix3x3& _mat, float _out[9]) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            _out[i * 3 + j] = _mat[i][j];
        }
    }
}

void Matrix3x3::FromFloatArray(Matrix3x3& _mat, const float _in[9]) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            _mat[i][j] = _in[i * 3 + j];
        }
    }
}

void Matrix3x3::Decompose2D(const Matrix3x3& _mat, Vec2f& _outScale, float& _outRotate, Vec2f& _outTranslate) {
    // スケール
    _outScale[X] = std::sqrt(_mat[0][0] * _mat[0][0] + _mat[1][0] * _mat[1][0]);
    _outScale[Y] = std::sqrt(_mat[0][1] * _mat[0][1] + _mat[1][1] * _mat[1][1]);
    // 回転（ラジアン）
    _outRotate = std::atan2(_mat[1][0], _mat[0][0]);
    // 平行移動
    _outTranslate[X] = _mat[2][0];
    _outTranslate[Y] = _mat[2][1];
}

Matrix3x3 Matrix3x3::inverse() const {
    return Matrix3x3::Inverse(*this);
}

} // namespace OriGine
