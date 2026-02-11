#pragma once

/// engine
// asset
#include "asset/TextureAsset.h"
#include "AssetManager.h"

namespace OriGine {
constexpr size_t kTextureAssetManagerDefaultCapacity = 256;

class TextureAssetManager
    : public AssetManager<TextureAsset> {
public:
    TextureAssetManager()           = default;
    ~TextureAssetManager() override = default;

    void Initialize(size_t _capacity = kTextureAssetManagerDefaultCapacity) override;

private:
    void SetupDirectoryRules() override;
    void SetupLoaders() override;
};

} // namespace OriGine
