#include "Matrix4x4.h"

/// stl
// assert
#include "assert.h"

/// math
#include "Quaternion.h"

#include <cmath>

namespace OriGine {

Matrix4x4 Matrix4x4::operator+(const Matrix4x4& _another) const {
    Matrix4x4 result;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.m[row][col] = this->m[row][col] + _another.m[row][col];
        }
    }
    return result;
}

Matrix4x4 Matrix4x4::operator-(const Matrix4x4& _another) const {
    Matrix4x4 result;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.m[row][col] = this->m[row][col] - _another.m[row][col];
        }
    }
    return result;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& _another) const {
    auto thisMat    = this->matrixToXMMATRIX();
    auto anotherMat = _another.matrixToXMMATRIX();

    DirectX::XMMATRIX result = DirectX::XMMatrixMultiply(thisMat, anotherMat);

    return Matrix4x4::XMMATRIXToMatrix(result);
}

Matrix4x4 Matrix4x4::operator*(const float& _scalar) const {
    DirectX::XMMATRIX thisMat   = this->matrixToXMMATRIX();
    DirectX::XMMATRIX resultMat = DirectX::XMMatrixMultiply(thisMat, DirectX::XMMatrixScaling(_scalar, _scalar, _scalar));

    return XMMATRIXToMatrix(resultMat);
}

Matrix4x4* Matrix4x4::operator*=(const Matrix4x4& _another) {
    *this = *this * _another;
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

Matrix4x4 Matrix4x4::Transpose(const Matrix4x4& _m) {
    Matrix4x4 result;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.m[row][col] = _m[col][row];
        }
    }
    return result;
}

Matrix4x4 Matrix4x4::inverse() const {
    DirectX::XMMATRIX thisMat = matrixToXMMATRIX();
    DirectX::XMVECTOR det     = DirectX::XMMatrixDeterminant(thisMat);
    Matrix4x4 inverse;
    inverse.xmmatrixToMatrix(DirectX::XMMatrixInverse(&det, thisMat));
    return inverse;
}
Matrix4x4 Matrix4x4::Inverse(const Matrix4x4& _m) {
    DirectX::XMMATRIX thisMat = _m.matrixToXMMATRIX();
    DirectX::XMVECTOR det     = DirectX::XMMatrixDeterminant(thisMat);
    Matrix4x4 inverse;
    inverse.xmmatrixToMatrix(DirectX::XMMatrixInverse(&det, thisMat));
    return inverse;
}
void Matrix4x4::ToFloatArray(const Matrix4x4& _mat, float _out[16]) {
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            _out[col + row * 4] = _mat.m[row][col]; // 行優先
        }
    }
}
void Matrix4x4::FromFloatArray(Matrix4x4& _mat, const float _in[16]) {
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            _mat.m[row][col] = _in[col + row * 4];
        }
    }
}

void Matrix4x4::DecomposeMatrixToComponents(const Matrix4x4& _mat, Vec3f& _outScale, Quaternion& _outRotate, Vec3f& _outTranslate) {
    DirectX::XMMATRIX xmMat = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&_mat));
    DirectX::XMVECTOR scale, rotQuat, trans;
    if (XMMatrixDecompose(&scale, &rotQuat, &trans, xmMat)) {
        DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&_outScale), scale);
        DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&_outTranslate), trans);
        DirectX::XMFLOAT4 q;
        XMStoreFloat4(&q, rotQuat);
        _outRotate = Quaternion(q.x, q.y, q.z, q.w);
    } else {
        // 失敗時は単位値をセット
        _outScale     = Vec3f(1, 1, 1);
        _outRotate    = Quaternion(0, 0, 0, 1);
        _outTranslate = Vec3f(0, 0, 0);
    }
}

Quaternion Matrix4x4::DecomposeMatrixToQuaternion(const Matrix4x4& _mat) {
    Quaternion result;
    DirectX::XMMATRIX xmMat = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&_mat));
    DirectX::XMVECTOR q     = DirectX::XMQuaternionRotationMatrix(xmMat);
    DirectX::XMFLOAT4 qf;
    DirectX::XMStoreFloat4(&qf, q);
    result = Quaternion(qf.x, qf.y, qf.z, qf.w);

    return result;
}

const Matrix4x4 MakeMatrix4x4::Identity() {
    return Matrix4x4(
        {1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f});
}

