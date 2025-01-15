#include "ParticleManager.h"

#include "../emitter/Emitter.h"

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

ParticleManager* ParticleManager::getInstance() {
    static ParticleManager instance;
    return &instance;
}

void ParticleManager::Init() {
    dxSrvArray_ = DxSrvArrayManager::getInstance()->Create(srvNum_);

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Init(Engine::getInstance()->getDxDevice()->getDevice(), "main", "main");

    for (size_t i = 0; i < kBlendNum; i++) {
        psoKey_[i] = "Particle_" + blendModeStr[i];
    }

    CreatePso();

    std::list<std::pair<std::string, std::string>> loadedEmitter = myfs::SearchFile("resource/GlobalVariables/Effects", "json");
    for (auto& [directory, filename] : loadedEmitter) {
        emitters_[filename] = std::make_unique<Emitter>(dxSrvArray_.get(), filename);
        emitters_[filename]->Init();
    }
}

void ParticleManager::Finalize() {
    if (dxCommand_) {
        dxCommand_->Finalize();
    }
    if (dxSrvArray_) {
        dxSrvArray_->Finalize();
    }
    emitters_.clear();
}

void ParticleManager::PreDraw() {
    auto* commandList = dxCommand_->getCommandList();

    commandList->SetGraphicsRootSignature(pso_[int(blendMode_)]->rootSignature.Get());
    commandList->SetPipelineState(pso_[int(blendMode_)]->pipelineState.Get());

    CameraManager::getInstance()->setBufferForRootParameter(commandList, 1);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ParticleManager::DrawDebug() {
    for (auto& [emitterName, emitter] : emitters_) {
        emitter->Draw();
    }
}

void ParticleManager::ChangeBlendMode(BlendMode mode) {
    if (blendMode_ == mode) {
        return;
    }
    blendMode_ = mode;

    auto* commandList = dxCommand_->getCommandList();

    commandList->SetGraphicsRootSignature(pso_[int(blendMode_)]->rootSignature.Get());
    commandList->SetPipelineState(pso_[int(blendMode_)]->pipelineState.Get());
}

ParticleManager::ParticleManager() {}

ParticleManager::~ParticleManager() {}

void ParticleManager::CreatePso() {
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

std::unique_ptr<Emitter> ParticleManager::CreateEmitter(DxSrvArray* srvArray, const std::string& name) const {
    std::unique_ptr<Emitter> result = std::make_unique<Emitter>(srvArray, name);
    result->Init();
    return result;
}

#ifdef _DEBUG
void ParticleManager::Edit() {
    // main window
    if (ImGui::Begin("ParticleManager")) {
        if (ImGui::Button("Create New Emitter")) {
            isOpenedCrateWindow_ = true;
        }
    }
    ImGui::End();

    for (auto& [emitterName, emitter] : emitters_) {
        emitter->Debug();
        emitter->Update(Engine::getInstance()->getDeltaTime());
    }

    if (isOpenedCrateWindow_) {
        ImGui::Begin("Create New", &isOpenedCrateWindow_);
        ImGui::InputText("name", &newInstanceName_[0], sizeof(char) * 64, ImGuiInputTextFlags_CharsNoBlank);

        // 終端文字で切り詰める
        newInstanceName_ = std::string(newInstanceName_.c_str());

        if (ImGui::Button("Create")) {
            // 名前が既に存在している場合は登録しない
            if (emitters_.find(newInstanceName_) == emitters_.end()) {
                emitters_[newInstanceName_] = std::make_unique<Emitter>(dxSrvArray_.get(), newInstanceName_);
                emitters_[newInstanceName_]->Init();
            }
            emitterWindowedState_ = false;
            isOpenedCrateWindow_  = false;
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            // window を閉じる
            isOpenedCrateWindow_ = false;
        }
        ImGui::End();
    } else {
        // 作成用文字列の初期化
        newInstanceName_ = "NULL";
    }
}
#endif // _DEBUG
