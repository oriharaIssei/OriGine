#pragma once

/// engine
// asset
#include "asset/ModelAsset.h"
#include "AssetManager.h"

namespace OriGine {
constexpr size_t kModelAssetManagerDefaultCapacity = 128;

/// <summary>
/// モデルアセットの管理を担当するクラス.
/// </summary>
class ModelAssetManager
    : public AssetManager<ModelAsset> {
public:
    ModelAssetManager()           = default;
    ~ModelAssetManager() override = default;

    void Initialize(size_t _capacity = kModelAssetManagerDefaultCapacity) override;

private:
    void SetupDirectoryRules() override;
    void SetupLoaders() override;
};

} // namespace OriGine
