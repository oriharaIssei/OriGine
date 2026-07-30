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

    /// <summary>
    /// バリアントを指定してアセットを読み込む.
    ///
    /// 同じファイルからパラメータ違いのアセットを生成するローダー
    /// （シェーダをプロファイル別にコンパイルする等）はこちらを override する.
    /// 既定実装はバリアントを無視して通常の読み込みを行う.
    /// </summary>
    /// <param name="_assetPath"></param>
    /// <param name="_variant">バリアント識別子（空文字ならバリアント無し）</param>
    /// <returns></returns>
    virtual T LoadAsset(const std::string& _assetPath, const std::string& _variant) {
        (void)_variant;
        return LoadAsset(_assetPath);
    }
};

} // namespace OriGine
