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
    void Initialize() override;
    void Finalize() override;

protected:
    void UpdateEntity(OriGine::EntityHandle _handle) override;
};
