#include "MeshRender.h"

/// engine
#include "Engine.h"
// assets
#include "model/Model.h"
// camera
#include "camera/CameraManager.h"
// material
#include "material/light/LightManager.h"
// manager
#include "material/Material.h"
#include "model/ModelManager.h"
#include "RenderManager.h"

//----------------------------------------------------------------------------------------------------------
// ↓ DefaultMeshRenderer
//----------------------------------------------------------------------------------------------------------
#pragma region "TextureMeshRenderer"
void TextureMeshRenderer::Init() {
    if (meshTransformBuff_.size() != meshGroup_->size()) {
        meshTransformBuff_.resize(meshGroup_->size());
    }
    if (meshMaterialBuff_.size() != meshGroup_->size()) {
        meshMaterialBuff_.resize(meshGroup_->size());
    }
    if (meshTextureNumber_.size() != meshGroup_->size()) {
        meshTextureNumber_.resize(meshGroup_->size());
    }
    for (size_t i = 0; i < meshGroup_->size(); ++i) {
        meshTransformBuff_[i].CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
        meshTransformBuff_[i].openData_.Init();
        meshMaterialBuff_[i]  = Engine::getInstance()->getMaterialManager()->getMaterial("white");
        meshTextureNumber_[i] = 0;
    }
}

void TextureMeshRenderer::Update() {
    for (auto& transform : meshTransformBuff_) {
        transform.openData_.Update();
        transform.ConvertToBuffer();
    }
}

void TextureMeshRenderer::Render() {
    IRendererComponentController* controller = RenderManager::getInstance()->getRendererController<TextureMeshRendererController>();
    auto* commandList                        = controller->getDxCommand()->getCommandList();

    uint32_t index = 0;

    for (auto& mesh : *meshGroup_) {
        auto& material = meshMaterialBuff_[index];

        IConstantBuffer<Transform>& meshTransform = meshTransformBuff_[index];
        meshTransform.ConvertToBuffer();

        ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
        commandList->SetDescriptorHeaps(1, ppHeaps);
        commandList->SetGraphicsRootDescriptorTable(
            7,
            TextureManager::getDescriptorGpuHandle(meshTextureNumber_[index]));

        commandList->IASetVertexBuffers(0, 1, &mesh.vbView);
        commandList->IASetIndexBuffer(&mesh.ibView);

        meshTransform.SetForRootParameter(commandList, 0);

        material->SetForRootParameter(commandList, 2);
        // 描画!!!
        commandList->DrawIndexedInstanced(UINT(mesh.indexSize), 1, 0, 0, 0);

        ++index;
    }
}
#pragma endregion

#pragma region "PrimitiveMeshRenderer"
PrimitiveMeshRenderer::PrimitiveMeshRenderer() {
}

PrimitiveMeshRenderer::~PrimitiveMeshRenderer() {
}

void PrimitiveMeshRenderer::Init() {
    if (meshTransformBuff_.size() != meshGroup_->size()) {
        meshTransformBuff_.resize(meshGroup_->size());
    }
    if (meshMaterialBuff_.size() != meshGroup_->size()) {
        meshMaterialBuff_.resize(meshGroup_->size());
    }
    for (size_t i = 0; i < meshGroup_->size(); ++i) {
        meshTransformBuff_[i].CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
        meshTransformBuff_[i].openData_.Init();
        meshMaterialBuff_[i] = Engine::getInstance()->getMaterialManager()->getMaterial("white");
    }
}

void PrimitiveMeshRenderer::Update() {
    for (auto& transform : meshTransformBuff_) {
        transform.openData_.Update();
        transform.ConvertToBuffer();
    }
}

void PrimitiveMeshRenderer::Render() {
    IRendererComponentController* controller = RenderManager::getInstance()->getRendererController<PrimitiveMeshRendererController>();
    auto* commandList                        = controller->getDxCommand()->getCommandList();

    uint32_t index = 0;

    for (auto& mesh : *meshGroup_) {
        auto& material = meshMaterialBuff_[index];

        IConstantBuffer<Transform>& meshTransform = meshTransformBuff_[index];
        meshTransform.ConvertToBuffer();

        commandList->IASetVertexBuffers(0, 1, &mesh.vbView);
        commandList->IASetIndexBuffer(&mesh.ibView);

        meshTransform.SetForRootParameter(commandList, 0);

        material->SetForRootParameter(commandList, 2);
        // 描画!!!
        commandList->DrawIndexedInstanced(UINT(mesh.indexSize), 1, 0, 0, 0);

        ++index;
    }
}
#pragma endregion

