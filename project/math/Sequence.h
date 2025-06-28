#pragma once

#include <functional>

/// ==========================================
// 数列
/// ==========================================

/// <summary>
/// 等差数列
/// </summary>
/// <typeparam name="T">数値型</typeparam>
/// <param name="_base">初期値</param>
/// <param name="_commonDifference">公差</param>
/// <param name="n">index</param>
/// <returns></returns>
template <typename T>
T ArithmeticSequence(T _base, T _commonDifference, int _index) {
    return _base + (_index - 1) * _commonDifference;
}

/// <summary>
/// 等比数列
/// </summary>
/// <typeparam name="T">数値型</typeparam>
/// <param name="_base">初期値</param>
/// <param name="_commonRatio">公比</param>
/// <param name="_index">index</param>
template <typename T>
T GeometricSequence(T _base, T _commonRatio, int _index) {
    return _base * pow(_commonRatio, _index - 1);
}

template <typename T>
T DifferenceSequence(T _base, std::function<T(int)> _generalTerm, int _index) {
    T result = _base;
    for (int i = 1; i < _index; ++i) {
        result += _generalTerm(i);
    }
    return result;
}

/// <summary>
/// フィボナッチ数列
/// </summary>
/// <typeparam name="T">数値型</typeparam>
/// <param name="_index">index</param>
/// <returns></returns>
template <typename T>
T FibonacciSequence(int _index) {
    if (_index <= 0)
        return 0;
    if (_index == 1)
        return 1;
    T a = 0, b = 1, c;
    for (int i = 2; i <= _index; ++i) {
        c = a + b;
        a = b;
        b = c;
    }
    return b;
}
