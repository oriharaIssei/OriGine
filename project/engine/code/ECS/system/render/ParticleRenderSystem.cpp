#include "ParticleRenderSystem.h"

/// engine
#include "Engine.h"
#include "scene/SceneManager.h"

// directX12
#include "directX12/DxDevice.h"

// module
#include "camera/CameraManager.h"

void ParticleRenderSystem::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
    CreatePso();

    for (size_t i = 0; i < kBlendNum; ++i) {
        BlendMode blend = static_cast<BlendMode>(i);
        activeEmittersByBlendMode_[blend].reserve(100);
    }
}

void ParticleRenderSystem::Update() {
    eraseDeadEntity();

    if (entityIDs_.empty()) {
        return;
    }

    activeEmittersByBlendMode_.clear();

    for (auto& id : entityIDs_) {
        GameEntity* entity = getEntity(id);
        DispatchRenderer(entity);
    }

    // アクティブなエミッターが一つもなければ終了
    bool isSkip = true;
    for (const auto& [_, emitters] : activeEmittersByBlendMode_) {
        if (!emitters.empty()) {
            isSkip = false;
            break;
        }
    }
    if (isSkip) {
        return;
    }

    for (size_t i = 0; i < kBlendNum; ++i) {
        BlendMode blend = static_cast<BlendMode>(i);
        RenderingBy(blend);
    }
}

void ParticleRenderSystem::DispatchRenderer(GameEntity* _entity) {
    if (_entity == nullptr) {
        return;
    }
    Transform* transform           = getComponent<Transform>(_entity);
    std::vector<Emitter>* emitters = getComponents<Emitter>(_entity);
    if (emitters == nullptr) {
        return;
    }
    for (auto& emitter : *emitters) {
        if (!emitter.IsActive()) {
            return;
        }
        BlendMode blend = emitter.getBlendMode();
        emitter.setParent(transform);
        activeEmittersByBlendMode_[blend].push_back(&emitter);
    }
}
void ParticleRenderSystem::RenderingBy(BlendMode _blend) {
    auto& emitters = activeEmittersByBlendMode_[_blend];
    if (emitters.empty()) {
        return;
    }
    // PSOセット
    dxCommand_->getCommandList()->SetPipelineState(pso_[_blend]->pipelineState.Get());
    // RootSignatureセット
    dxCommand_->getCommandList()->SetGraphicsRootSignature(pso_[_blend]->rootSignature.Get());

    StartRender();

    // プリミティブトポロジーの設定
    dxCommand_->getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    for (auto* emitter : emitters) {
        if (emitter == nullptr) {
            continue;
        }
        emitter->Draw(dxCommand_->getCommandList());
    }
}

void ParticleRenderSystem::Finalize() {
    dxCommand_->Finalize();
}

void ParticleRenderSystem::CreatePso() {
    ShaderManager* shaderManager = ShaderManager::getInstance();

    // 登録されているかどうかをチェック
    /*if (shaderManager->IsRegisteredPipelineStateObj("Particle_" + blendModeStr[0])) {
        for (size_t i = 0; i < kBlendNum; ++i) {
            BlendMode blend = static_cast<BlendMode>(i);
            if (pso_[blend]) {
                continue;
            }
            pso_[blend] = shaderManager->getPipelineStateObj("Particle_" + blendModeStr[i]);
        }
        return;
    }*/

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
    shaderInfo.setDescriptorRange2Parameter(structuredRange, 1, 0);
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
    shaderInfo.setDescriptorRange2Parameter(descriptorRange, 1, rootParameterIndex);

#pragma endregion

#pragma region "Input Element"
    D3D12_INPUT_ELEMENT_DESC inputElementDesc = {};
    inputElementDesc.SemanticName             = "POSITION"; /*Semantics*/
    inputElementDesc.SemanticIndex            = 0; /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format                   = DXGI_FORMAT_R32G32B32A32_FLOAT; // float 4
    inputElementDesc.AlignedByteOffset        = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDesc);
    inputElementDesc.SemanticName      = "TEXCOORD"; /*Semantics*/
    inputElementDesc.SemanticIndex     = 0; /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format            = DXGI_FORMAT_R32G32_FLOAT; // float 2
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDesc);
    inputElementDesc.SemanticName      = "NORMAL"; /*Semantics*/
    inputElementDesc.SemanticIndex     = 0; /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32_FLOAT; // float 4
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

    shaderInfo.customDepthStencilDesc().DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

    ///=================================================
    /// BlendMode ごとの Pso作成
    ///=================================================
    for (size_t i = 0; i < kBlendNum; i++) {
        shaderInfo.blendMode_       = BlendMode(i);
        pso_[shaderInfo.blendMode_] = shaderManager->CreatePso("Particle_" + blendModeStr[i], shaderInfo, Engine::getInstance()->getDxDevice()->getDevice());
    }
}

void ParticleRenderSystem::StartRender() {
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand_->getCommandList();
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    CameraManager::getInstance()->setBufferForRootParameter(commandList, 1);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::getInstance()->getSrvHeap()->getHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
}
