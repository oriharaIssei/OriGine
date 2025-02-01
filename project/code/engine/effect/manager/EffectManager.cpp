#include "EffectManager.h"

#include "../Effect.h"

///engine
#include "Engine.h"
//dx12Object
#include "directX12/DxSrvArrayManager.h"
//lib
#include "camera/CameraManager.h"
#include "myFileSystem/MyFileSystem.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

EffectManager* EffectManager::getInstance() {
    static EffectManager instance;
    return &instance;
}

void EffectManager::Init() {
    dxSrvArray_ = DxSrvArrayManager::getInstance()->Create(srvNum_);

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Init(Engine::getInstance()->getDxDevice()->getDevice(), "main", "main");

    for (size_t i = 0; i < kBlendNum; i++) {
        psoKey_[i] = "Particle_" + blendModeStr[i];
    }

    CreatePso();
}

void EffectManager::Finalize() {
    if (dxCommand_) {
        dxCommand_->Finalize();
    }
    if (dxSrvArray_) {
        dxSrvArray_->Finalize();
    }

    effects_.clear();
}

void EffectManager::PreDraw() {
    auto* commandList = dxCommand_->getCommandList();

    commandList->SetGraphicsRootSignature(pso_[int(blendMode_)]->rootSignature.Get());
    commandList->SetPipelineState(pso_[int(blendMode_)]->pipelineState.Get());

    CameraManager::getInstance()->setBufferForRootParameter(commandList, 1);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void EffectManager::UpdateEffects(float _deltaTime) {
    for (auto& effect : effects_) {
        effect->Update(_deltaTime);
    }
}

void EffectManager::Draw() {
    for (auto& effect : effects_) {
        effect->Draw();
    }
}

void EffectManager::ChangeBlendMode(BlendMode mode) {
    if (blendMode_ == mode) {
        return;
    }
    blendMode_ = mode;

    auto* commandList = dxCommand_->getCommandList();

    commandList->SetGraphicsRootSignature(pso_[int(blendMode_)]->rootSignature.Get());
    commandList->SetPipelineState(pso_[int(blendMode_)]->pipelineState.Get());
}

EffectManager::EffectManager() {}

EffectManager::~EffectManager() {}

void EffectManager::CreatePso() {
    ShaderManager* shaderManager = ShaderManager::getInstance();
    ///=================================================
    /// shader読み込み
    ///=================================================

    shaderManager->LoadShader("Particle.VS");
    shaderManager->LoadShader("Particle.PS", shaderDirectory, L"ps_6_0");

    ///=================================================
    /// shader情報の設定
    ///=================================================
    ShaderInfo shaderInfo;
    shaderInfo.vsKey = "Particle.VS";
    shaderInfo.psKey = "Particle.PS";

#pragma region "RootParameter"
    D3D12_ROOT_PARAMETER rootParameter[4]{};
    // 0 ... ParticleData
    rootParameter[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameter[0].Descriptor.ShaderRegister = 0;
    shaderInfo.pushBackRootParameter(rootParameter[0]);

    D3D12_DESCRIPTOR_RANGE structuredRange[1]            = {};
    structuredRange[0].BaseShaderRegister                = 0;
    structuredRange[0].NumDescriptors                    = 1;
    structuredRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    structuredRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(structuredRange, 1, 0);
    // 1 ... ViewProjection
    rootParameter[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter[1].Descriptor.ShaderRegister = 1;
    shaderInfo.pushBackRootParameter(rootParameter[1]);
    // 2 ... Material
    rootParameter[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[2].Descriptor.ShaderRegister = 0;
    shaderInfo.pushBackRootParameter(rootParameter[2]);
    // 3 ... Texture
    // DescriptorTable を使う
    rootParameter[3].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t rootParameterIndex         = shaderInfo.pushBackRootParameter(rootParameter[3]);

    D3D12_DESCRIPTOR_RANGE descriptorRange[1]            = {};
    descriptorRange[0].BaseShaderRegister                = 0;
    descriptorRange[0].NumDescriptors                    = 1;
    descriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(descriptorRange, 1, rootParameterIndex);

#pragma endregion

#pragma region "Input Element"
    D3D12_INPUT_ELEMENT_DESC inputElementDesc = {};
    inputElementDesc.SemanticName             = "POSITION";                     /*Semantics*/
    inputElementDesc.SemanticIndex            = 0;                              /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format                   = DXGI_FORMAT_R32G32B32A32_FLOAT; //float 4
    inputElementDesc.AlignedByteOffset        = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDesc);
    inputElementDesc.SemanticName      = "NORMAL";                    /*Semantics*/
    inputElementDesc.SemanticIndex     = 0;                           /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32_FLOAT; //float 4
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDesc);
    inputElementDesc.SemanticName      = "TEXCOORD";               /*Semantics*/
    inputElementDesc.SemanticIndex     = 0;                        /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format            = DXGI_FORMAT_R32G32_FLOAT; //float 2
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDesc);
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
    shaderInfo.pushBackSamplerDesc(staticSampler);
    /// Sampler
    ///=================================================

    ///=================================================
    /// BlendMode ごとの Pso作成
    ///=================================================
    for (size_t i = 0; i < kBlendNum; i++) {
        shaderInfo.blendMode_ = BlendMode(i);
        pso_[i]               = shaderManager->CreatePso(psoKey_[i], shaderInfo, Engine::getInstance()->getDxDevice()->getDevice());
    }
}

std::unique_ptr<Effect> EffectManager::CreateEffect(const std::string& name) {
    std::unique_ptr<Effect> result = std::make_unique<Effect>(this->dxSrvArray_, name);
    result->Init();
    usingSrvNum_ -= result->getUsingSrvNum();
    if (srvNum_ < usingSrvNum_) {
        // 未対応
        //dxSrvArray_->resize(srvNum_);
    }

    return result;
}

void EffectManager::PlayEffect(const std::string& _effectName) {
    std::unique_ptr<Effect> effect = CreateEffect(_effectName);
    effects_.push_back(std::move(effect));
}
