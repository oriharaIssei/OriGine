#pragma once

#include "system/ISystem.h"

/// ECS
// component
#include "component/animation/DissolveAnimation.h"
#include <entity/EntityHandle.h>

namespace OriGine {

/// <summary>
/// DissolveAnimation を更新するシステム
/// </summary>
class DissolveAnimationSystem
    : public ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    DissolveAnimationSystem();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~DissolveAnimationSystem() override;

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

    /// <summary>
    /// エンティティのディゾルブアニメーションを更新する
    /// </summary>
    /// <param name="_handle">対象のエンティティハンドル</param>
    void UpdateEntity(EntityHandle _handle) override;
};

} // namespace OriGine
