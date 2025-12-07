#include "Quaternion.h"

/// stl
#include <algorithm>

/// math
#include <cmath>
#include <Matrix4x4.h>
#include <numbers>

Quaternion Quaternion::Inverse(const Quaternion& q) {
    float normSq = q.normSq(); // ノルムの二乗
    if (normSq == 0.0f) {
        // ノルムが0の場合、逆元は存在しない
        return {0.0f, 0.0f, 0.0f, 0.0f}; // エラー処理としてゼロ四元数を返す
    }
    Quaternion conjugate = q.Conjugation();
    return Quaternion(conjugate / normSq); // 共役をノルムの二乗で割った結果を明示的にQuaternionへ変換
}

Quaternion Quaternion::inverse() const {
    float normSq = this->normSq(); // ノルムの二乗
    if (normSq == 0.0f) {
        // ノルムが0の場合、逆元は存在しない
        return {0.0f, 0.0f, 0.0f, 0.0f}; // エラー処理としてゼロ四元数を返す
    }
    Quaternion conjugate = this->Conjugation();
    return Quaternion(conjugate / normSq); // 明示的な変換
}

float Quaternion::Norm(const Quaternion& q) {
    return sqrtf((q[W] * q[W]) + (q[X] * q[X]) + (q[Y] * q[Y]) + (q[Z] * q[Z]));
}

float Quaternion::norm() const {
    return sqrtf((this->v[W] * this->v[W]) + (this->v[X] * this->v[X]) + (this->v[Y] * this->v[Y]) + (this->v[Z] * this->v[Z]));
}

float Quaternion::NormSq(const Quaternion& q) {
    return q[W] * q[W] + q[X] * q[X] + q[Y] * q[Y] + q[Z] * q[Z];
}

float Quaternion::normSq() const {
    return this->v[W] * this->v[W] + this->v[X] * this->v[X] + this->v[Y] * this->v[Y] + this->v[Z] * this->v[Z];
}

Quaternion Quaternion::Normalize(const Quaternion& q) {
    float norm = q.norm();
    if (norm == 0.0f) {
        return {0.0f, 0.0f, 0.0f, 1.0f};
    }
    return Quaternion(q / norm); // 明示的にQuaternionへ変換
}

Quaternion Quaternion::normalize() const {
    float norm = this->norm();
    if (norm == 0.0f) {
        return {0.0f, 0.0f, 0.0f, 1.0f};
    }
    return Quaternion(*this / norm); // 明示的にQuaternionへ変換
}

float Quaternion::Dot(const Quaternion& q0, const Quaternion& q1) {
    return q0[X] * q1[X] + q0[Y] * q1[Y] + q0[Z] * q1[Z] + q0[W] * q1[W];
}

float Quaternion::dot(const Quaternion& q) const {
    return v[X] * q[X] + v[Y] * q[Y] + v[Z] * q[Z] + v[W] * q[W];
}

Vec3f Quaternion::ToEulerAngles() const {
    Vec3f euler;

    // Yaw (Y-axis rotation)
    float siny_cosp = 2.0f * (v[W] * v[Y] + v[X] * v[Z]);
    float cosy_cosp = 1.0f - 2.0f * (v[Y] * v[Y] + v[Z] * v[Z]);
    euler[Y]        = std::atan2(siny_cosp, cosy_cosp);

    // Pitch (X-axis rotation)
    float sinp = 2.0f * (v[W] * v[X] - v[Y] * v[Z]);
    if (std::abs(sinp) >= 1.0f) {
        euler[X] = std::copysign(std::numbers::pi_v<float> / 2.0f, sinp);
    } else {
        euler[X] = std::asin(sinp);
    }

    // Roll (Z-axis rotation)
    float sinr_cosp = 2.0f * (v[W] * v[Z] - v[X] * v[Y]);
    float cosr_cosp = 1.0f - 2.0f * (v[Z] * v[Z] + v[X] * v[X]);
    euler[Z]        = std::atan2(sinr_cosp, cosr_cosp);

    return euler;
}

float Quaternion::ToPitch() const {
    float sinp = 2.0f * (v[W] * v[X] - v[Y] * v[Z]);
    if (std::abs(sinp) >= 1.0f) {
        return std::copysign(std::numbers::pi_v<float> / 2.0f, sinp);
    }
    return std::asin(sinp);
}

