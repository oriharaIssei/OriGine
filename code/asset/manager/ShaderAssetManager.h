#pragma once

/// engine
// asset
#include "asset/ShaderAsset.h"
#include "AssetManager.h"

namespace OriGine {
constexpr size_t kShaderAssetManagerDefaultCapacity = 128;

/// <summary>
/// シェーダーアセットの管理を担当するクラス.
///
/// 同じ HLSL でもプロファイルが違えば別バイナリになるため、
/// プロファイルをバリアントとして「パス + プロファイル」単位でキャッシュする.
/// </summary>
class ShaderAssetManager
    : public AssetManager<ShaderAsset> {
public:
    ShaderAssetManager()           = default;
    ~ShaderAssetManager() override = default;

    void Initialize(size_t _capacity = kShaderAssetManagerDefaultCapacity) override;

private:
    void SetupLoaders() override;
};

} // namespace OriGine
