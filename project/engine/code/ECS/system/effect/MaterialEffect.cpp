#include "MaterialEffect.h"

/// engine
#include "texture/TextureManager.h"
// directX12
#include "directX12/DxDevice.h"

/// ECS
#include "component/effect/post/DissolveEffectParam.h"
#include "component/effect/post/DistortionEffectParam.h"
#include "component/effect/post/GradationComponent.h"
#include "component/material/Material.h"

using namespace OriGine;

MaterialEffect::MaterialEffect() : ISystem(SystemCategory::Effect) {}
MaterialEffect::~MaterialEffect() {}

void MaterialEffect::Initialize() {
    constexpr Vec4f kClearColor        = {0.f, 0.f, 0.f, 0.f};
    constexpr Vec2f kDefaultTempRTSize = {1024.f, 1024.f};

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    // DSVリソースの作成
    dsvResource_ = std::make_unique<DxResource>();
    dsvResource_->CreateDSVBuffer(Engine::GetInstance()->GetDxDevice()->device_, static_cast<UINT64>(kDefaultTempRTSize[X]), static_cast<UINT>(kDefaultTempRTSize[Y]));

    // DSV の設定
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format        = DXGI_FORMAT_D24_UNORM_S8_UINT; // resourceに合わせる
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2d Texture

    dxDsv_ = Engine::GetInstance()->GetDsvHeap()->CreateDescriptor(dsvDesc, dsvResource_.get());

    // 一時RenderTextureの作成
    int32_t index = 0;
    for (auto& tempRenderTexture : tempRenderTextures_) {
        tempRenderTexture = std::make_unique<RenderTexture>(dxCommand_.get());
        tempRenderTexture->Initialize(2, kDefaultTempRTSize, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kClearColor);
        tempRenderTexture->SetTextureName("MaterialEffect_" + std::to_string(index));
        tempRenderTexture->SetDxDsv(&dxDsv_);
        ++index;
    }

    // system の初期化
    dissolveEffect_ = std::make_unique<DissolveEffect>();
    dissolveEffect_->SetScene(this->GetScene());
    dissolveEffect_->Initialize();
    distortionEffect_ = std::make_unique<DistortionEffect>();
    distortionEffect_->SetScene(this->GetScene());
    distortionEffect_->Initialize();
    gradationEffect_ = std::make_unique<GradationEffect>();
    gradationEffect_->SetScene(this->GetScene());
    gradationEffect_->Initialize();
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

    // DSVリソースの解放
    Engine::GetInstance()->GetDsvHeap()->ReleaseDescriptor(dxDsv_);
    dsvResource_->Finalize();
}

void MaterialEffect::Update() {
    EraseDeadEntity();

    if (entities_.empty()) {
        return;
    }

    // 前フレームの描画対象をクリア
    effectPipelines_.clear();

    for (auto& id : entities_) {
        DispatchComponents(id);
    }

    // アクティブなレンダラーが一つもなければ終了
    if (effectPipelines_.empty()) {
        return;
    }

    std::sort(effectPipelines_.begin(), effectPipelines_.end(), [](std::pair<EntityHandle, MaterialEffectPipeLine*>& a, std::pair<EntityHandle, MaterialEffectPipeLine*>& b) {
        return a.second->GetPriority() < b.second->GetPriority();
    });

    for (auto& [entity, pipeline] : effectPipelines_) {
        UpdateEffectPipeline(entity, pipeline);
    }
}

void MaterialEffect::DispatchComponents(EntityHandle _handle) {
    auto& materialEffectPipeLines = GetComponents<MaterialEffectPipeLine>(_handle);
    if (materialEffectPipeLines.empty()) {
        return;
    }
    for (auto& pipeline : materialEffectPipeLines) {
        // 非アクティブならスルー
        if (!pipeline.IsActive()) {
            continue;
        }
        // effectEntityDataList が空ならスルー
        if (pipeline.GetEffectEntityIdList().empty()) {
            continue;
        }
        // baseTextureId が不正ならスルー
        int32_t baseTextureId = pipeline.GetBaseTextureId();
        if (baseTextureId < 0) {
            continue;
        }

        Material* material = GetComponent<Material>(_handle, pipeline.GetMaterialIndex());
        if (!material) { // Material が存在しなかったらスルー
            continue;
        }
        effectPipelines_.emplace_back(std::make_pair(_handle, &pipeline));
    }
}