//----------------------------------------------------------------------------------------------------------
// ↓ PrimitiveMeshContorller
//----------------------------------------------------------------------------------------------------------
void PrimitiveMeshRendererController::StartRender() {
    auto* commandList = dxCommand_->getCommandList();
    commandList->SetGraphicsRootSignature(pso_[currentBlend_]->rootSignature.Get());
    commandList->SetPipelineState(pso_[currentBlend_]->pipelineState.Get());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void PrimitiveMeshRendererController::CreatePso() {
    ShaderManager* shaderManager = ShaderManager::getInstance();
    ///=================================================
    /// shader読み込み
    ///=================================================

    shaderManager->LoadShader("Object3d.VS");
    shaderManager->LoadShader("Object3d.PS", shaderDirectory, L"ps_6_0");

    ///=================================================
    /// shader情報の設定
    ///=================================================
    ShaderInfo primShaderInfo;
    primShaderInfo.vsKey = "Object3d.VS";
    primShaderInfo.psKey = "Object3d.PS";

#pragma region "RootParameter"
    D3D12_ROOT_PARAMETER rootParameter[7]{};
    rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    // PixelShaderで使う
    rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    // レジスタ番号0 とバインド
    // register(b0) の 0. b11 なら 11
    rootParameter[0].Descriptor.ShaderRegister = 0;
    primShaderInfo.pushBackRootParameter(rootParameter[0]);

    rootParameter[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter[1].Descriptor.ShaderRegister = 2;
    primShaderInfo.pushBackRootParameter(rootParameter[1]);

    rootParameter[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[2].Descriptor.ShaderRegister = 0;
    primShaderInfo.pushBackRootParameter(rootParameter[2]);

    rootParameter[3].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[3].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[3].Descriptor.ShaderRegister = 1; // t1 register for DirectionalLight StructuredBuffer
    primShaderInfo.pushBackRootParameter(rootParameter[3]);

    // PointLight ... 4 (StructuredBuffer)
    rootParameter[4].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[4].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[4].Descriptor.ShaderRegister = 3; // t3 register for PointLight StructuredBuffer
    primShaderInfo.pushBackRootParameter(rootParameter[4]);

    // SpotLight ... 5 (StructuredBuffer)
    rootParameter[5].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[5].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[5].Descriptor.ShaderRegister = 4; // t4 register for SpotLight StructuredBuffer
    primShaderInfo.pushBackRootParameter(rootParameter[5]);
    // lightCounts
    rootParameter[6].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[6].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[6].Descriptor.ShaderRegister = 5;
    primShaderInfo.pushBackRootParameter(rootParameter[6]);

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

    primShaderInfo.SetDescriptorRange2Parameter(directionalLightRange, 1, 3);
    primShaderInfo.SetDescriptorRange2Parameter(pointLightRange, 1, 4);
    primShaderInfo.SetDescriptorRange2Parameter(spotLightRange, 1, 5);
#pragma endregion

#pragma region "Input Element"
    D3D12_INPUT_ELEMENT_DESC inputElementDesc = {};
    inputElementDesc.SemanticName             = "POSITION"; /*Semantics*/
    inputElementDesc.SemanticIndex            = 0; /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format                   = DXGI_FORMAT_R32G32B32A32_FLOAT; // float 4
    inputElementDesc.AlignedByteOffset        = D3D12_APPEND_ALIGNED_ELEMENT;
    primShaderInfo.pushBackInputElementDesc(inputElementDesc);

    inputElementDesc.SemanticName      = "NORMAL"; /*Semantics*/
    inputElementDesc.SemanticIndex     = 0;
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    primShaderInfo.pushBackInputElementDesc(inputElementDesc);
#pragma endregion

    ///=================================================
    /// BlendMode ごとの Pso作成
    ///=================================================
    primShaderInfo.topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    for (size_t i = 0; i < kBlendNum; ++i) {
        primShaderInfo.blendMode_       = static_cast<BlendMode>(i);
        pso_[primShaderInfo.blendMode_] = shaderManager->CreatePso("PrimitiveMesh_" + blendModeStr[i], primShaderInfo, dxDevice_->getDevice());
    }
};

//----------------------------------------------------------------------------------------------------------
// ↓ TextureMeshContoroller
//----------------------------------------------------------------------------------------------------------
void TextureMeshRendererController::StartRender() {
    currentBlend_ = BlendMode::Alpha;

    ID3D12GraphicsCommandList* commandList = dxCommand_->getCommandList();

    commandList->SetGraphicsRootSignature(pso_[currentBlend_]->rootSignature.Get());
    commandList->SetPipelineState(pso_[currentBlend_]->pipelineState.Get());

    
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    Engine::getInstance()->getLightManager()->SetForRootParameter(commandList);

    CameraManager::getInstance()->setBufferForRootParameter(commandList, 1);
}

void TextureMeshRendererController::CreatePso() {
    ShaderManager* shaderManager_ = ShaderManager::getInstance();
    ///=================================================
    /// shader読み込み
    ///=================================================
    shaderManager_->LoadShader("Object3dTexture.VS");
    shaderManager_->LoadShader("Object3dTexture.PS", shaderDirectory, L"ps_6_0");

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
        texShaderInfo.blendMode_ = static_cast<BlendMode>(i);
        pso_[texShaderInfo.blendMode_] =  shaderManager_->CreatePso("TextureMesh_" + blendModeStr[i], texShaderInfo, dxDevice_->getDevice());
    }
}
