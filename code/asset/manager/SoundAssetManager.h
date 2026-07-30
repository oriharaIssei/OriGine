#pragma once

/// engine
// asset
#include "asset/SoundAsset.h"
#include "AssetManager.h"

namespace OriGine {
constexpr size_t kSoundAssetManagerDefaultCapacity = 64;

/// <summary>
/// 音声アセットの管理を担当するクラス.
/// </summary>
class SoundAssetManager
    : public AssetManager<SoundAsset> {
public:
    SoundAssetManager()           = default;
    ~SoundAssetManager() override = default;

    void Initialize(size_t _capacity = kSoundAssetManagerDefaultCapacity) override;

private:
    void SetupDirectoryRules() override;
    void SetupLoaders() override;
};

} // namespace OriGine
