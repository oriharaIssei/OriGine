#pragma once

#include "Asset.h"

/// engine
// directX12
#include "directX12/DxDescriptor.h"
#include "directX12/DxResource.h"

// externals
#include <DirectXTex/DirectXTex.h>

namespace OriGine {

/// <summary>
/// 個別のテクスチャリソースとその情報を保持する構造体.
/// </summary>
struct TextureAsset
    : public Asset {
    DirectX::TexMetadata metaData;
    DxResource resource;
    DxSrvDescriptor srv;
};

template <>
struct AssetTraits<TextureAsset> {
    using type = TextureAsset;

    /// <summary>
    /// 対応するファイル拡張子の配列を取得する.
    /// </summary>
    /// <returns></returns>
    static constexpr std::array<std::string, 4> Extensions() {
        return {".png", ".jpg", ".jpeg", ".dds"};
    }
};

} // namespace OriGine