Matrix4x4 MakeMatrix4x4::Translate(const Vec3f& _vec) {
    return Matrix4x4({1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, _vec[X], _vec[Y], _vec[Z], 1.0f});
}

Matrix4x4 MakeMatrix4x4::Scale(const Vec3f& _vec) {
    return Matrix4x4(
        {_vec[X], 0.0f, 0.0f, 0.0f, 0.0f, _vec[Y], 0.0f, 0.0f, 0.0f, 0.0f, _vec[Z], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
}

Matrix4x4 MakeMatrix4x4::RotateX(const float& _radian) {
    return Matrix4x4({01.0f, .0f, 0.0f, 0.0f, 0.0f, std::cosf(_radian), std::sinf(_radian), 0.0f, 0.0f, -std::sinf(_radian), std::cosf(_radian), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
}

Matrix4x4 MakeMatrix4x4::RotateY(const float& _radian) {
    return Matrix4x4({std::cosf(_radian), 0.0f, -std::sinf(_radian), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, std::sinf(_radian), 0.0f, std::cosf(_radian), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
}

Matrix4x4 MakeMatrix4x4::RotateZ(const float& _radian) {
    return Matrix4x4({std::cosf(_radian), std::sinf(_radian), 0.0f, 0.0f, -std::sinf(_radian), std::cosf(_radian), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
}

Matrix4x4 MakeMatrix4x4::RotateXYZ(const Vec3f& _radian) {
    return MakeMatrix4x4::RotateX(_radian[X]) * MakeMatrix4x4::RotateY(_radian[Y]) * MakeMatrix4x4::RotateZ(_radian[Z]);
}

Matrix4x4 MakeMatrix4x4::RotateXYZ(const Matrix4x4& _x, const Matrix4x4& _y, const Matrix4x4& _z) {
    return _z * _x * _y;
}

Matrix4x4 MakeMatrix4x4::RotateQuaternion(const Quaternion& _q) {
    float xy = _q.v[X] * _q.v[Y];
    float xz = _q.v[X] * _q.v[Z];
    float yz = _q.v[Y] * _q.v[Z];
    float wx = _q.v[W] * _q.v[X];
    float wy = _q.v[W] * _q.v[Y];
    float wz = _q.v[W] * _q.v[Z];

    float x2 = _q.v[X] * _q.v[X];
    float y2 = _q.v[Y] * _q.v[Y];
    float z2 = _q.v[Z] * _q.v[Z];
    float w2 = _q.v[W] * _q.v[W];

    return Matrix4x4(
        {(w2 + x2 - y2 - z2), 2.0f * (xy + wz), 2.0f * (xz - wy), 0.0f,
            2.0f * (xy - wz), (w2 - x2 + y2 - z2), 2.0f * (yz + wx), 0.0f,
            2.0f * (xz + wy), 2.0f * (yz - wx), (w2 - x2 - y2 + z2), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f});
}

Matrix4x4 MakeMatrix4x4::RotateAxisAngle(const Vec3f& _axis, float _angle) {
    float sinAngle  = sinf(_angle);
    float cosAngle  = cosf(_angle);
    float mCosAngle = (1.0f - cosAngle);
    return {
        _axis[X] * _axis[X] * mCosAngle + cosAngle, _axis[X] * _axis[Y] * mCosAngle + _axis[Z] * sinAngle, _axis[X] * _axis[Z] * mCosAngle - _axis[Y] * sinAngle, 0.0f, _axis[X] * _axis[Y] * mCosAngle - _axis[Z] * sinAngle, _axis[Y] * _axis[Y] * mCosAngle + cosAngle, _axis[Y] * _axis[Z] * mCosAngle + _axis[X] * sinAngle, 0.0f, _axis[X] * _axis[Z] * mCosAngle + _axis[Y] * sinAngle, _axis[Y] * _axis[Z] * mCosAngle - _axis[X] * sinAngle, _axis[Z] * _axis[Z] * mCosAngle + cosAngle, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
}

Matrix4x4 MakeMatrix4x4::RotateAxisAngle(const Vec3f& _fromV, const Vec3f& _toV) {
    float angle = std::acosf(_fromV.dot(_toV));
    Vec3f axis  = _fromV.cross(_toV).normalize();
    return MakeMatrix4x4::RotateAxisAngle(axis, angle);
}

Matrix4x4 MakeMatrix4x4::Affine(const Vec3f& _scale, const Vec3f& _rotate, const Vec3f& _translate) {
    return MakeMatrix4x4::Scale(_scale) * MakeMatrix4x4::RotateXYZ(_rotate) * MakeMatrix4x4::Translate(_translate);
}

Matrix4x4 MakeMatrix4x4::Affine(const Vec3f& _scale, const Quaternion& _rotate, const Vec3f& _translate) {
    return MakeMatrix4x4::Scale(_scale) * MakeMatrix4x4::RotateQuaternion(_rotate) * MakeMatrix4x4::Translate(_translate);
}

Vec3f TransformVector(const Vec3f& _vec, const Matrix4x4& _matrix) {
    DirectX::XMVECTOR vecXM = DirectX::XMVectorSet(_vec[X], _vec[Y], _vec[Z], 1.0f); // w=1
    DirectX::XMMATRIX matXM = _matrix.matrixToXMMATRIX();

    DirectX::XMVECTOR resultXM = DirectX::XMVector4Transform(vecXM, matXM);

    float result[4];
    DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(result), resultXM);

    if (result[3] == 0.0f) {
        return Vec3f(0.f, 0.f, 0.f);
    }

    return Vec3f(result[0] / result[3], result[1] / result[3], result[2] / result[3]);
}

Vec3f TransformNormal(const Vec3f& _v, const Matrix4x4& _m) {
    // 平行移動を無視して計算
    Vec3f result = {
        _v[X] * _m.m[0][0] + _v[Y] * _m.m[1][0] + _v[Z] * _m.m[2][0],
        _v[X] * _m.m[0][1] + _v[Y] * _m.m[1][1] + _v[Z] * _m.m[2][1],
        _v[X] * _m.m[0][2] + _v[Y] * _m.m[1][2] + _v[Z] * _m.m[2][2],
    };

    return result;
}

Vec2f WorldToScreen(const Vec3f& _worldPos, const Matrix4x4& _vpvpvMat) {
    // ワールド座標をビュー変換
    // ビュー空間からスクリーン空間へ変換
    Vec3f screenSpace = TransformVector(_worldPos, _vpvpvMat);

    return Vec2f(screenSpace[X], screenSpace[Y]);
}

Vec3f ScreenToWorld(const Vec2f& _screenPos, const float& _depth, const Matrix4x4& _invVpvpMat) {
    // スクリーン座標を3Dベクトルに拡張
    Vec3f screenPos3D = Vec3f(_screenPos[X], _screenPos[Y], _depth);
    // スクリーン座標からワールド座標へ変換
    Vec3f worldPos = TransformVector(screenPos3D, _invVpvpMat);
    return worldPos;
}

Matrix4x4 MakeMatrix4x4::PerspectiveFov(const float& _fovY, const float& _aspectRatio, const float& _nearClip, const float& _farClip) {
    const float cot = 1.0f / std::tanf(_fovY / 2.0f);
    return Matrix4x4(
        {(1.0f / _aspectRatio) * cot, 0.0f, 0.0f, 0.0f, 0.0f, cot, 0.0f, 0.0f, 0.0f, 0.0f, _farClip / (_farClip - _nearClip), 1.0f, 0.0f, 0.0f, (-_nearClip * _farClip) / (_farClip - _nearClip), 0.0f});
}

Matrix4x4 MakeMatrix4x4::Orthographic(const float& _left, const float& _top, const float& _right, const float& _bottom, const float& _nearClip, const float& _farClip) {
    return Matrix4x4(
        {2.0f / (_right - _left), 0.0f, 0.0f, 0.0f, 0.0f, 2.0f / (_top - _bottom), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f / (_farClip - _nearClip), 0.0f, (_left + _right) / (_left - _right), (_top + _bottom) / (_bottom - _top), _nearClip / (_nearClip - _farClip), 1.0f});
}

Matrix4x4 MakeMatrix4x4::ViewPort(const float& _left, const float& _top, const float& _width, const float& _height, const float& _minDepth, const float& _maxDepth) {
    return Matrix4x4(
        {_width / 2.0f, 0.0f, 0.0f, 0.0f,
            0.0f, -(_height / 2.0f), 0.0f, 0.0f,
            0.0f, 0.0f, _maxDepth - _minDepth, 0.0f,
            _left + (_width / 2.0f), _top + (_height / 2.0f), _minDepth, 1.0f});
}

} // namespace OriGine
