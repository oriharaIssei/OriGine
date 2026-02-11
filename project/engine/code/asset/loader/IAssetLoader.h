#pragma once

/// stl
#include <string>

/// engine
// asset
#include "asset/Asset.h"

namespace OriGine {

/// <summary>
/// Asset の読み込みを担当するインターフェース
/// </summary>
template <IsAsset T>
class IAssetLoader {
    using AssetType = typename AssetTraits<T>::type;

public:
    IAssetLoader()          = default;
    virtual ~IAssetLoader() = default;

    virtual void Initialize() {}
    virtual void Finalize() {}

    /// <summary>
    /// アセットを読み込む.
    /// </summary>
    /// <param name="_assetPath"></param>
    /// <returns></returns>
    virtual T LoadAsset(const std::string& _assetPath) = 0;
};

} // namespace OriGine
