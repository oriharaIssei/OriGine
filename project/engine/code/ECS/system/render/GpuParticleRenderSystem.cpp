#include "GpuParticleRenderSystem.h"

/// engine
#include "Engine.h"

#include "camera/CameraManager.h"
#include "scene/SceneManager.h"
#include "texture/TextureManager.h"

// directX12
#include "directX12/DxDevice.h"

using namespace OriGine;

GpuParticleRenderSystem::GpuParticleRenderSystem() : BaseRenderSystem() {}
GpuParticleRenderSystem::~GpuParticleRenderSystem() {}

void GpuParticleRenderSystem::Initialize() {
    constexpr size_t defaultReserveSize = 100;

    BaseRenderSystem::Initialize();

    // buffer作成
    perViewBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);

    // BlendModeごとのコンテナ準備
    for (size_t i = 0; i < static_cast<size_t>(BlendMode::Count); ++i) {
        activeEmitterByBlendMode_[i].reserve(defaultReserveSize);
    }
}

void GpuParticleRenderSystem::Finalize() {
    dxCommand_->Finalize();

    for (auto& pso : psoByBlendMode_) {
        pso->Finalize();
    }

    perViewBuffer_.Finalize();
}

void GpuParticleRenderSystem::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::GetInstance();

    // 登録されているかどうかをチェック
    if (shaderManager->IsRegisteredPipelineStateObj("GpuParticle_" + kBlendModeStr[0])) {
        for (size_t i = 0; i < kBlendNum; ++i) {
            if (psoByBlendMode_[i]) {
                continue;
            }
            psoByBlendMode_[i] = shaderManager->GetPipelineStateObj("GpuParticle_" + kBlendModeStr[i]);
        }
        return;
    }

    ///=================================================
    /// shader読み込み
    ///=================================================

    shaderManager->LoadShader("GpuParticle.VS");
    shaderManager->LoadShader("Particle.PS", kShaderDirectory, L"ps_6_0");

    ///=================================================
    /// shader情報の設定
    ///=================================================
    ShaderInfo shaderInfo;
    shaderInfo.vsKey = "GpuParticle.VS";
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

    // 1 ... PerView
    rootParameter[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameter[1].Descriptor.ShaderRegister = 0;
    shaderInfo.pushBackRootParameter(rootParameter[1]);
    // 2 ... Material
    rootParameter[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[2].Descriptor.ShaderRegister = 0;
    shaderInfo.pushBackRootParameter(rootParameter[2]);

    // 3 ... Texture
    // DescriptorTable を使う
    rootParameter[3].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[3].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t rootParameterIndex                            = shaderInfo.pushBackRootParameter(rootParameter[3]);
    D3D12_DESCRIPTOR_RANGE descriptorRange[1]            = {};
    descriptorRange[0].BaseShaderRegister                = 0;
    descriptorRange[0].NumDescriptors                    = 1;
    descriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(descriptorRange, 1, rootParameterIndex);

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

    shaderInfo.CustomDepthStencilDesc().DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

    ///=================================================
    /// BlendMode ごとの Pso作成
    ///=================================================
    for (size_t i = 0; i < kBlendNum; i++) {
        shaderInfo.blendMode_ = BlendMode(i);
        psoByBlendMode_[i]    = shaderManager->CreatePso("Particle_" + kBlendModeStr[i], shaderInfo, Engine::GetInstance()->GetDxDevice()->device_);
    }
}

void GpuParticleRenderSystem::StartRender() {

    // コマンドリスト取得
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand_->GetCommandList();
    // プリミティブトポロジー設定
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // デスクリプタヒープ設定
    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);

    // カメラ情報更新
    const CameraTransform& cameraTransform = CameraManager::GetInstance()->GetTransform();
    // カメラの回転行列を取得し、平行移動成分をゼロにする
    Matrix4x4 cameraRotationMat = cameraTransform.viewMat;
    cameraRotationMat[3][0]     = 0.0f;
    cameraRotationMat[3][1]     = 0.0f;
    cameraRotationMat[3][2]     = 0.0f;
    cameraRotationMat[3][3]     = 1.0f;

    // カメラの回転行列を反転してワールド空間への変換行列を作成
    perViewBuffer_->billboardMat      = cameraRotationMat.inverse();
    perViewBuffer_->viewProjectionMat = cameraTransform.viewMat * cameraTransform.projectionMat;
    perViewBuffer_.ConvertToBuffer();

    perViewBuffer_.SetForRootParameter(dxCommand_->GetCommandList(), 1);
}

void GpuParticleRenderSystem::DispatchRenderer(EntityHandle _entity) {
    auto& components = GetComponents<GpuParticleEmitter>(_entity);
    if (components.empty()) {
        return;
    }

    // アクティブなエミッタをBlendModeごとに振り分ける
    for (auto& comp : components) {
        if (!comp.IsActive()) {
            continue;
        }
        activeEmitterByBlendMode_[static_cast<size_t>(comp.GetBlendMode())].emplace_back(&comp);
    }
}

bool GpuParticleRenderSystem::ShouldSkipRender() const {
    bool isSkip = true;
    for (const auto& emitters : activeEmitterByBlendMode_) {
        if (!emitters.empty()) {
            isSkip = false;
            break;
        }
    }
    return isSkip;
}

void GpuParticleRenderSystem::RenderingBy(BlendMode _blendMode, bool /*_isCulling*/) {
    int32_t blendModeIndex = static_cast<int32_t>(_blendMode);
    if (activeEmitterByBlendMode_[blendModeIndex].empty()) {
        return;
    }

    auto& commandList = dxCommand_->GetCommandList();

    commandList->SetGraphicsRootSignature(psoByBlendMode_[blendModeIndex]->rootSignature.Get());
    commandList->SetPipelineState(psoByBlendMode_[blendModeIndex]->pipelineState.Get());

    for (auto& emitter : activeEmitterByBlendMode_[blendModeIndex]) {
        commandList->SetGraphicsRootDescriptorTable(
            0,
            emitter->GetParticleSrvDescriptor().GetGpuHandle());

        emitter->GetMaterialBuffer().ConvertToBuffer();
        emitter->GetMaterialBuffer().SetForRootParameter(commandList, 2);

        commandList->SetGraphicsRootDescriptorTable(
            3,
            TextureManager::GetDescriptorGpuHandle(emitter->GetTextureIndex()));

        const auto& particleMesh = emitter->GetMesh();
        commandList->IASetVertexBuffers(0, 1, &particleMesh.GetVBView());
        commandList->IASetIndexBuffer(&particleMesh.GetIBView());

        // 描画!!!
        commandList->DrawIndexedInstanced(UINT(particleMesh.GetIndexSize()), static_cast<UINT>(emitter->GetParticleSize()), 0, 0, 0);
    }

    // 描画後クリア
    activeEmitterByBlendMode_[blendModeIndex].clear();
}
