#include "Matrix4x4.h"

/// stl
// assert
#include "assert.h"

/// math
#include "Quaternion.h"

#include <cmath>

namespace OriGine {

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
    auto thisMat    = this->matrixToXMMATRIX();
    auto anotherMat = another.matrixToXMMATRIX();

    DirectX::XMMATRIX result = DirectX::XMMatrixMultiply(thisMat, anotherMat);

    return Matrix4x4::XMMATRIXToMatrix(result);
}

Matrix4x4 Matrix4x4::operator*(const float& scalar) const {
    DirectX::XMMATRIX thisMat   = this->matrixToXMMATRIX();
    DirectX::XMMATRIX resultMat = DirectX::XMMatrixMultiply(thisMat, DirectX::XMMatrixScaling(scalar, scalar, scalar));

    return XMMATRIXToMatrix(resultMat);
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
    DirectX::XMMATRIX thisMat = matrixToXMMATRIX();
    DirectX::XMVECTOR det     = DirectX::XMMatrixDeterminant(thisMat);
    Matrix4x4 inverse;
    inverse.xmmatrixToMatrix(DirectX::XMMatrixInverse(&det, thisMat));
    return inverse;
}
Matrix4x4 Matrix4x4::Inverse(const Matrix4x4& m) {
    DirectX::XMMATRIX thisMat = m.matrixToXMMATRIX();
    DirectX::XMVECTOR det     = DirectX::XMMatrixDeterminant(thisMat);
    Matrix4x4 inverse;
    inverse.xmmatrixToMatrix(DirectX::XMMatrixInverse(&det, thisMat));
    return inverse;
}
void Matrix4x4::ToFloatArray(const Matrix4x4& mat, float out[16]) {
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            out[col + row * 4] = mat.m[row][col]; // 行優先
        }
    }
}
void Matrix4x4::FromFloatArray(Matrix4x4& mat, const float in[16]) {
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            mat.m[row][col] = in[col + row * 4];
        }
    }
}

void Matrix4x4::DecomposeMatrixToComponents(const Matrix4x4& mat, Vec3f& outScale, Quaternion& outRotate, Vec3f& outTranslate) {
    DirectX::XMMATRIX xmMat = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&mat));
    DirectX::XMVECTOR scale, rotQuat, trans;
    if (XMMatrixDecompose(&scale, &rotQuat, &trans, xmMat)) {
        DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&outScale), scale);
        DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&outTranslate), trans);
        DirectX::XMFLOAT4 q;
        XMStoreFloat4(&q, rotQuat);
        outRotate = Quaternion(q.x, q.y, q.z, q.w);
    } else {
        // 失敗時は単位値をセット
        outScale     = Vec3f(1, 1, 1);
        outRotate    = Quaternion(0, 0, 0, 1);
        outTranslate = Vec3f(0, 0, 0);
    }
}

Quaternion Matrix4x4::DecomposeMatrixToQuaternion(const Matrix4x4& mat) {
    Quaternion result;
    DirectX::XMMATRIX xmMat = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&mat));
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

Matrix4x4 MakeMatrix4x4::Translate(const Vec3f& vec) {
    return Matrix4x4({1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, vec[X], vec[Y], vec[Z], 1.0f});
}