void MaterialEffect::UpdateEffectPipeline(EntityHandle _handle, MaterialEffectPipeLine* _pipeline) {
    auto& commandList = dxCommand_->GetCommandList();

    auto tempRenderTexture       = tempRenderTextures_[currentTempRTIndex_].get();
    const Vec2f& tempTextureSize = tempRenderTexture->GetTextureSize();

    Material* material    = GetComponent<Material>(_handle, _pipeline->GetMaterialIndex());
    int32_t baseTextureId = _pipeline->GetBaseTextureId();

    // CustomTexture がなければ作成
    if (!material->hasCustomTexture()) {
        // baseTexture のメタデータを取得
        // フォーマットとmipLevelと width,height を固定
        DirectX::TexMetadata metaData = TextureManager::GetTexMetadata(baseTextureId);
        metaData.width                = static_cast<size_t>(tempTextureSize[X]);
        metaData.height               = static_cast<size_t>(tempTextureSize[Y]);
        metaData.format               = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        metaData.mipLevels            = 1;
        material->CreateCustomTextureFromMetaData(metaData);
    }
    auto effectedTextureResource = &material->GetCustomTexture()->resource_;

    // tempRenderTexture_ に baseTexture を描画
    tempRenderTexture->PreDraw();
    tempRenderTexture->DrawTexture(TextureManager::GetDescriptorGpuHandle(baseTextureId));
    tempRenderTexture->PostDraw();

    // effectEntityDataList に登録されている Entity でエフェクトをかける
    const auto& effectEntityDataList = _pipeline->GetEffectEntityIdList();
    for (auto& effectData : effectEntityDataList) {
        TextureEffect(effectData.entityHandle, effectData.effectType, tempRenderTexture);
    }

    // 最終的に tempRenderTexture_ にエフェクトがかかったテクスチャが入っているので
    // Component に 渡す
    dxCommand_->ResourceBarrier(effectedTextureResource->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);
    dxCommand_->ResourceBarrier(tempRenderTexture->GetBackBuffer(), D3D12_RESOURCE_STATE_COPY_SOURCE);

    // コピー
    commandList->CopyResource(effectedTextureResource->GetResource().Get(), tempRenderTexture->GetBackBuffer().Get());

    // 状態を戻す
    dxCommand_->ResourceBarrier(tempRenderTexture->GetBackBuffer(), D3D12_RESOURCE_STATE_COMMON);
    dxCommand_->ResourceBarrier(effectedTextureResource->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // コマンド実行
    ExecuteCommand();

    // 次のRenderTextureを使う
    currentTempRTIndex_ = (currentTempRTIndex_ + 1) % static_cast<int32_t>(tempRenderTextures_.size());
}

void MaterialEffect::ExecuteCommand() {
    HRESULT result;
    DxFence* fence = Engine::GetInstance()->GetDxFence();

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
    UINT64 fenceVal = fence->Signal(dxCommand_->GetCommandQueue());
    fence->WaitForFence(fenceVal);
    ///===============================================================

    ///===============================================================
    /// リセット
    ///===============================================================
    dxCommand_->CommandReset();
    ///===============================================================
}

void MaterialEffect::TextureEffect(EntityHandle _handle, MaterialEffectType _type, RenderTexture* _output) {
    switch (_type) {
    case MaterialEffectType::Dissolve: {
        dissolveEffect_->AddEntity(_handle);

        dissolveEffect_->SetRenderTarget(_output);

        dissolveEffect_->Update();

        dissolveEffect_->ClearEntities();

        break;
    }
    case MaterialEffectType::Distortion: {
        distortionEffect_->AddEntity(_handle);

        distortionEffect_->SetRenderTarget(_output);

        distortionEffect_->Update();

        distortionEffect_->ClearEntities();

        break;
    }
    case MaterialEffectType::Gradation: {
        gradationEffect_->AddEntity(_handle);

        gradationEffect_->SetRenderTarget(_output);

        gradationEffect_->Update();

        gradationEffect_->ClearEntities();

        break;
    }
    default:
        break;
    }
}
