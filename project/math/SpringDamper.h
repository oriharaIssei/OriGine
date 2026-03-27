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

    static T Magnitude(T v) {
        return std::abs(v);
    }

    static T ClampMagnitude(T v, T max) {
        return (std::min)((std::max)(v, -max), max);
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

    static T Magnitude(const VecT& v) {
        return v.length();
    }

    static VecT ClampMagnitude(const VecT& v, T max) {
        T len = v.lengthSq();
        if (len > max * max) {
            return v.normalize() * max;
        }
        return v;
    }
};

/// <summary>
/// from から to へ、currentVelocity を考慮しながら、smoothTime の時間をかけて移動させる。
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="from">移動前</param>
/// <param name="to">移動後(目標であって実際にこの数値になるとは限らない)</param>
/// <param name="currentVelocity"></param>
/// <param name="smoothTime"></param>
/// <param name="deltaTime"></param>
/// <param name="maxSpeed"></param>
/// <returns></returns>
template <typename T>
T SmoothDamp(
    const T& from,
    const T& to,
    T& currentVelocity,
    typename DampTraits<T>::Scalar smoothTime,
    typename DampTraits<T>::Scalar deltaTime,
    typename DampTraits<T>::Scalar maxSpeed) {
    using Traits = DampTraits<T>;
    using Scalar = DampTraits<T>::Scalar;
    // e^(-x) を近似するための係数 パデ近似というらしい
    constexpr Scalar kExpCoeff2 = static_cast<Scalar>(0.48);
    constexpr Scalar kExpCoeff3 = static_cast<Scalar>(0.235);

    smoothTime = (std::max)(Scalar(kEpsilon), smoothTime);

    Scalar omega = Scalar(2) / smoothTime;
    Scalar x     = omega * deltaTime;
    Scalar exp   = Scalar(1) / (Scalar(1) + x + kExpCoeff2 * x * x + kExpCoeff3 * x * x * x);

    T change = to - from;

    Scalar maxChange = maxSpeed * smoothTime;
    change           = Traits::ClampMagnitude(change, maxChange);

    T temp           = (currentVelocity - omega * change) * deltaTime;
    currentVelocity = (currentVelocity - omega * temp) * exp;

    return from + (change + temp) * exp;
}

} // namespace OriGine
