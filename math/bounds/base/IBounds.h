#pragma once

/// stl
#include <concepts>

/// <summary>
/// 数学的境界形状群
/// </summary>
namespace OriGine {
namespace Bounds {

/// <summary>
/// 境界形状インターフェース
/// </summary>
struct IBounds {
    virtual ~IBounds() {}
};

template <typename T>
concept IsBounds = ::std::derived_from<T, IBounds>;

} // namespace Math
} // namespace OriGine
