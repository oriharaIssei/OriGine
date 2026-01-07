#pragma once

/// stl
#include <concepts>
#include <type_traits>

namespace OriGine {

/// <summary>
/// 内部宣言で ConstantBuffer 型を持つことを要求するコンセプト
/// </summary>
template <typename T>
concept HasInConstantBuffer = requires {
    typename T::ConstantBuffer;
    requires ::std::is_copy_assignable_v<typename T::ConstantBuffer>;
    { ::std::declval<typename T::ConstantBuffer>() = ::std::declval<const T&>() } -> ::std::same_as<typename T::ConstantBuffer&>;
};

}
