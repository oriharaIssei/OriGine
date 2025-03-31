#include "Matrix4x4.h"

/// stl
// assert
#include "assert.h"

/// math
#include "Quaternion.h"

#include <cmath>

Matrix4x4 Matrix4x4::operator+(const Matrix4x4& another) const {
    Matrix4x4 result;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.m[row][col] = this->m[row][col] + another.m[row][col];
        }
    }
    return result;
}

Matrix4x4 Matrix4x4::operator-(const Matrix4x4& another) const {
    Matrix4x4 result;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.m[row][col] = this->m[row][col] - another.m[row][col];
        }
    }
    return result;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& another) const {

    Matrix4x4 result;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i][j] =
                (m[i][0] * another.m[0][j]) + (m[i][1] * another.m[1][j]) + (m[i][2] * another.m[2][j]) + (m[i][3] * another.m[3][j]);
        }
    }

    return result;
}

Matrix4x4 Matrix4x4::operator*(const float& scalar) const {
    Matrix4x4 result;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.m[row][col] = this->m[row][col] * scalar;
        }
    }
    return result;
}

Matrix4x4* Matrix4x4::operator*=(const Matrix4x4& another) {
    *this = *this * another;
    return this;
}

Matrix4x4 Matrix4x4::transpose() const {
    Matrix4x4 result;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.m[row][col] = this->m[col][row];
        }
    }
    return result;
}

Matrix4x4 Matrix4x4::Transpose(const Matrix4x4& m) {
    Matrix4x4 result;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.m[row][col] = m[col][row];
        }
    }
    return result;
}

