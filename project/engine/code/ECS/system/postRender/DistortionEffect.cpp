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

DistortionEffect::DistortionEffect() : ISystem(SystemCategory::PostRender) {}

DistortionEffect::~DistortionEffect() {}

void DistortionEffect::Initialize() {
    if (!dxCommand_) {
        dxCommand_ = std::make_unique<DxCommand>();
        dxCommand_->Initialize("main", "main");
    }

    distortionSceneTexture_ = std::make_unique<RenderTexture>(dxCommand_.get());
    distortionSceneTexture_->Initialize(2, Engine::getInstance()->getWinApp()->getWindowSize(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Vec4f(0.0f, 0.0f, 0.0f, 0.0f));
    distortionSceneTexture_->setTextureName("DistortionSceneTexture");

    CreatePSO();

    texturedMeshRenderSystem_ = std::make_unique<TexturedMeshRenderSystem>();
    texturedMeshRenderSystem_->Initialize();
}

void DistortionEffect::Update() {
    eraseDeadEntity();

    if (distortionSceneTexture_->getTextureSize() != Engine::getInstance()->getWinApp()->getWindowSize()) {
        // サイズが変わったら再初期化
        distortionSceneTexture_->Resize(Engine::getInstance()->getWinApp()->getWindowSize());
    }

    /// ================================================
    // Rendering Distortion Scene Texture
    /// ================================================

    for (auto& id : entityIDs_) {
        Entity* entity = getEntity(id);
        UpdateEntity(entity);
    }
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

void DistortionEffect::UpdateEntity(Entity* _entity) {
    /// ================================================================================================
    // Rendering Distortion Scene Texture
    /// ================================================================================================
    auto& commandList           = dxCommand_->getCommandList();
    auto distortionEffectParams = getComponents<DistortionEffectParam>(_entity);
    if (!distortionEffectParams) {
        LOG_WARN("Not found DistortionEffectParam Component. EntityID :{}", _entity->getID());
        return;
    }
    Transform* entityTransform_ = getComponent<Transform>(_entity);

    for (auto& effectParam : *distortionEffectParams) {
        if (!effectParam.getIsActive()) {
            continue;
        }
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::getDescriptorGpuHandle(effectParam.getTextureIndex());
        if (effectParam.getUse3dObjectList()) {
            RenderEffectObjectScene(commandList, entityTransform_, &effectParam);
            srvHandle = distortionSceneTexture_->getBackBufferSrvHandle();
        }

        /// ================================================================================================
        // post Process
        /// ================================================================================================

        /// ----------------------------------------------------------
        /// pso set
        /// ----------------------------------------------------------
        auto* sceneView = this->getScene()->getSceneView();
        sceneView->PreDraw();

        commandList->SetPipelineState(pso_->pipelineState.Get());
        commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        /// ----------------------------------------------------------
        /// set buffer
        /// ----------------------------------------------------------
        ///
        int32_t materialIndex = effectParam.getMaterialIndex();
        auto& materialBuff    = effectParam.getMaterialBuffer();
        if (materialIndex >= 0) {
            Material* material = getComponent<Material>(_entity, materialIndex);
            material->UpdateUvMatrix();

            materialBuff.ConvertToBuffer(ColorAndUvTransform(material->color_, material->uvTransform_));

            if (material->hasCustomTexture()) {
                srvHandle = material->getCustomTexture()->srv_->getGpuHandle();
            }
        }
        materialBuff.SetForRootParameter(commandList, materialIndex_);

        effectParam.getEffectParamBuffer().ConvertToBuffer();
        effectParam.getEffectParamBuffer().SetForRootParameter(commandList, distortionParamIndex_);

        ID3D12DescriptorHeap* ppHeaps[] = {Engine::getInstance()->getSrvHeap()->getHeap().Get()};
        commandList->SetDescriptorHeaps(1, ppHeaps);

        commandList->SetGraphicsRootDescriptorTable(distortionTextureIndex_, srvHandle);

        commandList->SetGraphicsRootDescriptorTable(sceneTextureIndex_, sceneView->getBackBufferSrvHandle());

        /// ----------------------------------------------------------
        /// Draw
        /// ----------------------------------------------------------
        commandList->DrawInstanced(6, 1, 0, 0);

        sceneView->PostDraw();
    }
}

void DistortionEffect::RenderEffectObjectScene(
    const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _commandList,
    Transform* _entityTransform,
    DistortionEffectParam* _param) {

    // オブジェクトシーンテクスチャへの描画開始
    distortionSceneTexture_->PreDraw();
    texturedMeshRenderSystem_->SettingPSO(BlendMode::Alpha);
    texturedMeshRenderSystem_->StartRender();

    for (auto& [object, type] : _param->getDistortionObjects()) {

        // nullptr なら これ以上存在しないとして終了
        if (!object) {
            break;
        }
        // 描画フラグが立っていないならスキップ
        if (!object->isRender()) {
            continue;
        }
        ///==============================
        /// Transformの更新
        ///==============================
        {
            auto& transform = object->getTransformBuff();

            if (transform->parent == nullptr) {
                transform->parent = _entityTransform;
            }

            transform->UpdateMatrix();
            transform.ConvertToBuffer();
        }
        texturedMeshRenderSystem_->RenderPrimitiveMesh(
            _commandList,
            object.get());
    }

    distortionSceneTexture_->PostDraw();
}

void DistortionEffect::EffectEntity(RenderTexture* _output, Entity* _entity) {
    if (_output == nullptr) {
        LOG_ERROR("Output RenderTexture is nullptr");
        return;
    }
    auto& commandList            = dxCommand_->getCommandList();
    auto* distortionEffectParams = getComponents<DistortionEffectParam>(_entity);
    if (!distortionEffectParams) {
        LOG_WARN("Not found DistortionEffectParam Component. EntityID :{}", _entity->getID());
        return;
    }
    bool isSkip = true;

    for (auto& effectParam : *distortionEffectParams) {
        if (effectParam.getIsActive()) {
            isSkip = false;
            break;
        }
    }
    if (isSkip) {
        return;
    }

    for (auto& effectParam : *distortionEffectParams) {
        if (!effectParam.getIsActive()) {
            continue;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::getDescriptorGpuHandle(effectParam.getTextureIndex());

        /// ================================================================================================
        // post Process
        /// ================================================================================================

        /// ----------------------------------------------------------
        /// pso set
        /// ----------------------------------------------------------
        _output->PreDraw();

        commandList->SetPipelineState(pso_->pipelineState.Get());
        commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        /// ----------------------------------------------------------
        /// set buffer
        /// ----------------------------------------------------------

        int32_t materialIndex = effectParam.getMaterialIndex();
        auto& materialBuff    = effectParam.getMaterialBuffer();
        if (materialIndex >= 0) {
            Material* material = getComponent<Material>(_entity, materialIndex);
            material->UpdateUvMatrix();

            materialBuff.ConvertToBuffer(ColorAndUvTransform(material->color_, material->uvTransform_));

            if (material->hasCustomTexture()) {
                srvHandle = material->getCustomTexture()->srv_->getGpuHandle();
            }
        }
        materialBuff.SetForRootParameter(commandList, materialIndex_);

        effectParam.getEffectParamBuffer().ConvertToBuffer();
        effectParam.getEffectParamBuffer().SetForRootParameter(commandList, distortionParamIndex_);

        ID3D12DescriptorHeap* ppHeaps[] = {Engine::getInstance()->getSrvHeap()->getHeap().Get()};
        commandList->SetDescriptorHeaps(1, ppHeaps);

        commandList->SetGraphicsRootDescriptorTable(distortionTextureIndex_, srvHandle);

        commandList->SetGraphicsRootDescriptorTable(sceneTextureIndex_, _output->getBackBufferSrvHandle());

        /// ----------------------------------------------------------
        /// Draw
        /// ----------------------------------------------------------
        commandList->DrawInstanced(6, 1, 0, 0);

        _output->PostDraw();
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

    pso_ = shaderManager->CreatePso("DistortionEffect", shaderInfo, Engine::getInstance()->getDxDevice()->getDevice());
}
