#pragma once

#include "asset/SoundAsset.h"
#include "IAssetLoader.h"

namespace OriGine {

/// <summary>
/// RIFF/WAVE 形式の音声ファイルを読み込むローダー.
/// </summary>
class WaveLoader
    : public IAssetLoader<SoundAsset> {
public:
    WaveLoader()           = default;
    ~WaveLoader() override = default;

    /// <summary>
    /// WAVE ファイルを読み込む.
    /// </summary>
    /// <param name="_assetPath">読み込む WAVE ファイルのパス</param>
    /// <returns>読み込まれた音声アセット（失敗時は空の SoundData を持つアセット）</returns>
    SoundAsset LoadAsset(const std::string& _assetPath) override;
};

} // namespace OriGine