Matrix4x4 MakeMatrix4x4::Scale(const Vec3f& vec) {
    return Matrix4x4(
        {vec[X], 0.0f, 0.0f, 0.0f, 0.0f, vec[Y], 0.0f, 0.0f, 0.0f, 0.0f, vec[Z], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
}

Matrix4x4 MakeMatrix4x4::RotateX(const float& radian) {
    return Matrix4x4({01.0f, .0f, 0.0f, 0.0f, 0.0f, std::cosf(radian), std::sinf(radian), 0.0f, 0.0f, -std::sinf(radian), std::cosf(radian), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
}

Matrix4x4 MakeMatrix4x4::RotateY(const float& radian) {
    return Matrix4x4({std::cosf(radian), 0.0f, -std::sinf(radian), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, std::sinf(radian), 0.0f, std::cosf(radian), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
}

Matrix4x4 MakeMatrix4x4::RotateZ(const float& radian) {
    return Matrix4x4({std::cosf(radian), std::sinf(radian), 0.0f, 0.0f, -std::sinf(radian), std::cosf(radian), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
}

/// <summary>
/// XYZ各軸の回転角(ラジアン)から回転行列を作る。
/// 行列の積は順序で結果が変わるため、X→Y→Zの順に適用している。
/// </summary>
Matrix4x4 MakeMatrix4x4::RotateXYZ(const Vec3f& radian) {
    return MakeMatrix4x4::RotateX(radian[X]) * MakeMatrix4x4::RotateY(radian[Y]) * MakeMatrix4x4::RotateZ(radian[Z]);
}

/// <summary>
/// 各軸の回転行列を合成する。
/// </summary>
/// <remarks>
/// NOTE: 合成順が z * x * y であり、角度から作る上のRotateXYZ(X→Y→Zの順)とは一致しない。
///       同じ角度を渡しても両者の結果は別の姿勢になるため、混在させないこと。
/// </remarks>
Matrix4x4 MakeMatrix4x4::RotateXYZ(const Matrix4x4& x, const Matrix4x4& y, const Matrix4x4& z) {
    return z * x * y;
}

/// <summary>
/// クォータニオンを回転行列に変換する。
/// </summary>
/// <remarks>
/// q * v * q^-1 のサンドイッチ積を展開して整理すると、各要素が成分同士の積だけで
/// 表せる形になる。積の組み合わせ(xy, wz, x2 など)を先に計算して使い回すことで、
/// 同じ乗算を何度も行わずに済ませている。
/// qが正規化されている前提の式で、そうでない場合はスケールが掛かった行列になる。
/// </remarks>
Matrix4x4 MakeMatrix4x4::RotateQuaternion(const Quaternion& q) {
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
        {(w2 + x2 - y2 - z2), 2.0f * (xy + wz), 2.0f * (xz - wy), 0.0f,
            2.0f * (xy - wz), (w2 - x2 + y2 - z2), 2.0f * (yz + wx), 0.0f,
            2.0f * (xz + wy), 2.0f * (yz - wx), (w2 - x2 - y2 + z2), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f});
}

/// <summary>
/// 任意軸まわりの回転行列を作る(ロドリゲスの回転公式)。
/// axisは正規化されている必要がある。
/// </summary>
Matrix4x4 MakeMatrix4x4::RotateAxisAngle(const Vec3f& axis, float angle) {
    float sinAngle  = sinf(angle);
    float cosAngle  = cosf(angle);
    // (1 - cosθ) は公式中に繰り返し現れるため先に求めておく
    float mCosAngle = (1.0f - cosAngle);
    return {
        axis[X] * axis[X] * mCosAngle + cosAngle, axis[X] * axis[Y] * mCosAngle + axis[Z] * sinAngle, axis[X] * axis[Z] * mCosAngle - axis[Y] * sinAngle, 0.0f, axis[X] * axis[Y] * mCosAngle - axis[Z] * sinAngle, axis[Y] * axis[Y] * mCosAngle + cosAngle, axis[Y] * axis[Z] * mCosAngle + axis[X] * sinAngle, 0.0f, axis[X] * axis[Z] * mCosAngle + axis[Y] * sinAngle, axis[Y] * axis[Z] * mCosAngle - axis[X] * sinAngle, axis[Z] * axis[Z] * mCosAngle + cosAngle, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
}

/// <summary>
/// fromVの向きをtoVの向きへ合わせる回転行列を作る。
/// </summary>
/// <remarks>
/// WARNING: 内積をclampせずacosに渡しているため、両ベクトルが正規化されていないと
///          定義域外でNaNになる。また平行/真逆の場合は外積がゼロベクトルとなり、
///          normalizeが破綻する。呼び出し側でこれらを保証すること。
/// </remarks>
Matrix4x4 MakeMatrix4x4::RotateAxisAngle(const Vec3f& fromV, const Vec3f& toV) {
    float angle = std::acosf(fromV.dot(toV));
    Vec3f axis  = fromV.cross(toV).normalize();
    return MakeMatrix4x4::RotateAxisAngle(axis, angle);
}

/// <summary>
/// 拡大・回転・平行移動をまとめたアフィン変換行列を作る。
/// </summary>
/// <remarks>
/// 拡大→回転→平行移動の順で掛けること。
/// 回転より先に平行移動すると原点から離れた位置を中心に振り回され、
/// 回転より後に拡大すると軸が傾いた状態で伸びて形が歪む。
/// このエンジンは行ベクトル(v * M)規約なので、適用したい順に左から掛ける。
/// </remarks>
Matrix4x4 MakeMatrix4x4::Affine(const Vec3f& scale, const Vec3f& rotate, const Vec3f& translate) {
    return MakeMatrix4x4::Scale(scale) * MakeMatrix4x4::RotateXYZ(rotate) * MakeMatrix4x4::Translate(translate);
}

Matrix4x4 MakeMatrix4x4::Affine(const Vec3f& scale, const Quaternion& rotate, const Vec3f& translate) {
    return MakeMatrix4x4::Scale(scale) * MakeMatrix4x4::RotateQuaternion(rotate) * MakeMatrix4x4::Translate(translate);
}

Vec3f TransformVector(const Vec3f& vec, const Matrix4x4& matrix) {
    DirectX::XMVECTOR vecXM = DirectX::XMVectorSet(vec[X], vec[Y], vec[Z], 1.0f); // w=1
    DirectX::XMMATRIX matXM = matrix.matrixToXMMATRIX();

    DirectX::XMVECTOR resultXM = DirectX::XMVector4Transform(vecXM, matXM);

    float result[4];
    DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(result), resultXM);

    // 射影変換ではw成分に深度が入るため、xyzをwで割って同次座標から3次元に戻す
    // (この除算がいわゆるパースペクティブ除算で、遠くのものを小さく見せている)。
    // カメラ平面上の点などでwが0になると無限大に発散するため、その場合は原点を返す
    if (result[3] == 0.0f) {
        return Vec3f(0.f, 0.f, 0.f);
    }

    return Vec3f(result[0] / result[3], result[1] / result[3], result[2] / result[3]);
}

/// <summary>
/// 法線ベクトルを変換する。
/// </summary>
/// <remarks>
/// 法線は「位置」ではなく「向き」なので、平行移動を適用してはいけない。
/// 左上3x3の部分だけを使うことで、回転と拡大のみを反映させている。
/// (位置と同じように平行移動まで掛けると、原点からの距離だけ法線がずれてしまう)
/// </remarks>
Vec3f TransformNormal(const Vec3f& v, const Matrix4x4& m) {
    // 平行移動を無視して計算
    Vec3f result = {
        v[X] * m.m[0][0] + v[Y] * m.m[1][0] + v[Z] * m.m[2][0],
        v[X] * m.m[0][1] + v[Y] * m.m[1][1] + v[Z] * m.m[2][1],
        v[X] * m.m[0][2] + v[Y] * m.m[1][2] + v[Z] * m.m[2][2],
    };

    return result;
}

Vec2f WorldToScreen(const Vec3f& worldPos, const Matrix4x4& vpvpvMat) {
    // ワールド座標をビュー変換
    // ビュー空間からスクリーン空間へ変換
    Vec3f screenSpace = TransformVector(worldPos, vpvpvMat);

    return Vec2f(screenSpace[X], screenSpace[Y]);
}

Vec3f ScreenToWorld(const Vec2f& screenPos, const float& depth, const Matrix4x4& invVpvpMat) {
    // スクリーン座標を3Dベクトルに拡張
    Vec3f screenPos3D = Vec3f(screenPos[X], screenPos[Y], depth);
    // スクリーン座標からワールド座標へ変換
    Vec3f worldPos = TransformVector(screenPos3D, invVpvpMat);
    return worldPos;
}

/// <summary>
/// 透視投影行列を作る。
/// </summary>
/// <remarks>
/// DirectX規約(左手系・深度0〜1)の射影行列。
/// [2][3]に1.0が入っているのが要点で、これにより変換後のw成分にビュー空間のZ(奥行き)が
/// コピーされ、後段のパースペクティブ除算で遠くのものほど小さく描かれる。
/// nearClipとfarClipが同値だと分母が0になるため、必ず nearClip &lt; farClip とすること。
/// </remarks>
/// <param name="fovY">垂直方向の視野角(ラジアン)</param>
Matrix4x4 MakeMatrix4x4::PerspectiveFov(const float& fovY, const float& aspectRatio, const float& nearClip, const float& farClip) {
    // 視野角の半分のコタンジェント。視野角が広いほど小さくなり、頂点が中心寄りに圧縮される
    const float cot = 1.0f / std::tanf(fovY / 2.0f);
    return Matrix4x4(
        {(1.0f / aspectRatio) * cot, 0.0f, 0.0f, 0.0f, 0.0f, cot, 0.0f, 0.0f, 0.0f, 0.0f, farClip / (farClip - nearClip), 1.0f, 0.0f, 0.0f, (-nearClip * farClip) / (farClip - nearClip), 0.0f});
}

Matrix4x4 MakeMatrix4x4::Orthographic(const float& left, const float& top, const float& right, const float& bottom, const float& nearClip, const float& farClip) {
    return Matrix4x4(
        {2.0f / (right - left), 0.0f, 0.0f, 0.0f, 0.0f, 2.0f / (top - bottom), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f / (farClip - nearClip), 0.0f, (left + right) / (left - right), (top + bottom) / (bottom - top), nearClip / (nearClip - farClip), 1.0f});
}

/// <summary>
/// 正規化デバイス座標(-1〜1)をスクリーンのピクセル座標へ移すビューポート行列を作る。
/// </summary>
/// <remarks>
/// Y成分のスケールだけ符号が負なのは、NDCがY上向きなのに対し
/// スクリーン座標は左上原点でY下向きだから。ここで上下を反転させないと画面が逆さになる。
/// </remarks>
Matrix4x4 MakeMatrix4x4::ViewPort(const float& left, const float& top, const float& width, const float& height, const float& minDepth, const float& maxDepth) {
    return Matrix4x4(
        {width / 2.0f, 0.0f, 0.0f, 0.0f,
            0.0f, -(height / 2.0f), 0.0f, 0.0f,
            0.0f, 0.0f, maxDepth - minDepth, 0.0f,
            left + (width / 2.0f), top + (height / 2.0f), minDepth, 1.0f});
}

} // namespace OriGine
