#pragma once

/// stl
#include <concepts>


/// <summary>
/// 数学的境界形状群
/// </summary>
namespace math::bounds {

struct IBounds {
    virtual ~IBounds() {}
};

template <typename T>
concept IsBounds = std::derived_from<T, IBounds>;

}
