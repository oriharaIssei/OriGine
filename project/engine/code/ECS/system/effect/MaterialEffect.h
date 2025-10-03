#pragma once
#include "system/ISystem.h"

#include <memory>

/// engine
// directX12
#include "directX12/RenderTexture.h"

/// component
#include "component/effect/MaterialEffectPipeLine.h"
/// system
#include "system/postRender/DissolveEffect.h"
#include "system/postRender/DistortionEffect.h"

/// <summary>
/// Material にPostEffectをかけるためのSystem
/// </summary>
class MaterialEffect
    : public ISystem {
public:
    MaterialEffect();
    ~MaterialEffect() override;
    void Initialize() override;
    void Finalize() override;

private:
    void UpdateEntity(GameEntity* _entity) override;
    /// <summary>
    /// TextureにEffectをかける
    /// </summary>
    void TextureEffect(GameEntity* _entity, MaterialEffectType _type, RenderTexture* _output);

private:
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;

    std::unique_ptr<RenderTexture> tempRenderTexture_ = nullptr;

    std::unique_ptr<DissolveEffect> dissolveEffect_     = nullptr;
    std::unique_ptr<DistortionEffect> distortionEffect_ = nullptr;
};
