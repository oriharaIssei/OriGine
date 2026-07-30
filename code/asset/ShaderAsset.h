#pragma once

#include "Asset.h"

/// Microsoft
#include <wrl.h>
/// api
#include <dxcapi.h>

namespace OriGine {

/// <summary>
/// シェーダーアセット. HLSL をコンパイルした結果のバイナリ (Blob) を保持する.
///
/// 同じ HLSL ファイルでもプロファイル (vs_6_0 / ps_6_5 / cs_6_0 など) が異なれば
/// 別のバイナリになるため、AssetManager 上ではプロファイルをバリアントとして扱い、
/// 「パス + プロファイル」で一意のアセットとして管理される.
/// </summary>
struct ShaderAsset
    : public Asset {
    /// <summary>コンパイル済みシェーダーバイナリ</summary>
    Microsoft::WRL::ComPtr<IDxcBlob> blob;
    /// <summary>コンパイルに使用したプロファイル (vs_6_0 等)</summary>
    std::string profile;
};

template <>
struct AssetTraits<ShaderAsset> {
    using type = ShaderAsset;

    /// <summary>
    /// 対応するファイル拡張子の配列を取得する.
    /// </summary>
    /// <returns></returns>
    static constexpr std::array<std::string, 1> Extensions() {
        return {".hlsl"};
    }
};

} // namespace OriGine
