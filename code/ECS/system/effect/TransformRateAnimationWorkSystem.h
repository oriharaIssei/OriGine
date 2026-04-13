#pragma once
#include "system/ISystem.h"

namespace OriGine {

/// <summary>
/// 速度・加速度ベースのトランスフォームアニメーションを制御するシステム
/// </summary>
class TransformRateAnimationWorkSystem
    : public ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    TransformRateAnimationWorkSystem() : ISystem(SystemCategory::Effect) {}

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~TransformRateAnimationWorkSystem() override = default;

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
    /// 各エンティティの速度・加速度アニメーションを更新する
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
