#pragma once

#include "asset/ModelAsset.h"
#include "IAssetLoader.h"

namespace OriGine {

/// <summary>
/// assimp を用いてモデルファイル（obj / gltf / glb / fbx 等）を読み込むローダー.
/// 頂点・インデックス・ノード階層・スケルトン・スキンクラスタ・既定マテリアルを構築する.
/// </summary>
class ModelLoader
    : public IAssetLoader<ModelAsset> {
public:
    ModelLoader()           = default;
    ~ModelLoader() override = default;

    /// <summary>
    /// モデルファイルを読み込む.
    /// </summary>
    /// <param name="_assetPath">読み込むモデルファイルのパス</param>
    /// <returns>読み込まれたモデルアセット（失敗時は空のアセット）</returns>
    ModelAsset LoadAsset(const std::string& _assetPath) override;
};

} // namespace OriGine
