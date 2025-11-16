#include "MaterialEffect.h"

/// engine
#include "texture/TextureManager.h"

/// ECS
#include "component/effect/post/DissolveEffectParam.h"
#include "component/effect/post/DistortionEffectParam.h"
#include "component/effect/post/GradationTextureComponent.h"
#include "component/material/Material.h"

MaterialEffect::MaterialEffect() : ISystem(SystemCategory::Effect) {}

MaterialEffect::~MaterialEffect() {}

void MaterialEffect::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    int32_t index = 0;
    for (auto& tempRenderTexture : tempRenderTextures_) {
        tempRenderTexture = std::make_unique<RenderTexture>(dxCommand_.get());
        tempRenderTexture->Initialize(2, Vec2f(512.f, 512.f));
        tempRenderTexture->setTextureName("MaterialEffect_" + std::to_string(index));
        ++index;
    }

    dissolveEffect_ = std::make_unique<DissolveEffect>();
    dissolveEffect_->setScene(this->getScene());
    dissolveEffect_->Initialize();
    distortionEffect_ = std::make_unique<DistortionEffect>();
    distortionEffect_->setScene(this->getScene());
    distortionEffect_->Initialize();
    gradationEffect_ = std::make_unique<GradationEffect>();
    gradationEffect_->setScene(this->getScene());
    gradationEffect_->Initialize();
}

void MaterialEffect::Update() {
    eraseDeadEntity();

    if (entityIDs_.empty()) {
        return;
    }

    // 前フレームの描画対象をクリア
    effectPipelines_.clear();

    for (auto& id : entityIDs_) {
        Entity* entity = getEntity(id);
        DispatchComponents(entity);
    }

    // アクティブなレンダラーが一つもなければ終了
    if (effectPipelines_.empty()) {
        return;
    }

    std::sort(effectPipelines_.begin(), effectPipelines_.end(), [](std::pair<Entity*, MaterialEffectPipeLine*>& a, std::pair<Entity*, MaterialEffectPipeLine*>& b) {
        return a.second->getPriority() < b.second->getPriority();
    });

    for (auto& [entity, pipeline] : effectPipelines_) {
        UpdateEffectPipeline(entity, pipeline);
    }
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

    for (auto& tempRenderTexture : tempRenderTextures_) {
        if (tempRenderTexture) {
            tempRenderTexture->Finalize();
        }
    }
}

void MaterialEffect::DispatchComponents(Entity* _entity) {
    auto materialEffectPipeLines = getComponents<MaterialEffectPipeLine>(_entity);
    if (!materialEffectPipeLines) {
        return;
    }
    for (auto& pipeline : *materialEffectPipeLines) {
        if (!pipeline.isActive()) {
            continue;
        }
        Material* material = getComponent<Material>(_entity, pipeline.getMaterialIndex());
        if (!material) { // Material が存在しなかったらスルー
            continue;
        }
        int32_t baseTextureId = pipeline.getBaseTextureId();
        if (baseTextureId < 0) {
            continue;
        }
        effectPipelines_.emplace_back(std::make_pair(_entity, &pipeline));
    }
}

