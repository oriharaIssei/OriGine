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
    void UpdateEntity(Entity* /*_entity*/) override {}

    void DispatchComponents(Entity* _entity);
    void UpdateEffectPipeline(Entity* _entity, MaterialEffectPipeLine* _pipeline);
    /// <summary>
    /// TextureにEffectをかける
    /// </summary>
    void TextureEffect(Entity* _entity, MaterialEffectType _type, RenderTexture* _output);

    void ExecuteCommand();

private:
    int32_t currentTempRTIndex_ = 0;

    std::array<std::unique_ptr<RenderTexture>, 2> tempRenderTextures_ = {nullptr, nullptr};

    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    DxDsvDescriptor dxDsv_{};
    std::unique_ptr<DxResource> dsvResource_ = nullptr;

    std::vector<std::pair<Entity*, MaterialEffectPipeLine*>> effectPipelines_;

    std::unique_ptr<DissolveEffect> dissolveEffect_     = nullptr;
    std::unique_ptr<DistortionEffect> distortionEffect_ = nullptr;
    std::unique_ptr<GradationEffect> gradationEffect_   = nullptr;
};
