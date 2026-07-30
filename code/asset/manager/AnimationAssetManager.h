#pragma once

/// engine
// asset
#include "asset/AnimationAsset.h"
#include "AssetManager.h"

namespace OriGine {
constexpr size_t kAnimationAssetManagerDefaultCapacity = 128;

/// <summary>
/// アニメーションアセットの管理を担当するクラス.
/// </summary>
class AnimationAssetManager
    : public AssetManager<AnimationAsset> {
public:
    AnimationAssetManager()           = default;
    ~AnimationAssetManager() override = default;

    void Initialize(size_t _capacity = kAnimationAssetManagerDefaultCapacity) override;

private:
    void SetupDirectoryRules() override;
    void SetupLoaders() override;
};

/// <summary>
/// アニメーションの書き出しを担当するユーティリティ.
/// 読み込みは AnimationAssetManager / 各ローダーが担当するが、
/// 書き出しはエディタからのみ使うためアセット管理とは分離してある.
/// </summary>
namespace AnimationSerializer {
/// <summary>
/// アニメーションを独自バイナリ形式 (.anm) で保存する.
/// </summary>
/// <param name="_directory">保存先ディレクトリ</param>
/// <param name="_filename">ファイル名（拡張子は付けない。.anm が付与される）</param>
/// <param name="_animationData">保存するアニメーション</param>
/// <returns>保存に成功したら true</returns>
bool Save(const std::string& _directory, const std::string& _filename, const AnimationData& _animationData);
} // namespace AnimationSerializer

} // namespace OriGine
