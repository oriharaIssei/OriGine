#include "DistortionEffect.h"

/// engine

#include "Engine.h"
#include "scene/SceneManager.h"
#include "winApp/WinApp.h"
// component
#include "component/effect/post/DistortionEffectParam.h"
#include "component/renderer/primitive/PlaneRenderer.h"
#include "component/transform/Transform.h"
// system
#include "system/render/TexturedMeshRenderSystem.h"

// directX12
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

DistortionEffect::DistortionEffect() : BasePostRenderingSystem() {}

DistortionEffect::~DistortionEffect() {}

void DistortionEffect::Initialize() {
    BasePostRenderingSystem::Initialize();

    distortionSceneTexture_ = std::make_unique<RenderTexture>(dxCommand_.get());
    distortionSceneTexture_->Initialize(2, Engine::getInstance()->getWinApp()->getWindowSize(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Vec4f(0.0f, 0.0f, 0.0f, 0.0f));
    distortionSceneTexture_->setTextureName("DistortionSceneTexture");

    texturedMeshRenderSystem_ = std::make_unique<TexturedMeshRenderSystem>();
    texturedMeshRenderSystem_->Initialize();
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
    ShaderManager* shaderManager = ShaderManager::getInstance();
    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("Distortion.PS", shaderDirectory, L"ps_6_0");
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
    shaderInfo.setDescriptorRange2Parameter(&texRange[0], 1, distortionTextureIndex_);

    // scene texture (t1)
    texRange[1].BaseShaderRegister                = 1;
    texRange[1].NumDescriptors                    = 1;
    texRange[1].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    texRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParameter[1].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    sceneTextureIndex_                = (int32_t)shaderInfo.pushBackRootParameter(rootParameter[1]);
    shaderInfo.setDescriptorRange2Parameter(&texRange[1], 1, sceneTextureIndex_);

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
    shaderInfo.setDepthStencilDesc(depthStencilDesc);

    pso_ = shaderManager->CreatePso("DistortionEffect", shaderInfo, Engine::getInstance()->getDxDevice()->device_);
}

void DistortionEffect::RenderStart() {
    auto& commandList = dxCommand_->getCommandList();

    // シーンテクスチャのサイズ確認
    if (distortionSceneTexture_->getTextureSize() != Engine::getInstance()->getWinApp()->getWindowSize()) {
        // サイズが変わったら再初期化
        distortionSceneTexture_->Resize(Engine::getInstance()->getWinApp()->getWindowSize());
    }

    /// ----------------------------------------------------------
    /// 3dオブジェクトシーンテクスチャへの描画開始
    /// ----------------------------------------------------------
    distortionSceneTexture_->PreDraw();
    texturedMeshRenderSystem_->SettingPSO(BlendMode::Alpha, false);
    texturedMeshRenderSystem_->StartRender();

    for (auto& object : activeDistortionObjects_) {
        ///==============================
        /// Transformの更新
        ///==============================
        {
            auto& transform = object->getTransformBuff();
            transform->UpdateMatrix();
            transform.ConvertToBuffer();
        }

        texturedMeshRenderSystem_->RenderPrimitiveMesh(
            commandList,
            object);
    }
    distortionSceneTexture_->PostDraw();

    /// ----------------------------------------------------------
    /// pso set
    /// ----------------------------------------------------------
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());

    renderTarget_->PreDraw();

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::getInstance()->getSrvHeap()->getHeap().Get()};
    dxCommand_->getCommandList()->SetDescriptorHeaps(1, ppHeaps);
}

void DistortionEffect::Rendering() {
    auto commandList = dxCommand_->getCommandList();

    for (auto& renderingData : activeRenderingData_) {
        // 開始処理
        RenderStart();

        // set buffer
        renderingData.effectParam->getEffectParamBuffer().ConvertToBuffer();
        renderingData.effectParam->getEffectParamBuffer().SetForRootParameter(commandList, distortionParamIndex_);
        commandList->SetGraphicsRootDescriptorTable(distortionTextureIndex_, renderingData.srvHandle);
        commandList->SetGraphicsRootDescriptorTable(sceneTextureIndex_, renderTarget_->getBackBufferSrvHandle());

        // Draw
        commandList->DrawInstanced(6, 1, 0, 0);

        // 終了処理
        RenderEnd();
    }

    // 描画データクリア
    activeRenderingData_.clear();
    activeDistortionObjects_.clear();
}

void DistortionEffect::RenderEnd() {
    // 終了処理
    renderTarget_->PostDraw();
}

void DistortionEffect::DispatchComponent(Entity* _entity) {
    auto distortionEffectParams = getComponents<DistortionEffectParam>(_entity);
    if (!distortionEffectParams) {
        return;
    }
    for (auto& effectParam : *distortionEffectParams) {
        if (!effectParam.getIsActive()) {
            continue;
        }

        RenderingData renderingData{};
        renderingData.effectParam = &effectParam;
        renderingData.srvHandle   = TextureManager::getDescriptorGpuHandle(effectParam.getTextureIndex());

        if (effectParam.getUse3dObjectList()) {
            for (auto& [primitiveRenderBase, type] : effectParam.getDistortionObjects()) {
                // activeでないならスキップ
                if (!primitiveRenderBase->isRender()) {
                    continue;
                }
                // 追加
                activeDistortionObjects_.push_back(primitiveRenderBase.get());

                // srvハンドルを3dオブジェクトシーンテクスチャのものに変更
                // frontなのは、RenderStart内で 描画が終わった後に使用するため(frontに書き込み -> backになる ため)
                renderingData.srvHandle = distortionSceneTexture_->getFrontBufferSrvHandle();
            }
        } else {
            // 単一テクスチャを使用する場合
            // マテリアル情報の更新
            int32_t materialIndex = effectParam.getMaterialIndex();
            auto& materialBuff    = effectParam.getMaterialBuffer();
            if (materialIndex >= 0) {
                Material* material = getComponent<Material>(_entity, materialIndex);
                material->UpdateUvMatrix();

                materialBuff.ConvertToBuffer(ColorAndUvTransform(material->color_, material->uvTransform_));

                if (material->hasCustomTexture()) {
                    renderingData.srvHandle = material->getCustomTexture()->srv_->getGpuHandle();
                }
            }

            // 追加
            activeRenderingData_.emplace_back(renderingData);
        }
    }
}

bool DistortionEffect::ShouldSkipPostRender() const {
    return activeRenderingData_.empty();
}
