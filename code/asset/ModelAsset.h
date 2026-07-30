#pragma once

#include "Asset.h"

/// engine
#include "model/Model.h"

namespace OriGine {

/// <summary>
/// モデルアセット.
/// メッシュ・ノード階層・スケルトン・既定マテリアルといった、
/// モデルファイル由来で全インスタンスから共有される静的データを保持する.
/// インスタンス固有の情報（マテリアルバッファ等）は Model 側が持つ.
/// </summary>
struct ModelAsset
    : public Asset {
    ModelMeshData meshData;
};

template <>
struct AssetTraits<ModelAsset> {
    using type = ModelAsset;

    /// <summary>
    /// 対応するファイル拡張子の配列を取得する.
    /// </summary>
    /// <returns></returns>
    static constexpr std::array<std::string, 4> Extensions() {
        return {".obj", ".gltf", ".glb", ".fbx"};
    }
};

} // namespace OriGine
