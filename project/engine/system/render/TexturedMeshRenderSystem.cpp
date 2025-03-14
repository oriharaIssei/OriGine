#include "TexturedMeshRenderSystem.h"

/// engine
#include "Engine.h"
// module
#include "camera/CameraManager.h"
#include "texture/TextureManager.h"

// ECS
// component
#include "ECSManager.h"
#include "component/material/light/LightManager.h"
#include "component/renderer/MeshRender.h"

void TexturedMeshRenderSystem::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    CreatePso();
}

void TexturedMeshRenderSystem::Update() {
    StartRender();

    for (auto& entity : entities_) {
        UpdateEntity(entity);
    }
}

void TexturedMeshRenderSystem::Finalize() {
    dxCommand_->Finalize();
}

void TexturedMeshRenderSystem::CreatePso() {
    ShaderManager* shaderManager = ShaderManager::getInstance();
    DxDevice* dxDevice           = Engine::getInstance()->getDxDevice();

    ///=================================================
    /// shader読み込み
    ///=================================================
    shaderManager->LoadShader("Object3dTexture.VS");
    shaderManager->LoadShader("Object3dTexture.PS", shaderDirectory, L"ps_6_0");

    ///=================================================
    /// shader情報の設定
    ///=================================================
    ShaderInfo texShaderInfo{};
    texShaderInfo.vsKey = "Object3dTexture.VS";
    texShaderInfo.psKey = "Object3dTexture.PS";

#pragma region "RootParameter"
    D3D12_ROOT_PARAMETER rootParameter[8]{};
    // Transform ... 0
    rootParameter[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameter[0].Descriptor.ShaderRegister = 0;
    texShaderInfo.pushBackRootParameter(rootParameter[0]);
    // CameraTransform ... 1
    rootParameter[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter[1].Descriptor.ShaderRegister = 2;
    texShaderInfo.pushBackRootParameter(rootParameter[1]);
    // Material ... 2
    rootParameter[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[2].Descriptor.ShaderRegister = 0;
    texShaderInfo.pushBackRootParameter(rootParameter[2]);

    rootParameter[3].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[3].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[3].Descriptor.ShaderRegister = 1; // t1 register for DirectionalLight StructuredBuffer
    texShaderInfo.pushBackRootParameter(rootParameter[3]);

    // PointLight ... 4 (StructuredBuffer)
    rootParameter[4].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[4].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[4].Descriptor.ShaderRegister = 3; // t3 register for PointLight StructuredBuffer
    texShaderInfo.pushBackRootParameter(rootParameter[4]);

    // SpotLight ... 5 (StructuredBuffer)
    rootParameter[5].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[5].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[5].Descriptor.ShaderRegister = 4; // t4 register for SpotLight StructuredBuffer
    texShaderInfo.pushBackRootParameter(rootParameter[5]);

    // lightCounts ... 6
    rootParameter[6].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[6].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[6].Descriptor.ShaderRegister = 5;
    texShaderInfo.pushBackRootParameter(rootParameter[6]);

    // Texture ... 7
    // DescriptorTable を使う
    rootParameter[7].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t rootParameterIndex         = texShaderInfo.pushBackRootParameter(rootParameter[7]);

    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister     = 0;
    descriptorRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE directionalLightRange[1]            = {};
    directionalLightRange[0].BaseShaderRegister                = 1;
    directionalLightRange[0].NumDescriptors                    = 1;
    directionalLightRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    directionalLightRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE pointLightRange[1]            = {};
    pointLightRange[0].BaseShaderRegister                = 3;
    pointLightRange[0].NumDescriptors                    = 1;
    pointLightRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    pointLightRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE spotLightRange[1]            = {};
    spotLightRange[0].BaseShaderRegister                = 4;
    spotLightRange[0].NumDescriptors                    = 1;
    spotLightRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    spotLightRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    texShaderInfo.SetDescriptorRange2Parameter(descriptorRange, 1, rootParameterIndex);
    texShaderInfo.SetDescriptorRange2Parameter(directionalLightRange, 1, 3);
    texShaderInfo.SetDescriptorRange2Parameter(pointLightRange, 1, 4);
    texShaderInfo.SetDescriptorRange2Parameter(spotLightRange, 1, 5);
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
        texShaderInfo.blendMode_       = static_cast<BlendMode>(i);
        pso_[texShaderInfo.blendMode_] = shaderManager->CreatePso("TextureMesh_" + blendModeStr[i], texShaderInfo, dxDevice->getDevice());
    }
}

void TexturedMeshRenderSystem::StartRender() {
    currentBlend_ = BlendMode::Alpha;

    ID3D12GraphicsCommandList* commandList = dxCommand_->getCommandList();

    commandList->SetGraphicsRootSignature(pso_[currentBlend_]->rootSignature.Get());
    commandList->SetPipelineState(pso_[currentBlend_]->pipelineState.Get());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    Engine::getInstance()->getLightManager()->SetForRootParameter(commandList);

    CameraManager::getInstance()->setBufferForRootParameter(commandList, 1);
    LightManager::getInstance()->SetForRootParameter(commandList);
}

/// <summary>
/// 描画
/// </summary>
/// <param name="_entity">描画対象オブジェクト</param>
void TexturedMeshRenderSystem::UpdateEntity(GameEntity* _entity) {
    auto* commandList = dxCommand_->getCommandList();

    ModelMeshRenderer* renderer = getComponent<ModelMeshRenderer>(_entity);

    // nullptr と 描画しないものは skip
    if (!renderer) {
        return;
    }
    if (!renderer->isRender()) {
        return;
    }

    ///==============================
    /// Transformの更新
    ///==============================
    {
        Transform* entityTransform = getComponent<Transform>(_entity);
        for (int32_t i = 0; i < renderer->getMeshSize(); ++i) {
            auto& transform = renderer->getTransformBuff(i);

            if (transform->parent == nullptr) {
                transform->parent = entityTransform;
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
        ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
        commandList->SetDescriptorHeaps(1, ppHeaps);
        commandList->SetGraphicsRootDescriptorTable(
            7,
            TextureManager::getDescriptorGpuHandle(renderer->getTextureNumber(index)));

        // ============================= Viewのセット ============================= //
        commandList->IASetVertexBuffers(0, 1, &mesh.vbView);
        commandList->IASetIndexBuffer(&mesh.ibView);

        // ============================= Transformのセット ============================= //
        const IConstantBuffer<Transform>& meshTransform = renderer->getTransformBuff(index);
        meshTransform.ConvertToBuffer();
        meshTransform.SetForRootParameter(commandList, 0);

        // ============================= Materialのセット ============================= //
        const auto& material = renderer->getMaterialBuff(index);
        material->SetForRootParameter(commandList, 2);

        // ============================= 描画 ============================= //
        commandList->DrawIndexedInstanced(UINT(mesh.indexSize), 1, 0, 0, 0);

        ++index;
    }
}