void MaterialEffect::UpdateEffectPipeline(Entity* _entity, MaterialEffectPipeLine* _pipeline) {
    auto commandList = dxCommand_->getCommandList();

    Material* material    = getComponent<Material>(_entity, _pipeline->getMaterialIndex());
    int32_t baseTextureId = _pipeline->getBaseTextureId();

    // CustomTexture がなければ作成
    if (!material->hasCustomTexture()) {
        material->CreateCustomTextureFromTextureFile(_pipeline->getBaseTextureId());
    }
    auto effectedTextureResource = &material->getCustomTexture()->resource_;
    Vec2f textureSize            = {(float)effectedTextureResource->width(), (float)effectedTextureResource->height()};

    // baseTexture のメタデータを取得
    // フォーマットとmipLevelを固定
    DirectX::TexMetadata metaData = TextureManager::getTexMetadata(baseTextureId);
    metaData.format               = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    metaData.mipLevels            = 1;

    // tempRenderTexture_ のサイズを baseTexture に合わせる
    auto tempRenderTexture = tempRenderTextures_[currentTempRTIndex_].get();
    if (tempRenderTexture->getTextureSize() != textureSize) {
        tempRenderTexture->Resize(textureSize);
    }

    // tempRenderTexture_ に baseTexture を描画
    tempRenderTexture->PreDraw();
    tempRenderTexture->DrawTexture(TextureManager::getDescriptorGpuHandle(_pipeline->getBaseTextureId()));
    tempRenderTexture->PostDraw();

    // effectEntityDataList に登録されている Entity でエフェクトをかける
    const auto& effectEntityDataList = _pipeline->getEffectEntityIdList();
    for (auto& id : effectEntityDataList) {
        Entity* effectEntity = getEntity(id.entityID);
        if (!effectEntity) { // エンティティが存在しなかったらスルー
            continue;
        }
        TextureEffect(effectEntity, id.effectType, tempRenderTexture);
    }

    // 最終的に tempRenderTexture_ にエフェクトがかかったテクスチャが入っているので
    // Component に 渡す
    dxCommand_->ResourceBarrier(effectedTextureResource->getResource(), D3D12_RESOURCE_STATE_COPY_DEST);
    dxCommand_->ResourceBarrier(tempRenderTexture->getBackBuffer(), D3D12_RESOURCE_STATE_COPY_SOURCE);

    // コピー
    commandList->CopyResource(effectedTextureResource->getResource().Get(), tempRenderTexture->getBackBuffer().Get());

    // 状態を戻す
    dxCommand_->ResourceBarrier(tempRenderTexture->getBackBuffer(), D3D12_RESOURCE_STATE_COMMON);
    dxCommand_->ResourceBarrier(effectedTextureResource->getResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // コマンド実行
    ExecuteCommand();

    // 次のRenderTextureを使う
    currentTempRTIndex_ = (currentTempRTIndex_ + 1) % static_cast<int32_t>(tempRenderTextures_.size());
}

void MaterialEffect::ExecuteCommand() {
    HRESULT result;
    DxFence* fence = Engine::getInstance()->getDxFence();

    // コマンドの受付終了 -----------------------------------
    result = dxCommand_->Close();
    if (FAILED(result)) {
        LOG_ERROR("Failed to close command list. HRESULT: {}", std::to_string(result));
        assert(false);
    }
    //----------------------------------------------------

    ///===============================================================
    /// コマンドリストの実行
    ///===============================================================
    dxCommand_->ExecuteCommand();
    ///===============================================================

    ///===============================================================
    /// コマンドリストの実行を待つ
    ///===============================================================
    fence->Signal(dxCommand_->getCommandQueue());
    fence->WaitForFence();
    ///===============================================================

    ///===============================================================
    /// リセット
    ///===============================================================
    dxCommand_->CommandReset();
    ///===============================================================
}

void MaterialEffect::TextureEffect(Entity* _entity, MaterialEffectType _type, RenderTexture* _output) {
    switch (_type) {
    case MaterialEffectType::Dissolve: {
        dissolveEffect_->addEntity(_entity);

        dissolveEffect_->setRenderTarget(_output);

        dissolveEffect_->Update();

        dissolveEffect_->clearEntities();

        break;
    }
    case MaterialEffectType::Distortion: {
        distortionEffect_->addEntity(_entity);

        distortionEffect_->setRenderTarget(_output);

        distortionEffect_->Update();

        distortionEffect_->clearEntities();

        break;
    }
    case MaterialEffectType::Gradation: {
        gradationEffect_->addEntity(_entity);

        gradationEffect_->setRenderTarget(_output);

        gradationEffect_->Update();

        gradationEffect_->clearEntities();

        break;
    }
    default:
        break;
    }
}
