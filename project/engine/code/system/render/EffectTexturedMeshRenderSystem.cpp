#include "EffectTexturedMeshRenderSystem.h"

/// engine
#include "Engine.h"
// directX12Object
#include "directX12/DxDevice.h"
// module
#include "camera/CameraManager.h"
#include "texture/TextureManager.h"

// ECS
// component
#include "component/effect/TextureEffectParam.h"
#include "component/material/light/LightManager.h"
#include "component/renderer/MeshRenderer.h"
#include "component/renderer/primitive/Primitive.h"
#include "component/renderer/SkyboxRenderer.h"

void EffectTexturedMeshRenderSystem::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    CreatePso();
}

void EffectTexturedMeshRenderSystem::Update() {
    if (entityIDs_.empty()) {
        return;
    }
    ISystem::eraseDeadEntity();

    StartRender();

    for (auto& id : entityIDs_) {
        GameEntity* entity = getEntity(id);
        UpdateEntity(entity);
    }
}

void EffectTexturedMeshRenderSystem::Finalize() {
    dxCommand_->Finalize();
}

void EffectTexturedMeshRenderSystem::CreatePso() {

    ShaderManager* shaderManager = ShaderManager::getInstance();
    DxDevice* dxDevice           = Engine::getInstance()->getDxDevice();

    // 登録されているかどうかをチェック
    if (shaderManager->IsRegistertedPipelineStateObj("EffectTextured_" + blendModeStr[0])) {
        for (size_t i = 0; i < kBlendNum; ++i) {
            BlendMode blend = static_cast<BlendMode>(i);
            if (pso_[blend]) {
                continue;
            }
            pso_[blend] = shaderManager->getPipelineStateObj("EffectTextured_" + blendModeStr[i]);
        }

        transformBufferIndex_ = 0;
        cameraBufferIndex_    = 1;
        materialBufferIndex_  = 2;
        directionalLightBufferIndex_ = 3;
        pointLightBufferIndex_       = 4;
        spotLightBufferIndex_        = 5;
        lightCountBufferIndex_       = 6;
        mainTextureBufferIndex_      = 7;
        dissolveTextureBufferIndex_  = 8;
        maskTextureBufferIndex_      = 9;
        distortionTextureBufferIndex_ = 10;
        effectParameterBufferIndex_   = 11;

        return;
    }

    ///=================================================
    /// shader読み込み
    ///=================================================
    shaderManager->LoadShader("Object3dTexture.VS");
    shaderManager->LoadShader("TextureEffect.PS", shaderDirectory, L"ps_6_0");

    ///=================================================
    /// shader情報の設定
    ///=================================================
    ShaderInfo texShaderInfo{};
    texShaderInfo.vsKey = "Object3dTexture.VS";
    texShaderInfo.psKey = "TextureEffect.PS";

#pragma region "RootParameter"
    D3D12_ROOT_PARAMETER rootParameter[12]{};
    // Transform ... 0
    rootParameter[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameter[0].Descriptor.ShaderRegister = 0;
    transformBufferIndex_                      = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[0]);
    // CameraTransform ... 1
    rootParameter[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter[1].Descriptor.ShaderRegister = 2;
    cameraBufferIndex_                         = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[1]);
    // Material ... 2
    rootParameter[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[2].Descriptor.ShaderRegister = 0;
    materialBufferIndex_                       = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[2]);

    // DirectionalLight ... 3 (StructuredBuffer)
    rootParameter[3].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[3].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[3].Descriptor.ShaderRegister = 4; // t1 register for DirectionalLight StructuredBuffer
    directionalLightBufferIndex_               = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[3]);

    // PointLight ... 4 (StructuredBuffer)
    rootParameter[4].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[4].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[4].Descriptor.ShaderRegister = 5; // t3 register for PointLight StructuredBuffer
    pointLightBufferIndex_                     = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[4]);

    // SpotLight ... 5 (StructuredBuffer)
    rootParameter[5].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[5].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[5].Descriptor.ShaderRegister = 6; // t4 register for SpotLight StructuredBuffer
    spotLightBufferIndex_                      = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[5]);

    // lightCounts ... 6
    rootParameter[6].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[6].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[6].Descriptor.ShaderRegister = 7;
    lightCountBufferIndex_                     = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[6]);

    // Main Texture ... 7
    // DescriptorTable を使う
    rootParameter[7].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    mainTextureBufferIndex_           = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[7]);
    // Dissolve Texture ... 8
    // DescriptorTable を使う
    rootParameter[8].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    dissolveTextureBufferIndex_       = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[8]);
    // Mask Texture ... 9
    // DescriptorTable を使う
    rootParameter[9].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    maskTextureBufferIndex_           = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[9]);
    // Distortion Texture ... 10
    // DescriptorTable を使う
    rootParameter[10].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    distortionTextureBufferIndex_      = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[10]);

    // effectParam ... 11
    rootParameter[11].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[11].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[11].Descriptor.ShaderRegister = 8;
    effectParameterBufferIndex_                 = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[11]);

