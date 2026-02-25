#pragma once

/// stl
#include <algorithm>
#include <concepts>

/// math
#include "MathEnv.h"
#include "Vector.h"

namespace OriGine {

/// <summary
/// DampTraits<T> を定義することで、SmoothDamp 関数を T 型に対して使用できるようになる。
/// Magnitude と ClampMagnitude を定義する必要がある。
/// </summary>
/// <typeparam name="T"></typeparam>
template <typename T>
struct DampTraits {
};

/// <summary>
/// DampTraits<T> の特殊化。T が浮動小数点数型の場合、Magnitude と ClampMagnitude を定義する。
/// </summary>
/// <typeparam name="T"></typeparam>
template <std::floating_point T>
struct DampTraits<T> {

    using Scalar = T;

    static T Magnitude(T _v) {
        return std::abs(_v);
    }

    static T ClampMagnitude(T _v, T _max) {
        return (std::min)((std::max)(_v, -_max), _max);
    }
};

/// <summary>
/// DampTraits<T> の特殊化。Magnitude と ClampMagnitude を定義する。
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="dimension"></typeparam>
template <int dimension, typename T>
struct DampTraits<Vector<dimension, T>> {
    using VecT = Vec<dimension, T>;

    using Scalar = T;

    static T Magnitude(const VecT& _v) {
        return _v.length();
    }

    static VecT ClampMagnitude(const VecT& _v, T _max) {
        T len = _v.lengthSq();
        if (len > _max * _max) {
            return _v.normalize() * _max;
        }
        return _v;
    }
};

/// <summary>
/// _from から _to へ、currentVelocity を考慮しながら、smoothTime の時間をかけて移動させる。
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="_from">移動前</param>
/// <param name="_to">移動後(目標であって実際にこの数値になるとは限らない)</param>
/// <param name="_currentVelocity"></param>
/// <param name="_smoothTime"></param>
/// <param name="_deltaTime"></param>
/// <param name="_maxSpeed"></param>
/// <returns></returns>
template <typename T>
T SmoothDamp(
    const T& _from,
    const T& _to,
    T& _currentVelocity,
    typename DampTraits<T>::Scalar _smoothTime,
    typename DampTraits<T>::Scalar _deltaTime,
    typename DampTraits<T>::Scalar _maxSpeed) {
    using Traits = DampTraits<T>;
    using Scalar = DampTraits<T>::Scalar;
    // e^(-x) を近似するための係数 パデ近似というらしい
    constexpr Scalar kExpCoeff2 = static_cast<Scalar>(0.48);
    constexpr Scalar kExpCoeff3 = static_cast<Scalar>(0.235);

    _smoothTime = (std::max)(Scalar(kEpsilon), _smoothTime);

    Scalar omega = Scalar(2) / _smoothTime;
    Scalar x     = omega * _deltaTime;
    Scalar exp   = Scalar(1) / (Scalar(1) + x + kExpCoeff2 * x * x + kExpCoeff3 * x * x * x);

    T change = _to - _from;

    Scalar maxChange = _maxSpeed * _smoothTime;
    change           = Traits::ClampMagnitude(change, maxChange);

    T temp           = (_currentVelocity - omega * change) * _deltaTime;
    _currentVelocity = (_currentVelocity - omega * temp) * exp;

    return _from + (change + temp) * exp;
}

} // namespace OriGine