float Quaternion::ToYaw() const {
    float siny_cosp = 2.0f * (v[W] * v[Y] + v[X] * v[Z]);
    float cosy_cosp = 1.0f - 2.0f * (v[Y] * v[Y] + v[Z] * v[Z]);
    return std::atan2(siny_cosp, cosy_cosp);
}

float Quaternion::ToRoll() const {
    float sinr_cosp = 2.0f * (v[W] * v[Z] - v[X] * v[Y]);
    float cosr_cosp = 1.0f - 2.0f * (v[Z] * v[Z] + v[X] * v[X]);
    return std::atan2(sinr_cosp, cosr_cosp);
}

Vec3f Quaternion::RotateVector(const Vec3f& vec, const Quaternion& q) {
    Quaternion r = Quaternion(vec, 0.0f);
    r            = q * r * q.Conjugation();
    return Vec3f(r[X], r[Y], r[Z]);
}

Vec3f Quaternion::RotateVector(const Vec3f& vec) const {
    Quaternion r = Quaternion(vec, 0.0f);
    r            = *this * r * this->Conjugation();
    return Vec3f(r[X], r[Y], r[Z]);
}

Quaternion Quaternion::RotateAxisAngle(const Vec3f& axis, float angle) {
    float halfAngle = angle / 2.0f;
    return Quaternion(
        axis * sinf(halfAngle),
        cosf(halfAngle))
        .normalize();
}

const Quaternion Quaternion::RotateAxisVector(const Vec3f& _from, const Vec3f& _to) {
    float angle = std::acosf(_from.dot(_to));
    Vec3f axis  = _from.cross(_to).normalize();

    float halfAngle = angle / 2.0f;
    return Quaternion(
        axis * sinf(halfAngle),
        cosf(halfAngle))
        .normalize();
}

const Quaternion Quaternion::FromNormalVector(const Vec3f& _normal, const Vec3f& _up) {
    Vec3f from = _up.normalize();
    Vec3f to   = _normal.normalize();

    float dot = from.dot(to);
    // 数値誤差対策
    dot = std::clamp(dot, -1.0f, 1.0f);

    if (std::abs(dot - 1.0f) < 1e-6f) {
        // ほぼ同じ方向 → 回転不要
        return Quaternion::Identity();
    }

    if (std::abs(dot + 1.0f) < 1e-6f) {
        // 真逆方向 → 180度回転
        // from と直交する任意の軸を求める
        Vec3f axis = Vec3f::Cross(from, Vec3f(1.0f, 0.0f, 0.0f));
        if (axis.lengthSq() < 1e-6f) {
            // 万一 from が (1,0,0) と平行なら別の軸を選ぶ
            axis = Vec3f::Cross(from, Vec3f(0.0f, 1.0f, 0.0f));
        }
        axis = axis.normalize();
        return Quaternion::RotateAxisAngle(axis, std::numbers::pi_v<float>);
    }

    Vec3f axis  = Vec3f::Cross(from, to).normalize();
    float angle = std::acos(dot);
    return Quaternion::RotateAxisAngle(axis, angle);
}

