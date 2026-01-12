#pragma once
#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// トランスフォーム（座標・回転・スケール）のアニメーションを制御するシステム
/// </summary>
class TransformAnimationWorkSystem
    : public ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    TransformAnimationWorkSystem() : ISystem(SystemCategory::Effect) {}

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~TransformAnimationWorkSystem() override = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize() override {}

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override {}

protected:
    /// <summary>
    /// 各エンティティのトランスフォームアニメーションを更新する
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