#pragma region "TextureDescriptorRange"
    D3D12_DESCRIPTOR_RANGE textureRange[4] = {};
    for (size_t i = 0; i < 4; i++) {
        textureRange[i].BaseShaderRegister = (UINT)i;
        textureRange[i].NumDescriptors     = 1;
        // SRV を扱うように設定
        textureRange[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        // offset を自動計算するように 設定
        textureRange[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    }

    texShaderInfo.setDescriptorRange2Parameter(&textureRange[0], 1, mainTextureBufferIndex_);
    texShaderInfo.setDescriptorRange2Parameter(&textureRange[1], 1, dissolveTextureBufferIndex_);
    texShaderInfo.setDescriptorRange2Parameter(&textureRange[2], 1, maskTextureBufferIndex_);
    texShaderInfo.setDescriptorRange2Parameter(&textureRange[3], 1, distortionTextureBufferIndex_);
#pragma endregion

#pragma region "LightDescriptorRange"
    D3D12_DESCRIPTOR_RANGE directionalLightRange[1]            = {};
    directionalLightRange[0].BaseShaderRegister                = 4;
    directionalLightRange[0].NumDescriptors                    = 1;
    directionalLightRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    directionalLightRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE pointLightRange[1]            = {};
    pointLightRange[0].BaseShaderRegister                = 5;
    pointLightRange[0].NumDescriptors                    = 1;
    pointLightRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    pointLightRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE spotLightRange[1]            = {};
    spotLightRange[0].BaseShaderRegister                = 6;
    spotLightRange[0].NumDescriptors                    = 1;
    spotLightRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    spotLightRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    texShaderInfo.setDescriptorRange2Parameter(directionalLightRange, 1, directionalLightBufferIndex_);
    texShaderInfo.setDescriptorRange2Parameter(pointLightRange, 1, pointLightBufferIndex_);
    texShaderInfo.setDescriptorRange2Parameter(spotLightRange, 1, spotLightBufferIndex_);
#pragma endregion

#pragma endregion

    ///=================================================
    /// Sampler
    D3D12_STATIC_SAMPLER_DESC staticSampler = {};
    staticSampler.Filter                    = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイナリニアフィルタ
    // 0 ~ 1 の間をリピート
    staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

    staticSampler.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
    staticSampler.MinLOD           = 0;
    staticSampler.MaxLOD           = D3D12_FLOAT32_MAX;
    staticSampler.ShaderRegister   = 0;
    staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    texShaderInfo.pushBackSamplerDesc(staticSampler);
    /// Sampler
    ///=================================================

#pragma region "InputElement"
    D3D12_INPUT_ELEMENT_DESC inputElementDesc = {};
    inputElementDesc.SemanticName             = "POSITION"; /*Semantics*/
    inputElementDesc.SemanticIndex            = 0; /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format                   = DXGI_FORMAT_R32G32B32A32_FLOAT; // float 4
    inputElementDesc.AlignedByteOffset        = D3D12_APPEND_ALIGNED_ELEMENT;
    texShaderInfo.pushBackInputElementDesc(inputElementDesc);

    inputElementDesc.SemanticName      = "TEXCOORD"; /*Semantics*/
    inputElementDesc.SemanticIndex     = 0;
    inputElementDesc.Format            = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    texShaderInfo.pushBackInputElementDesc(inputElementDesc);

    inputElementDesc.SemanticName      = "NORMAL"; /*Semantics*/
    inputElementDesc.SemanticIndex     = 0;
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    texShaderInfo.pushBackInputElementDesc(inputElementDesc);
#pragma endregion

    ///=================================================
    /// BlendMode ごとの Psoを作成
    ///=================================================
    for (size_t i = 0; i < kBlendNum; ++i) {
        BlendMode blend = static_cast<BlendMode>(i);
        if (pso_[blend]) {
            continue;
        }
        texShaderInfo.blendMode_       = blend;
        pso_[texShaderInfo.blendMode_] = shaderManager->CreatePso("EffectTextured_" + blendModeStr[i], texShaderInfo, dxDevice->getDevice());
    }
}

void EffectTexturedMeshRenderSystem::LightUpdate() {
    auto* directionalLight = getComponentArray<DirectionalLight>();
    auto* pointLight       = getComponentArray<PointLight>();
    auto* spotLight        = getComponentArray<SpotLight>();

    auto* lightManager = LightManager::getInstance();

    lightManager->clearLights();

    for (auto& lightVec : *directionalLight->getAllComponents()) {
        for (auto& light : lightVec) {
            if (light.isActive()) {
                lightManager->pushDirectionalLight(light);
            }
        }
    }
    for (auto& lightVec : *pointLight->getAllComponents()) {
        for (auto& light : lightVec) {
            if (light.isActive()) {
                lightManager->pushPointLight(light);
            }
        }
    }
    for (auto& lightVec : *spotLight->getAllComponents()) {
        for (auto& light : lightVec) {
            if (light.isActive()) {
                lightManager->pushSpotLight(light);
            }
        }
    }

    LightManager::getInstance()->Update();
}

void EffectTexturedMeshRenderSystem::StartRender() {
    currentBlend_ = BlendMode::Alpha;

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand_->getCommandList();

    commandList->SetGraphicsRootSignature(pso_[currentBlend_]->rootSignature.Get());
    commandList->SetPipelineState(pso_[currentBlend_]->pipelineState.Get());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    CameraManager::getInstance()->setBufferForRootParameter(commandList, cameraBufferIndex_);

    LightUpdate();
    LightManager::getInstance()->SetForRootParameter(
        commandList, lightCountBufferIndex_, directionalLightBufferIndex_, pointLightBufferIndex_, spotLightBufferIndex_);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::getInstance()->getSrvHeap()->getHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
}

/// <summary>
/// 描画
/// </summary>
/// <param name="_entity">描画対象オブジェクト</param>
void EffectTexturedMeshRenderSystem::UpdateEntity(GameEntity* _entity) {
    auto commandList      = dxCommand_->getCommandList();
    int32_t componentIndex = 0;

    TextureEffectParam* effectParam = getComponent<TextureEffectParam>(_entity);
    if (effectParam) {
        // ============================= テクスチャの設定 ============================= //
        // dissolve
        commandList->SetGraphicsRootDescriptorTable(
            dissolveTextureBufferIndex_,
            TextureManager::getDescriptorGpuHandle(effectParam->getDissolveTexIndex()));
        // mask
        commandList->SetGraphicsRootDescriptorTable(
            maskTextureBufferIndex_,
            TextureManager::getDescriptorGpuHandle(effectParam->getMaskTexIndex()));
        // distortion
        commandList->SetGraphicsRootDescriptorTable(
            distortionTextureBufferIndex_,
            TextureManager::getDescriptorGpuHandle(effectParam->getDistortionTexIndex()));

        // ============================= ConstantBuffer ============================= //
        auto& paramBuff = effectParam->getEffectParamBuffer();
        paramBuff->UpdateTransform();
        paramBuff.ConvertToBuffer();
        paramBuff.SetForRootParameter(commandList, effectParameterBufferIndex_);
    }

    Transform* entityTransform_ = getComponent<Transform>(_entity);
    // model
    while (true) {
        ModelMeshRenderer* renderer = getComponent<ModelMeshRenderer>(_entity, componentIndex);

        // nullptr なら これ以上存在しないとして終了
        if (!renderer) {
            break;
        }
        // 描画フラグが立っていないならスキップ
        if (!renderer->isRender()) {
            continue;
        }

        ///==============================
        /// Transformの更新
        ///==============================
        {
            for (int32_t i = 0; i < renderer->getMeshGroupSize(); ++i) {
                auto& transform = renderer->getTransformBuff(i);

                if (transform->parent == nullptr) {
                    transform->parent = entityTransform_;
                }

                transform.openData_.Update();
                transform.ConvertToBuffer();
            }
        }

        // BlendMode を 適応
        BlendMode rendererBlend = renderer->getCurrentBlend();
        if (rendererBlend != currentBlend_) {
            currentBlend_ = rendererBlend;
            commandList->SetGraphicsRootSignature(pso_[currentBlend_]->rootSignature.Get());
            commandList->SetPipelineState(pso_[currentBlend_]->pipelineState.Get());
        }

        uint32_t index = 0;

        auto& meshGroup = renderer->getMeshGroup();
        for (auto& mesh : *meshGroup) {
            // ============================= テクスチャの設定 ============================= //

            commandList->SetGraphicsRootDescriptorTable(
                mainTextureBufferIndex_,
                TextureManager::getDescriptorGpuHandle(renderer->getTextureNumber(index)));

            // ============================= Viewのセット ============================= //
            commandList->IASetVertexBuffers(0, 1, &mesh.getVBView());
            commandList->IASetIndexBuffer(&mesh.getIBView());

            // ============================= Transformのセット ============================= //
            const IConstantBuffer<Transform>& meshTransform = renderer->getTransformBuff(index);
            meshTransform.ConvertToBuffer();
            meshTransform.SetForRootParameter(commandList, transformBufferIndex_);

            // ============================= Materialのセット ============================= //
            auto& material = renderer->getMaterialBuff(index);
            material.openData_.UpdateUvMatrix();
            material.ConvertToBuffer();
            material.SetForRootParameter(commandList, materialBufferIndex_);

            // ============================= 描画 ============================= //
            commandList->DrawIndexedInstanced(UINT(mesh.getIndexSize()), 1, 0, 0, 0);

            ++index;
        }
        componentIndex++;
    }

    // plane
    while (true) {
        PlaneRenderer* renderer = getComponent<PlaneRenderer>(_entity, componentIndex);

        // nullptr なら これ以上存在しないとして終了
        if (!renderer) {
            break;
        }
        // 描画フラグが立っていないならスキップ
        if (!renderer->isRender()) {
            continue;
        }

        ///==============================
        /// Transformの更新
        ///==============================
        {
            auto& transform = renderer->getTransformBuff();

            if (transform->parent == nullptr) {
                transform->parent = entityTransform_;
            }

            transform.openData_.Update();
            transform.ConvertToBuffer();
        }

        // BlendMode を 適応
        BlendMode rendererBlend = renderer->getCurrentBlend();
        if (rendererBlend != currentBlend_) {
            currentBlend_ = rendererBlend;
            commandList->SetGraphicsRootSignature(pso_[currentBlend_]->rootSignature.Get());
            commandList->SetPipelineState(pso_[currentBlend_]->pipelineState.Get());
        }

        uint32_t index = 0;

        auto& meshGroup = renderer->getMeshGroup();
        for (auto& mesh : *meshGroup) {
            // ============================= テクスチャの設定 ============================= //

            commandList->SetGraphicsRootDescriptorTable(
                mainTextureBufferIndex_,
                TextureManager::getDescriptorGpuHandle(renderer->getTextureIndex()));

            // ============================= Viewのセット ============================= //
            commandList->IASetVertexBuffers(0, 1, &mesh.getVBView());
            commandList->IASetIndexBuffer(&mesh.getIBView());

            // ============================= Transformのセット ============================= //
            const IConstantBuffer<Transform>& meshTransform = renderer->getTransformBuff();
            meshTransform.ConvertToBuffer();
            meshTransform.SetForRootParameter(commandList, transformBufferIndex_);

            // ============================= Materialのセット ============================= //
            auto& material = renderer->getMaterialBuff();
            material.openData_.UpdateUvMatrix();
            material.ConvertToBuffer();
            material.SetForRootParameter(commandList, materialBufferIndex_);

            // ============================= 描画 ============================= //
            commandList->DrawIndexedInstanced(UINT(mesh.getIndexSize()), 1, 0, 0, 0);

            ++index;
        }

        componentIndex++;
    }

    // ring
    while (true) {
        RingRenderer* renderer = getComponent<RingRenderer>(_entity, componentIndex);

        // nullptr なら これ以上存在しないとして終了
        if (!renderer) {
            break;
        }
        // 描画フラグが立っていないならスキップ
        if (!renderer->isRender()) {
            continue;
        }

        ///==============================
        /// Transformの更新
        ///==============================
        {
            auto& transform = renderer->getTransformBuff();

            if (transform->parent == nullptr) {
                transform->parent = entityTransform_;
            }

            transform.openData_.Update();
            transform.ConvertToBuffer();
        }

        // BlendMode を 適応
        BlendMode rendererBlend = renderer->getCurrentBlend();
        if (rendererBlend != currentBlend_) {
            currentBlend_ = rendererBlend;
            commandList->SetGraphicsRootSignature(pso_[currentBlend_]->rootSignature.Get());
            commandList->SetPipelineState(pso_[currentBlend_]->pipelineState.Get());
        }

        uint32_t index = 0;

        auto& meshGroup = renderer->getMeshGroup();
        for (auto& mesh : *meshGroup) {
            // ============================= テクスチャの設定 ============================= //

            commandList->SetGraphicsRootDescriptorTable(
                mainTextureBufferIndex_,
                TextureManager::getDescriptorGpuHandle(renderer->getTextureIndex()));

            // ============================= Viewのセット ============================= //
            commandList->IASetVertexBuffers(0, 1, &mesh.getVBView());
            commandList->IASetIndexBuffer(&mesh.getIBView());

            // ============================= Transformのセット ============================= //
            const IConstantBuffer<Transform>& meshTransform = renderer->getTransformBuff();
            meshTransform.ConvertToBuffer();
            meshTransform.SetForRootParameter(commandList, transformBufferIndex_);

            // ============================= Materialのセット ============================= //
            auto& material = renderer->getMaterialBuff();
            material.openData_.UpdateUvMatrix();
            material.ConvertToBuffer();
            material.SetForRootParameter(commandList, materialBufferIndex_);

            // ============================= 描画 ============================= //
            commandList->DrawIndexedInstanced(UINT(mesh.getIndexSize()), 1, 0, 0, 0);

            ++index;
        }

        componentIndex++;
    }
}