Matrix4x4 Matrix4x4::inverse() const {
    DirectX::XMMATRIX thisMat = MatrixToXMMATRIX();
    DirectX::XMVECTOR det     = DirectX::XMMatrixDeterminant(thisMat);
    Matrix4x4 inverse;
    inverse.XMMATRIXToMatrix(DirectX::XMMatrixInverse(&det, thisMat));
    return inverse;
}
Matrix4x4 Matrix4x4::Inverse(const Matrix4x4& m) {
    DirectX::XMMATRIX thisMat = m.MatrixToXMMATRIX();
    DirectX::XMVECTOR det     = DirectX::XMMatrixDeterminant(thisMat);
    Matrix4x4 inverse;
    inverse.XMMATRIXToMatrix(DirectX::XMMatrixInverse(&det, thisMat));
    return inverse;
}
const Matrix4x4 MakeMatrix::Identity() {
    return Matrix4x4({1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
}

Matrix4x4 MakeMatrix::Translate(const Vec3f& vec) {
    return Matrix4x4({1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, vec[X], vec[Y], vec[Z], 1.0f});
}

Matrix4x4 MakeMatrix::Scale(const Vec3f& vec) {
    return Matrix4x4(
        {vec[X], 0.0f, 0.0f, 0.0f, 0.0f, vec[Y], 0.0f, 0.0f, 0.0f, 0.0f, vec[Z], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
}

Matrix4x4 MakeMatrix::RotateX(const float& radian) {
    return Matrix4x4({01.0f, .0f, 0.0f, 0.0f, 0.0f, std::cosf(radian), std::sinf(radian), 0.0f, 0.0f, -std::sinf(radian), std::cosf(radian), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
}

Matrix4x4 MakeMatrix::RotateY(const float& radian) {
    return Matrix4x4({std::cosf(radian), 0.0f, -std::sinf(radian), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, std::sinf(radian), 0.0f, std::cosf(radian), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
}

Matrix4x4 MakeMatrix::RotateZ(const float& radian) {
    return Matrix4x4({std::cosf(radian), std::sinf(radian), 0.0f, 0.0f, -std::sinf(radian), std::cosf(radian), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
}

Matrix4x4 MakeMatrix::RotateXYZ(const Vec3f& radian) {
    return MakeMatrix::RotateZ(radian[Z]) * MakeMatrix::RotateX(radian[X]) * MakeMatrix::RotateY(radian[Y]);
}

Matrix4x4 MakeMatrix::RotateXYZ(const Matrix4x4& x, const Matrix4x4& y, const Matrix4x4& z) {
    return z * x * y;
}

Matrix4x4 MakeMatrix::RotateQuaternion(const Quaternion& q) {
    float xy = q.v[X] * q.v[Y];
    float xz = q.v[X] * q.v[Z];
    float yz = q.v[Y] * q.v[Z];
    float wx = q.v[W] * q.v[X];
    float wy = q.v[W] * q.v[Y];
    float wz = q.v[W] * q.v[Z];

    float x2 = q.v[X] * q.v[X];
    float y2 = q.v[Y] * q.v[Y];
    float z2 = q.v[Z] * q.v[Z];
    float w2 = q.v[W] * q.v[W];

    return Matrix4x4(
        {(w2 + x2 - y2 - z2), 2.0f * (xy + wz)   , 2.0f * (xz - wy)   , 0.0f,
            2.0f * (xy - wz), (w2 - x2 + y2 - z2), 2.0f * (yz + wx)   , 0.0f,
            2.0f * (xz + wy), 2.0f * (yz - wx)   , (w2 - x2 - y2 + z2), 0.0f,
                        0.0f,                0.0f,                0.0f, 1.0f});
}

Matrix4x4 MakeMatrix::RotateAxisAngle(const Vec3f& axis, float angle) {
    float sinAngle  = sinf(angle);
    float cosAngle  = cosf(angle);
    float mCosAngle = (1.0f - cosAngle);
    return {
        axis[X] * axis[X] * mCosAngle + cosAngle, axis[X] * axis[Y] * mCosAngle + axis[Z] * sinAngle, axis[X] * axis[Z] * mCosAngle - axis[Y] * sinAngle, 0.0f, axis[X] * axis[Y] * mCosAngle - axis[Z] * sinAngle, axis[Y] * axis[Y] * mCosAngle + cosAngle, axis[Y] * axis[Z] * mCosAngle + axis[X] * sinAngle, 0.0f, axis[X] * axis[Z] * mCosAngle + axis[Y] * sinAngle, axis[Y] * axis[Z] * mCosAngle - axis[X] * sinAngle, axis[Z] * axis[Z] * mCosAngle + cosAngle, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
}

Matrix4x4 MakeMatrix::RotateAxisAngle(const Vec3f& fromV, const Vec3f& toV) {
    float angle = std::acosf(fromV.Dot(toV));
    Vec3f axis  = fromV.cross(toV).normalize();
    return MakeMatrix::RotateAxisAngle(axis, angle);
}

Matrix4x4 MakeMatrix::Affine(const Vec3f& scale, const Vec3f& rotate, const Vec3f& translate) {
    return MakeMatrix::Scale(scale) * MakeMatrix::RotateXYZ(rotate) * MakeMatrix::Translate(translate);
}

Matrix4x4 MakeMatrix::Affine(const Vec3f& scale, const Quaternion& rotate, const Vec3f& translate) {
    return MakeMatrix::Scale(scale) * MakeMatrix::RotateQuaternion(rotate) * MakeMatrix::Translate(translate);
}

Vec3f TransformVector(const Vec3f& vec, const Matrix4x4& matrix) {
    float result[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float hcs[4]    = {vec[X], vec[Y], vec[Z], 1.0f};

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            result[r] += hcs[c] * matrix[c][r];
        }
    }

    assert(result[3] != 0.0f);
    return Vec3f(result[0] / result[3], result[1] / result[3], result[2] / result[3]);
}

Vec3f TransformNormal(const Vec3f& v, const Matrix4x4& m) {
    // 平行移動を無視して計算
    Vec3f result = {
        v[X] * m.m[0][0] + v[Y] * m.m[1][0] + v[Z] * m.m[2][0],
        v[X] * m.m[0][1] + v[Y] * m.m[1][1] + v[Z] * m.m[2][1],
        v[X] * m.m[0][2] + v[Y] * m.m[1][2] + v[Z] * m.m[2][2],
    };

    return result;
}

Matrix4x4 MakeMatrix::PerspectiveFov(const float& fovY, const float& aspectRatio, const float& nearClip, const float& farClip) {
    const float cot = 1.0f / std::tanf(fovY / 2.0f);
    return Matrix4x4(
        {(1.0f / aspectRatio) * cot, 0.0f, 0.0f, 0.0f, 0.0f, cot, 0.0f, 0.0f, 0.0f, 0.0f, farClip / (farClip - nearClip), 1.0f, 0.0f, 0.0f, (-nearClip * farClip) / (farClip - nearClip), 0.0f});
}

Matrix4x4 MakeMatrix::Orthographic(const float& left, const float& top, const float& right, const float& bottom, const float& nearClip, const float& farClip) {
    return Matrix4x4(
        {2.0f / (right - left), 0.0f, 0.0f, 0.0f, 0.0f, 2.0f / (top - bottom), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f / (farClip - nearClip), 0.0f, (left + right) / (left - right), (top + bottom) / (bottom - top), nearClip / (nearClip - farClip), 1.0f});
}

Matrix4x4 MakeMatrix::ViewPort(const float& left, const float& top, const float& width, const float& height, const float& minDepth, const float& maxDepth) {
    return Matrix4x4({width / 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, -(height / 2.0f), 0.0f, 0.0f, 0.0f, 0.0f, maxDepth - minDepth, 0.0f, left + (width / 2.0f), top + (height / 2.0f), minDepth, 1.0f});
}
