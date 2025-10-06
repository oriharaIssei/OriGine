#pragma once
#include "system/ISystem.h"

#include <array>
#include <memory>

/// engine
// directX12
#include "directX12/RenderTexture.h"

/// component
#include "component/effect/MaterialEffectPipeLine.h"
/// system
#include "system/postRender/DissolveEffect.h"
#include "system/postRender/DistortionEffect.h"
#include "system/postRender/GradationEffect.h"

/// <summary>
/// Material にPostEffectをかけるためのSystem
/// </summary>
class MaterialEffect
    : public ISystem {
public:
    MaterialEffect();
    ~MaterialEffect() override;
    void Initialize() override;
    void Update();
    void Finalize() override;

private:
    /// <summary>
    /// 使用していない
    /// </summary>
    void UpdateEntity(GameEntity* /*_entity*/) override {}

    void DispatchComponents(GameEntity* _entity);
    void UpdateEffectPipeline(GameEntity* _entity, MaterialEffectPipeLine* _pipeline);
    /// <summary>
    /// TextureにEffectをかける
    /// </summary>
    void TextureEffect(GameEntity* _entity, MaterialEffectType _type, RenderTexture* _output);

    void ExecuteCommand();

private:
    std::unique_ptr<DxCommand> dxCommand_ = nullptr; // Direct

    int32_t currentTempRTIndex_                                       = 0;
    std::array<std::unique_ptr<RenderTexture>, 3> tempRenderTextures_ = {nullptr, nullptr};

    std::vector<std::pair<GameEntity*, MaterialEffectPipeLine*>> effectPipelines_;

    std::unique_ptr<DissolveEffect> dissolveEffect_     = nullptr;
    std::unique_ptr<DistortionEffect> distortionEffect_ = nullptr;
    std::unique_ptr<GradationEffect> gradationEffect_   = nullptr;
};
