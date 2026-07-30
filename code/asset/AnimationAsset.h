#pragma once

#include "Asset.h"

/// engine
#include "component/animation/AnimationData.h"

namespace OriGine {

/// <summary>
/// アニメーションアセット.
/// gltf 等のモデルファイルに含まれるアニメーション、または
/// 独自形式 (.anm) にシリアライズされたアニメーションを保持する.
/// </summary>
struct AnimationAsset
    : public Asset {
    AnimationData data;
};

template <>
struct AssetTraits<AnimationAsset> {
    using type = AnimationAsset;

    /// <summary>
    /// 対応するファイル拡張子の配列を取得する.
    /// </summary>
    /// <returns></returns>
    static constexpr std::array<std::string, 3> Extensions() {
        return {".gltf", ".glb", ".anm"};
    }
};

} // namespace OriGine
