#pragma once
#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// マテリアルアニメーションワークシステム
/// </summary>
class MaterialAnimationWorkSystem
    : public ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    MaterialAnimationWorkSystem() : ISystem(SystemCategory::Effect) {}

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~MaterialAnimationWorkSystem() override = default;

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override {}

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override {}

protected:
    /// <summary>
    /// 各エンティティのマテリアルアニメーションを更新する
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
