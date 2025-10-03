#include "MaterialEffect.h"

/// engine
#include "texture/TextureManager.h"

/// ECS
#include "component/effect/post/DissolveEffectParam.h"
#include "component/effect/post/DistortionEffectParam.h"

MaterialEffect::MaterialEffect() : ISystem(SystemCategory::Effect) {}

MaterialEffect::~MaterialEffect() {}

void MaterialEffect::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    DxCommand::CreateCommandListWithAllocator(Engine::getInstance()->getDxDevice()->getDevice(), "MaterialEffect", D3D12_COMMAND_LIST_TYPE_COPY);
    dxCommand_->Initialize("MaterialEffect", "MaterialEffect");

    tempRenderTexture_ = std::make_unique<RenderTexture>();
    tempRenderTexture_->Initialize(2, Vec2f(1280.f, 1280.f));

    dissolveEffect_ = std::make_unique<DissolveEffect>();
    dissolveEffect_->Initialize();
    distortionEffect_ = std::make_unique<DistortionEffect>();
    distortionEffect_->Initialize();
}

void MaterialEffect::Finalize() {
    if (dissolveEffect_) {
        dissolveEffect_->Finalize();
        dissolveEffect_.reset();
        dissolveEffect_ = nullptr;
    }
    if (distortionEffect_) {
        distortionEffect_->Finalize();
        distortionEffect_.reset();
        distortionEffect_ = nullptr;
    }
    if (tempRenderTexture_) {
        tempRenderTexture_->Finalize();
        tempRenderTexture_.reset();
        tempRenderTexture_ = nullptr;
    }
}

void MaterialEffect::UpdateEntity(GameEntity* _entity) {
    auto commandList             = dxCommand_->getCommandList();
    auto materialEffectPipeLines = getComponents<MaterialEffectPipeLine>(_entity);
    if (!materialEffectPipeLines) {
        return;
    }
    for (auto& pipeline : *materialEffectPipeLines) {
        if (!pipeline.isActive()) { // active じゃなかったらスルー
            continue;
        }
        DxResource* effectedTextureResource = &pipeline.getEffectedTextureResource();

        if (!effectedTextureResource->getResource()) {
            continue;
        }

        // 必要ならサイズを合わせる
        if (tempRenderTexture_->getTextureSize() != pipeline.getTextureSize()) {
            tempRenderTexture_->Resize(pipeline.getTextureSize());
        }
        // tempRenderTexture_ に baseTexture を描画
        tempRenderTexture_->DrawTexture(TextureManager::getDescriptorGpuHandle(pipeline.getBaseTextureId()));

        const auto& effectEntityDataList = pipeline.getEffectEntityIdList();
        for (auto id : effectEntityDataList) {
            GameEntity* effectEntity = getEntity(id.entityID);
            if (!effectEntity) { // エンティティが存在しなかったらスルー
                continue;
            }
            TextureEffect(effectEntity, id.effectType, tempRenderTexture_.get());
        }
        // 最終的に tempRenderTexture_ にエフェクトがかかったテクスチャが入っているので
        // Component に 渡す
        dxCommand_->ResourceBarrier(tempRenderTexture_->getBackBuffer(), D3D12_RESOURCE_STATE_COPY_SOURCE);
        dxCommand_->ResourceBarrier(effectedTextureResource, D3D12_RESOURCE_STATE_COPY_DEST);

        // コピー
        commandList->CopyResource(effectedTextureResource->getResource().Get(), tempRenderTexture_->getBackBuffer().Get());

        // 状態を戻す
        dxCommand_->ResourceBarrier(effectedTextureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        dxCommand_->ResourceBarrier(tempRenderTexture_->getBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
}

void MaterialEffect::TextureEffect(GameEntity* _entity, MaterialEffectType _type, RenderTexture* _output) {
    switch (_type) {
    case MaterialEffectType::Dissolve: {
        auto dissolveParam = getComponent<DissolveEffectParam>(_entity);
        if (dissolveParam) {
            if (dissolveParam->isActive()) {
                dissolveEffect_->EffectEntity(_output, _entity);
            }
        }
    } break;
    case MaterialEffectType::Distortion: {
        auto distortionParam = getComponent<DistortionEffectParam>(_entity);
        if (distortionParam) {
            distortionEffect_->EffectEntity(_output, _entity);
        }
    } break;
    default:
        break;
    }
}
