#pragma once

/// stl
#include <functional>
#include <stdexcept>
#include <string>
/// math
#include <cmath>

namespace OriGine {

/// ==========================================
// 数列
/// ==========================================

enum class CommonSequenceType {
    Arithmetic, // 等差数列
    Geometric // 等比数列
};

/// <summary>
/// 等差数列
/// </summary>
/// <typeparam name="T">数値型</typeparam>
/// <param name="base">初期値</param>
/// <param name="commonDifference">公差</param>
/// <param name="n">index</param>
/// <returns></returns>
template <typename T>
T ArithmeticSequence(T base, T commonDifference, int index) {
    return base + (index - 1) * commonDifference;
}

/// <summary>
/// 等比数列
/// </summary>
/// <typeparam name="T">数値型</typeparam>
/// <param name="base">初期値</param>
/// <param name="commonRatio">公比</param>
/// <param name="index">index</param>
/// <returns>第n項の値</returns>
template <typename T>
T GeometricSequence(T base, T commonRatio, int index) {
    return base * pow(commonRatio, T(index - 1));
}

/// <summary>
/// 等差または等比数列の第n項を取得
/// </summary>
/// <typeparam name="T">数値型</typeparam>
/// <param name="type">数列の種類</param>
/// <param name="base">初期値</param>
/// <param name="commonValue">公差または公比</param>
/// <param name="index">index</param>
/// <returns>第n項の値</returns>
template <typename T>
T CommonSequence(CommonSequenceType type, T base, T commonValue, int index) {
    switch (type) {
    case CommonSequenceType::Arithmetic:
        return ArithmeticSequence(base, commonValue, index);
    case CommonSequenceType::Geometric:
        return GeometricSequence(base, commonValue, index);
    default:
        throw std::invalid_argument("Unsupported sequence type");
    }
}

/// <summary>
/// フィボナッチ数列
/// </summary>
/// <typeparam name="T">数値型</typeparam>
/// <param name="index">index</param>
/// <returns></returns>
template <typename T>
T FibonacciSequence(int index) {
    if (index <= 0)
        return 0;
    if (index == 1)
        return 1;
    T a = 0, b = 1, c;
    for (int i = 2; i <= index; ++i) {
        c = a + b;
        a = b;
        b = c;
    }
    return b;
}

} // namespace OriGine

namespace std {
/// <summary>
/// 数列タイプを文字列へ変換
/// </summary>
string to_string(OriGine::CommonSequenceType type);
}
