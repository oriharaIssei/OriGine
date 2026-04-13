#pragma once

/// stl
#include <array>
#include <cassert>
#include <concepts>
#include <string>

namespace OriGine {

/// <summary>
/// Interface
/// </summary>
struct Asset {
    virtual ~Asset() = default;

    std::string path; // Asset file path
};

/// <summary>
/// Asset 型のコンセプト.
/// </summary>
template <typename T>
concept IsAsset = std::derived_from<T, Asset>;

/// <summary>
/// Asset 型に関する特性情報を提供するテンプレート構造体.
/// </summary>
/// <typeparam name="T"></typeparam>
template <IsAsset T>
struct AssetTraits {
    using type = T;

    /// <summary>
    /// 対応するファイル拡張子の配列を取得する.
    /// </summary>
    /// <returns></returns>
    static constexpr std::array<std::string, 1> Extensions() {
        assert(false && "Extensions() not specialized for this Asset type.");
        return {};
    }
};

} // namespace OriGine
