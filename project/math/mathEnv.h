#pragma once

/// =========================================================
// 数学系環境 ヘッダ
/// =========================================================

// 標準ライブラリ
#include <cmath>
#include <cstdint>
#include <limits>
#include <numbers>

/// ----------------------------------------------
/// 色チャンネル
/// ----------------------------------------------
enum class ColorChannel {
    R = 0,
    G,
    B,
    A,
    MAX
};

/// ----------------------------------------------
/// 円周率関連
/// ----------------------------------------------
constexpr float kPi        = std::numbers::pi_v<float>;
constexpr float kHalfPi    = std::numbers::pi_v<float> * 0.5f;
constexpr float kQuarterPi = std::numbers::pi_v<float> * 0.25f;
constexpr float kTao       = std::numbers::pi_v<float> * 2.0f; // 2π

constexpr float kDeg2Rad = kPi / 180.0f;
constexpr float kRad2Deg = 180.0f / kPi;

/// ----------------------------------------------
/// 数値的な閾値や境界
/// ----------------------------------------------
constexpr float kEpsilon     = 1.0e-6f;
constexpr float kSmallNumber = 1.0e-8f;
constexpr float kHugeNumber  = 1.0e+8f;
constexpr float kInfinity    = std::numeric_limits<float>::infinity();
constexpr float kNegInfinity = -std::numeric_limits<float>::infinity();

/// ----------------------------------------------
/// 単位・汎用値
/// ----------------------------------------------
constexpr float kOneThird  = 1.0f / 3.0f;
constexpr float kTwoThirds = 2.0f / 3.0f;
constexpr float kSqrt2     = std::numbers::sqrt2_v<float>;
constexpr float kInvSqrt2  = 1.0f / std::numbers::sqrt2_v<float>;
constexpr float kSqrt3     = std::numbers::sqrt3_v<float>;
constexpr float kInvSqrt3  = 1.0f / std::numbers::sqrt3_v<float>;

/// ----------------------------------------------
/// 角度単位
/// ----------------------------------------------
constexpr float kRightAngle    = kHalfPi; // 90度
constexpr float kStraightAngle = kPi; // 180度
constexpr float kFullAngle     = kTao; // 360度

/// <summary>
/// Radian を Degree に変換
/// </summary>
/// <param name="radian"></param>
/// <returns></returns>
float radianToDegree(float radian) {
    return radian * kRad2Deg;
}
/// <summary>
/// Degree を Radian に変換
/// </summary>
float degreeToRadian(float degree) {
    return degree * kDeg2Rad;
}


/// ----------------------------------------------
/// 軸列挙（Transformなどと独立して）
enum class Axis : uint8_t {
    X = 0,
    Y,
    Z,
    MAX
};

/// ----------------------------------------------
/// 回転方向
/// ----------------------------------------------
enum class RotationOrder {
    XYZ,
    XZY,
    YXZ,
    YZX,
    ZXY,
    ZYX
};

/// ----------------------------------------------
/// 比較やClampなどに使う補助
/// ----------------------------------------------
constexpr float kMinNormalizedFloat = -1.0f;
constexpr float kMaxNormalizedFloat = 1.0f;
constexpr float kUnitLength         = 1.0f;

/// ----------------------------------------------
/// 乱数・ノイズ系で使う共通種（任意）
/// ----------------------------------------------
constexpr uint32_t kDefaultSeed = 0x12345678u;