Quaternion Quaternion::FromMatrix(const Matrix4x4& _rotateMat) {
    float trace = _rotateMat.m[0][0] + _rotateMat.m[1][1] + _rotateMat.m[2][2];

    if (trace > 0.0f) {
        float s    = std::sqrt(trace + 1.0f) * 2.0f;
        float invS = 1.0f / s;

        return Quaternion(
            (_rotateMat.m[2][1] - _rotateMat.m[1][2]) * invS, // x
            (_rotateMat.m[0][2] - _rotateMat.m[2][0]) * invS, // y
            (_rotateMat.m[1][0] - _rotateMat.m[0][1]) * invS, // z
            0.25f * s // w
        );
    } else {
        if (_rotateMat.m[0][0] > _rotateMat.m[1][1] && _rotateMat.m[0][0] > _rotateMat.m[2][2]) {
            float s    = std::sqrt(1.0f + _rotateMat.m[0][0] - _rotateMat.m[1][1] - _rotateMat.m[2][2]) * 2.0f;
            float invS = 1.0f / s;

            return Quaternion(
                0.25f * s,
                (_rotateMat.m[0][1] + _rotateMat.m[1][0]) * invS,
                (_rotateMat.m[0][2] + _rotateMat.m[2][0]) * invS,
                (_rotateMat.m[2][1] - _rotateMat.m[1][2]) * invS);
        } else if (_rotateMat.m[1][1] > _rotateMat.m[2][2]) {
            float s    = std::sqrt(1.0f + _rotateMat.m[1][1] - _rotateMat.m[0][0] - _rotateMat.m[2][2]) * 2.0f;
            float invS = 1.0f / s;

            return Quaternion(
                (_rotateMat.m[0][1] + _rotateMat.m[1][0]) * invS,
                0.25f * s,
                (_rotateMat.m[1][2] + _rotateMat.m[2][1]) * invS,
                (_rotateMat.m[0][2] - _rotateMat.m[2][0]) * invS);
        } else {
            float s    = std::sqrt(1.0f + _rotateMat.m[2][2] - _rotateMat.m[0][0] - _rotateMat.m[1][1]) * 2.0f;
            float invS = 1.0f / s;

            return Quaternion(
                (_rotateMat.m[0][2] + _rotateMat.m[2][0]) * invS,
                (_rotateMat.m[1][2] + _rotateMat.m[2][1]) * invS,
                0.25f * s,
                (_rotateMat.m[1][0] - _rotateMat.m[0][1]) * invS);
        }
    }
}

Quaternion Quaternion::FromEulerAngles(float pitch,float yaw, float roll) {
    // 半分の角度を計算
    float halfPitch = pitch * 0.5f;
    float halfYaw   = yaw * 0.5f;
    float halfRoll  = roll * 0.5f;

    // サインとコサインを計算
    float sinPitch = sin(halfPitch);
    float cosPitch = cos(halfPitch);
    float sinYaw   = sin(halfYaw);
    float cosYaw   = cos(halfYaw);
    float sinRoll  = sin(halfRoll);
    float cosRoll  = cos(halfRoll);

    // クォータニオンを計算
    Quaternion q;
    q[X] = cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll;
    q[Y] = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;
    q[Z] = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
    q[W] = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;

    return q;
}

Quaternion Quaternion::FromEulerAngles(const Vec3f& euler) {
    return FromEulerAngles(euler[X], euler[Y], euler[Z]);
}

Quaternion Quaternion::LookAt(const Vec3f& _forward, const Vec3f& up) { // Z軸を向けるべき方向にする
    Vec3f forward = Vec3f::Normalize(_forward);

    // 右ベクトルを計算（外積）
    Vec3f right = Vec3f::Normalize(Vec3f::Cross(up, forward));

    // 上ベクトルを再計算
    Vec3f newUp = Vec3f::Cross(forward, right);

    // 回転行列を作成
    Matrix4x4 lookAtMatrix = {
        right[X], newUp[X], forward[X], 0.0f,
        right[Y], newUp[Y], forward[Y], 0.0f,
        right[Z], newUp[Z], forward[Z], 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};

    // 行列からクォータニオンに変換
    return FromMatrix(lookAtMatrix);
};

constexpr Quaternion operator*(float scalar, const Quaternion& q) {
    return Quaternion(q * scalar);
}

constexpr Quaternion operator/(float scalar, const Quaternion& q) {
    return Quaternion(q / scalar);
}

Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t) {
    float dot = q0.dot(q1);

    // ドット積が負の場合、q1 を反転して最短経路を取る
    Quaternion q1Adjusted = q1;
    if (dot < 0.0f) {
        q1Adjusted = -q1Adjusted;
        dot        = -dot;
    }

    // θがほぼゼロの場合、直接返す
    if (dot > 0.9995f) {
        return Quaternion(q0 * (1.0f - t) + q1Adjusted * t).normalize(); // 線形補間を用いる
    }

    float theta    = acosf(dot);
    float sinTheta = sinf(theta);

    float scale0 = sinf((1.0f - t) * theta) / sinTheta;
    float scale1 = sinf(t * theta) / sinTheta;

    return Quaternion(q0 * scale0 + q1Adjusted * scale1).normalize(); // 結果を正規化
}
