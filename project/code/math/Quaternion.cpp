#include "Quaternion.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numbers>

#pragma region "Operators"
Quaternion Quaternion::operator+(const Quaternion& q) const {
    return {
        x + q.x,
        y + q.y,
        z + q.z,
        w + q.w,
    };
}

Quaternion Quaternion::operator-(const Quaternion& q) const {
    return {
        x - q.x,
        y - q.y,
        z - q.z,
        w - q.w,
    };
}

Quaternion Quaternion::operator*(const Quaternion& q) const {
    return {
        w * q.x + x * q.w + y * q.z - z * q.y, // x'
        w * q.y + y * q.w + z * q.x - x * q.z, // y'
        w * q.z + z * q.w + x * q.y - y * q.x, // z'
        w * q.w - x * q.x - y * q.y - z * q.z  // w'
    };
}

Quaternion Quaternion::operator*(float scalar) const {
    return Quaternion(
        x * scalar,
        y * scalar,
        z * scalar,
        w * scalar);
}

Quaternion Quaternion::operator/(float scalar) const {
    return Quaternion(
        this->x / scalar,
        this->y / scalar,
        this->z / scalar,
        this->w / scalar);
}

Quaternion* Quaternion::operator=(const Quaternion& q) {
    this->x = q.x;
    this->y = q.y;
    this->z = q.z;
    this->w = q.w;
    return this;
}

Quaternion* Quaternion::operator+=(const Quaternion& q) {
    this->x += q.x;
    this->y += q.y;
    this->z += q.z;
    this->w += q.w;
    return this;
}

Quaternion* Quaternion::operator*=(const Quaternion& q) {
    this->x *= q.x;
    this->y *= q.y;
    this->z *= q.z;
    this->w *= q.w;
    return this;
}

Quaternion* Quaternion::operator*=(float scalar) {
    this->x *= scalar;
    this->y *= scalar;
    this->z *= scalar;
    this->w *= scalar;
    return this;
}

Quaternion* Quaternion::operator/=(float scalar) {
    this->x /= scalar;
    this->y /= scalar;
    this->z /= scalar;
    this->w /= scalar;
    return this;
}

Quaternion operator*(float scalar, const Quaternion& q) {
    return {
        q.x * scalar,
        q.y * scalar,
        q.z * scalar,
        q.w * scalar};
}

Quaternion operator/(float scalar, const Quaternion& q) {
    return {
        q.x / scalar,
        q.y / scalar,
        q.z / scalar,
        q.w / scalar};
}
#pragma endregion

Quaternion Quaternion::Inverse(const Quaternion& q) {
    float normSq = q.normSq(); // ノルムの二乗
    if (normSq == 0.0f) {
        // ノルムが0の場合、逆元は存在しない
        return {0.0f, 0.0f, 0.0f, 0.0f}; // エラー処理としてゼロ四元数を返す
    }
    Quaternion conjugate = q.Conjugation();
    return conjugate / normSq; // 共役をノルムの二乗で割る
}

Quaternion Quaternion::inverse() const {
    float normSq = this->normSq(); // ノルムの二乗
    if (normSq == 0.0f) {
        // ノルムが0の場合、逆元は存在しない
        return {0.0f, 0.0f, 0.0f, 0.0f}; // エラー処理としてゼロ四元数を返す
    }
    Quaternion conjugate = this->Conjugation();
    return conjugate / normSq; // 共役をノルムの二乗で割る
}

float Quaternion::Norm(const Quaternion& q) {
    return sqrtf(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
}

float Quaternion::norm() const {
    return sqrtf(this->w * this->w + this->x * this->x + this->y * this->y + this->z * this->z);
}

float Quaternion::NormSq(const Quaternion& q) {
    return q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z;
}

float Quaternion::normSq() const {
    return this->w * this->w + this->x * this->x + this->y * this->y + this->z * this->z;
}

Quaternion Quaternion::Normalize(const Quaternion& q) {
    float norm = q.norm();
    if (norm == 0.0f) {
        return {0.0f, 0.0f, 0.0f, 1.0f};
    }
    return q / norm;
}

Quaternion Quaternion::normalize() const {
    float norm = this->norm();
    if (norm == 0.0f) {
        return {0.0f, 0.0f, 0.0f, 1.0f};
    }
    return *this / norm;
}

float Quaternion::Dot(const Quaternion& q0, const Quaternion& q1) {
    return q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w;
}

float Quaternion::dot(const Quaternion& q) const {
    return x * q.x + y * q.y + z * q.z + w * q.w;
}

Quaternion Quaternion::RotateAxisAngle(const Vec3f& axis, float angle) {
    float halfAngle = angle / 2.0f;
    return Quaternion(
        axis * sinf(halfAngle),
        cosf(halfAngle));
}

void Quaternion::Show() {
    std::cout << std::fixed << std::setprecision(3) << x << " ";
    std::cout << std::fixed << std::setprecision(3) << y << " ";
    std::cout << std::fixed << std::setprecision(3) << z << " ";
    std::cout << std::fixed << std::setprecision(3) << w << " ";
    std::cout << "\n"
              << std::endl;
}

Vec3f Quaternion::ToEulerAngles() const {
    Vec3f euler;

    // Roll (x-axis rotation)
    float sinr_cosp = 2.0f * (w * x + y * z);
    float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
    euler[X]        = std::atan2(sinr_cosp, cosr_cosp);

    // Pitch (y-axis rotation)
    float sinp = 2.0f * (w * y - z * x);
    if (std::abs(sinp) >= 1.0f)
        euler[Y] = std::copysign(std::numbers::pi_v<float> / 2.0f, sinp); // use 90 degrees if out of range
    else
        euler[Y] = std::asin(sinp);

    // Yaw (z-axis rotation)
    float siny_cosp = 2.0f * (w * z + x * y);
    float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
    euler[Z]        = std::atan2(siny_cosp, cosy_cosp);

    return euler;
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
        return (q0 * (1.0f - t) + q1Adjusted * t).normalize(); // 線形補間を用いる
    }

    float theta    = acosf(dot);
    float sinTheta = sinf(theta);

    float scale0 = sinf((1.0f - t) * theta) / sinTheta;
    float scale1 = sinf(t * theta) / sinTheta;

    return (q0 * scale0 + q1Adjusted * scale1).normalize(); // 結果を正規化
}
