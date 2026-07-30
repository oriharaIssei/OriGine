#pragma once

#include "asset/AnimationAsset.h"
#include "IAssetLoader.h"

namespace OriGine {

/// <summary>
/// gltf / glb 等、assimp が解釈できるモデルファイルからアニメーションを読み込むローダー.
/// </summary>
class GltfAnimationLoader
    : public IAssetLoader<AnimationAsset> {
public:
    GltfAnimationLoader()           = default;
    ~GltfAnimationLoader() override = default;

    AnimationAsset LoadAsset(const std::string& _assetPath) override;
};

/// <summary>
/// 独自バイナリ形式 (.anm) のアニメーションを読み込むローダー.
/// 書き出しは AnimationSerializer::Save が担当する.
/// </summary>
class MyAnimationLoader
    : public IAssetLoader<AnimationAsset> {
public:
    MyAnimationLoader()           = default;
    ~MyAnimationLoader() override = default;

    AnimationAsset LoadAsset(const std::string& _assetPath) override;
};

} // namespace OriGine
