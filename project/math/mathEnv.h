#pragma once

/// =========================================================
// 数学系環境 ヘッダ
/// =========================================================

/// stl
#include <charconv>
#include <concepts>
#include <string>
#include <system_error>
#include <vector>
/// math
#include <cmath>
#include <cstdint>
#include <limits>
#include <numbers>

namespace OriGine {

/// ----------------------------------------------
/// 円周率関連
/// ----------------------------------------------
constexpr float kPi        = ::std::numbers::pi_v<float>;
constexpr float kHalfPi    = ::std::numbers::pi_v<float> * 0.5f;
constexpr float kQuarterPi = ::std::numbers::pi_v<float> * 0.25f;
constexpr float kTau       = ::std::numbers::pi_v<float> * 2.0f; // 2π

constexpr float kDeg2Rad = kPi / 180.0f;
constexpr float kRad2Deg = 180.0f / kPi;

/// ----------------------------------------------
/// 数値的な閾値や境界
/// ----------------------------------------------
constexpr float kEpsilon     = 1.0e-6f;
constexpr float kSmallNumber = 1.0e-8f;
constexpr float kHugeNumber  = 1.0e+8f;
constexpr float kInfinity    = ::std::numeric_limits<float>::infinity();
constexpr float kNegInfinity = -::std::numeric_limits<float>::infinity();

/// ----------------------------------------------
/// 単位・汎用値
/// ----------------------------------------------
constexpr float kOneThird  = 1.0f / 3.0f;
constexpr float kTwoThirds = 2.0f / 3.0f;
constexpr float kSqrt2     = ::std::numbers::sqrt2_v<float>;
constexpr float kInvSqrt2  = 1.0f / ::std::numbers::sqrt2_v<float>;
constexpr float kSqrt3     = ::std::numbers::sqrt3_v<float>;
constexpr float kInvSqrt3  = 1.0f / ::std::numbers::sqrt3_v<float>;

/// ----------------------------------------------
/// 角度単位
/// ----------------------------------------------
constexpr float kRightAngle    = kHalfPi; // 90度
constexpr float kStraightAngle = kPi; // 180度
constexpr float kFullAngle     = kTau; // 360度

/// <summary>
/// Radian を Degree に変換
/// </summary>
/// <param name="_radian"></param>
/// <returns></returns>
float RadianToDegree(float _radian);
/// <summary>
/// Degree を Radian に変換
/// </summary>
float DegreeToRadian(float _degree);

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
/// utilities
/// ----------------------------------------------

/// <summary>
/// 数値の ”整数部” の桁数を取得（整数型用）
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="_value"></param>
/// <returns></returns>
template <::std::integral T>
T CountIntegralDigits(T _value) {
    if (_value == 0) {
        return 1;
    }
    // 桁数 = log10(絶対値) + 1
    return log10(::std::abs(_value)) + 1;
}

/// <summary>
/// 数値の ”整数部” の桁数を取得（浮動小数点型用）
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="ReturnT"></typeparam>
/// <param name="_value"></param>
/// <returns></returns>
template <::std::floating_point T, ::std::integral ReturnT = size_t>
ReturnT CountIntegralDigits(T _value) {
    if (_value == 0.0f) {
        return 1;
    }
    // 桁数 = floor( log10(絶対値) ) + 1
    return static_cast<ReturnT>(::std::floor(::std::log10(::std::abs(_value)))) + 1;
}

/// <summary>
/// 数値の ”小数部” の桁数を取得（浮動小数点型用）
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="ReturnT"></typeparam>
/// <param name="_value"></param>
/// <returns></returns>
template <::std::floating_point T, ::std::integral ReturnT = size_t>
ReturnT CountDecimalDigits(T _value) {
    constexpr size_t kBufferSize       = 64;
    constexpr size_t kMaxDecimalPlaces = 20;

    // 小数点以下がほぼ 0 の場合 → 0
    T fractionalPart = ::std::abs(_value - static_cast<T>(static_cast<int64_t>(_value)));
    if (fractionalPart < kEpsilon) {
        return 0;
    }

    char buf[kBufferSize];

    // ※ 20 桁まで固定小数で文字列化
    auto [ptr, ec] = ::std::to_chars(buf, buf + sizeof(buf), fractionalPart,
        ::std::chars_format::fixed, kMaxDecimalPlaces);
    if (ec != ::std::errc{}) {
        return 0;
    }

    ::std::string s(buf, ptr);

    // 小数点なし → 0
    auto pos = static_cast<ReturnT>(s.find('.'));
    if (pos == ::std::string::npos) {
        return 0;
    }

    // 小数点以下の末尾 0 を削る
    ReturnT end = static_cast<ReturnT>(s.size());
    while (end > pos + 1 && s[end - 1] == '0') {
        end--;
    }

    return end - (pos + 1);
}

/// <summary>
/// 浮動小数点数から整数部と小数部の各桁の数字を抽出する
/// </summary>
/// <param name="_value">変換前</param>
/// <param name="_intDigits">整数部の桁数</param>
/// <param name="_fracDigits">小数部の桁数</param>
/// <returns></returns>
::std::vector<int> CalculateDigitsFromFloat(float _value, int _intDigits, int _fracDigits);

} // namespace OriGine
