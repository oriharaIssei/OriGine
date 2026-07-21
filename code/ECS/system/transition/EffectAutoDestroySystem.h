#pragma once
#include "system/ISystem.h"

/// <summary>
/// エフェクト自動破棄システム
/// エフェクトエンティティのアニメーションが全て終了したら自動で破棄する
/// </summary>
class EffectAutoDestroySystem
    : public OriGine::ISystem {
public:
    EffectAutoDestroySystem();
    ~EffectAutoDestroySystem() override;
    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize() override;
    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

protected:
    /// <summary>
    /// 対象エンティティのアニメーションが全て終了しているか確認し、終了していれば破棄する
    /// </summary>
    /// <param name="_handle">対象エンティティのハンドル</param>
    void UpdateEntity(const OriGine::EntityHandle& _handle) override;
};
