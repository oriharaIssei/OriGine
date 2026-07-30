#pragma once

#include "Asset.h"

/// api
#include <xaudio2.h>

/// stl
#include <cstdint>
#include <vector>

namespace OriGine {

/// <summary>
/// 音声データ
/// </summary>
struct SoundData {
    /// <summary>波形フォーマット</summary>
    WAVEFORMATEX wfex{};
    /// <summary>バッファ本体（RAII管理）</summary>
    std::vector<BYTE> pBuffer;
    /// <summary>バッファサイズ</summary>
    uint32_t bufferSize = 0;
};

/// <summary>
/// 音声アセット. WAVE ファイルから読み込まれた波形データを保持する.
/// </summary>
struct SoundAsset
    : public Asset {
    SoundData data;
};

template <>
struct AssetTraits<SoundAsset> {
    using type = SoundAsset;

    /// <summary>
    /// 対応するファイル拡張子の配列を取得する.
    /// </summary>
    /// <returns></returns>
    static constexpr std::array<std::string, 1> Extensions() {
        return {".wav"};
    }
};

} // namespace OriGine
