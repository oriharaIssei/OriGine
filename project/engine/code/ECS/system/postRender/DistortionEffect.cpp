#include "DistortionEffect.h"

/// engine
#include "Engine.h"
#include "scene/SceneManager.h"
#include "texture/TextureManager.h"
#include "winApp/WinApp.h"
// component
#include "component/effect/post/DistortionEffectParam.h"
#include "component/renderer/primitive/base/PrimitiveMeshRendererBase.h"
#include "component/transform/Transform.h"
// system
#include "system/render/TexturedMeshRenderSystemWithoutRaytracing.h"

// directX12
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

using namespace OriGine;

DistortionEffect::DistortionEffect() : BasePostRenderingSystem() {}
DistortionEffect::~DistortionEffect() {}

void DistortionEffect::Initialize() {
    BasePostRenderingSystem::Initialize();

    distortionSceneTexture_ = std::make_unique<RenderTexture>(dxCommand_.get());
    distortionSceneTexture_->Initialize(2, Engine::GetInstance()->GetWinApp()->GetWindowSize(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Vec4f(0.0f, 0.0f, 0.0f, 0.0f));
    distortionSceneTexture_->SetTextureName("DistortionSceneTexture");

    texturedMeshRenderSystem_ = std::make_unique<TexturedMeshRenderSystemWithoutRaytracing>();
    texturedMeshRenderSystem_->SetScene(GetScene());
    texturedMeshRenderSystem_->Initialize();

    defaultParam_ = std::make_unique<DistortionEffectParam>();
    defaultParam_->Initialize(nullptr, EntityHandle());
}

void DistortionEffect::Finalize() {
    pso_ = nullptr;

    if (dxCommand_) {
        dxCommand_->Finalize();
        dxCommand_.reset();
        dxCommand_ = nullptr;
    }

    texturedMeshRenderSystem_->Finalize();

    if (distortionSceneTexture_) {
        distortionSceneTexture_->Finalize();
        distortionSceneTexture_.reset();
    }
}

void DistortionEffect::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("Distortion.PS", kShaderDirectory, L"ps_6_0");
    ShaderInformation shaderInfo{};
    shaderInfo.vsKey = "FullScreen.VS";
    shaderInfo.psKey = "Distortion.PS";

    ///================================================
    /// Sampler の設定
    ///================================================
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter                    = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイナリニアフィルタ
    // 0 ~ 1 の間をリピート
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

    sampler.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
    sampler.MinLOD           = 0;
    sampler.MaxLOD           = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister   = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    shaderInfo.pushBackSamplerDesc(sampler);

    ///================================================
    /// RootParameter の設定
    ///================================================
    // distortion Texture
    D3D12_ROOT_PARAMETER rootParameter[4] = {};
    D3D12_DESCRIPTOR_RANGE texRange[2]    = {};

    // distortion texture (t0)
    texRange[0].BaseShaderRegister                = 0;
    texRange[0].NumDescriptors                    = 1;
    texRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    texRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParameter[0].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    distortionTextureIndex_           = (int32_t)shaderInfo.pushBackRootParameter(rootParameter[0]);
    shaderInfo.SetDescriptorRange2Parameter(&texRange[0], 1, distortionTextureIndex_);

    // scene texture (t1)
    texRange[1].BaseShaderRegister                = 1;
    texRange[1].NumDescriptors                    = 1;
    texRange[1].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    texRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParameter[1].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    sceneTextureIndex_                = (int32_t)shaderInfo.pushBackRootParameter(rootParameter[1]);
    shaderInfo.SetDescriptorRange2Parameter(&texRange[1], 1, sceneTextureIndex_);

    // distortion param
    rootParameter[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[2].Descriptor.ShaderRegister = 0;
    distortionParamIndex_                      = (int32_t)shaderInfo.pushBackRootParameter(rootParameter[2]);

    rootParameter[3].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[3].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[3].Descriptor.ShaderRegister = 1;
    materialIndex_                             = (int32_t)shaderInfo.pushBackRootParameter(rootParameter[3]);

    ///================================================
    /// InputElement の設定
    ///================================================

    // 特に使わない

    ///================================================
    /// depthStencil の設定
    ///================================================
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = false;
    shaderInfo.SetDepthStencilDesc(depthStencilDesc);

    pso_ = shaderManager->CreatePso("DistortionEffect", shaderInfo, Engine::GetInstance()->GetDxDevice()->device_);
}

void DistortionEffect::RenderStart() {
    auto& commandList = dxCommand_->GetCommandList();

    // シーンテクスチャのサイズ確認
    if (distortionSceneTexture_->GetTextureSize() != Engine::GetInstance()->GetWinApp()->GetWindowSize()) {
        // サイズが変わったら再初期化
        distortionSceneTexture_->Resize(Engine::GetInstance()->GetWinApp()->GetWindowSize());
    }

    /// ----------------------------------------------------------
    /// 3dオブジェクトシーンテクスチャへの描画開始
    /// ----------------------------------------------------------
    if (!activeDistortionObjects_.empty()) {
        distortionSceneTexture_->PreDraw();

        texturedMeshRenderSystem_->SetBlendMode(BlendMode::Alpha);
        texturedMeshRenderSystem_->SetCulling(false);

        texturedMeshRenderSystem_->StartRender();

        for (auto& object : activeDistortionObjects_) {
            ///==============================
            /// Transformの更新
            ///==============================
            {
                auto& transform = object->GetTransformBuff();
                transform->UpdateMatrix();
                transform.ConvertToBuffer();
            }

            texturedMeshRenderSystem_->RenderPrimitiveMesh(
                commandList,
                object);
        }
        distortionSceneTexture_->PostDraw();
    }

    /// ----------------------------------------------------------
    /// pso Set
    /// ----------------------------------------------------------
    renderTarget_->PreDraw();

    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    dxCommand_->GetCommandList()->SetDescriptorHeaps(1, ppHeaps);
}

void DistortionEffect::Rendering() {
    auto& commandList = dxCommand_->GetCommandList();

    // 3dオブジェクトでエフェクトをかける
    if (!activeDistortionObjects_.empty()) {
        // 開始処理
        RenderStart();

        // Set buffer
        commandList->SetGraphicsRootDescriptorTable(distortionTextureIndex_, distortionSceneTexture_->GetBackBufferSrvHandle());
        commandList->SetGraphicsRootDescriptorTable(sceneTextureIndex_, renderTarget_->GetBackBufferSrvHandle());
        defaultParam_->GetEffectParamBuffer().SetForRootParameter(commandList, distortionParamIndex_);
        defaultParam_->GetMaterialBuffer().SetForRootParameter(commandList, materialIndex_);

        // Draw
        commandList->DrawInstanced(6, 1, 0, 0);

        // 終了処理
        RenderEnd();

        activeDistortionObjects_.clear();
    }

    for (auto& renderingData : activeRenderingData_) {
        // 開始処理
        RenderStart();

        // Set buffer
        renderingData.effectParam->GetEffectParamBuffer().ConvertToBuffer();
        commandList->SetGraphicsRootDescriptorTable(distortionTextureIndex_, renderingData.srvHandle);
        commandList->SetGraphicsRootDescriptorTable(sceneTextureIndex_, renderTarget_->GetBackBufferSrvHandle());
        renderingData.effectParam->GetEffectParamBuffer().SetForRootParameter(commandList, distortionParamIndex_);
        renderingData.effectParam->GetMaterialBuffer().SetForRootParameter(commandList, materialIndex_);

        // Draw
        commandList->DrawInstanced(6, 1, 0, 0);

        // 終了処理
        RenderEnd();
    }

    // 描画データクリア
    activeRenderingData_.clear();
}

void DistortionEffect::RenderEnd() {
    // 終了処理
    renderTarget_->PostDraw();
}

void DistortionEffect::DispatchComponent(EntityHandle _handle) {
    auto& distortionEffectParams = GetComponents<DistortionEffectParam>(_handle);
    if (distortionEffectParams.empty()) {
        return;
    }

    for (auto& effectParam : distortionEffectParams) {
        if (!effectParam.GetIsActive()) {
            continue;
        }

        auto* entityTransform = GetComponent<Transform>(_handle);

        // 3dオブジェクトリストを使用する場合
        if (effectParam.GetUse3dObjectList()) {
            auto& paramData = effectParam.GetEffectParamData();
            for (auto& [primitiveRenderBase, type] : effectParam.GetDistortionObjects()) {
                // activeでないならスキップ
                if (!primitiveRenderBase->IsRender()) {
                    continue;
                }

                // マテリアル情報の更新
                int32_t materialIndex = primitiveRenderBase->GetMaterialIndex();
                auto& materialBuff    = primitiveRenderBase->GetMaterialBuff();
                if (materialIndex >= 0) {
                    Material* material = GetComponent<Material>(_handle, materialIndex);
                    material->UpdateUvMatrix();

                    // 歪み強度・バイアスの反映
                    // 後に すべての Objectを描画し一度だけシーンテクスチャに描画するため、
                    // 前もってエフェクトパラメータを反映させておく
                    Material data  = *material;
                    data.color_[X] = (data.color_[X] - paramData.distortionBias[X]) * paramData.distortionStrength[X];
                    data.color_[Y] = (data.color_[Y] - paramData.distortionBias[Y]) * paramData.distortionStrength[Y];
                    if (material->hasCustomTexture()) {
                        data.SetCustomTexture(material->GetCustomTexture()->srv_, material->GetCustomTexture()->resource_);
                    }

                    materialBuff.ConvertToBuffer(data);
                }
                // Transformの更新
                {
                    auto& transform   = primitiveRenderBase->GetTransformBuff();
                    transform->parent = entityTransform;
                    transform->UpdateMatrix();
                    transform.ConvertToBuffer();
                }

                // 追加
                activeDistortionObjects_.push_back(primitiveRenderBase.get());
            }
        } else {
            // 単一テクスチャを使用する場合
            RenderingData renderingData{};
            renderingData.effectParam = &effectParam;
            renderingData.srvHandle   = TextureManager::GetDescriptorGpuHandle(effectParam.GetTextureIndex());

            // マテリアル情報の更新
            int32_t materialIndex = effectParam.GetMaterialIndex();
            auto& materialBuff    = effectParam.GetMaterialBuffer();
            if (materialIndex >= 0) {
                Material* material = GetComponent<Material>(_handle, materialIndex);
                material->UpdateUvMatrix();

                materialBuff.ConvertToBuffer(ColorAndUvTransform(material->color_, material->uvTransform_));

                if (material->hasCustomTexture()) {
                    renderingData.srvHandle = material->GetCustomTexture()->srv_.GetGpuHandle();
                }
            }
            // 追加
            activeRenderingData_.emplace_back(renderingData);
        }
    }
}

bool DistortionEffect::ShouldSkipPostRender() const {
    return activeRenderingData_.empty() && activeDistortionObjects_.empty();
}
