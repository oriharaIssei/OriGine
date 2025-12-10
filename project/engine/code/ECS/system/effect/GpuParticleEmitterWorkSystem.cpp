#include "GpuParticleEmitterWorkSystem.h"

/// engine
#define ENGINE_INCLUDE
#include <EngineInclude.h>
// DirectX12
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/DxFence.h"
#include "directX12/ShaderManager.h"

using namespace OriGine;

GpuParticleEmitterWorkSystem::GpuParticleEmitterWorkSystem() : ISystem(SystemCategory::Effect) {}
GpuParticleEmitterWorkSystem::~GpuParticleEmitterWorkSystem() {}

void GpuParticleEmitterWorkSystem::Initialize() {
    constexpr int32_t initialReserveCount = 100;
    dxCommand_                            = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
    perFrameBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);

    workEmitters_.reserve(initialReserveCount);

    CreatePso();
}

void GpuParticleEmitterWorkSystem::Update() {
    if (entityIDs_.empty()) {
        return;
    }
    ISystem::EraseDeadEntity();

    /// Update する必要があるかどうか調べる
    {
        if (!workEmitters_.empty()) {
            workEmitters_.clear();
        }

        auto* emitterArray = GetComponentArray<GpuParticleEmitter>();
        if (!emitterArray) {
            return; // エミッターが存在しない場合は何もしない
        }
        for (auto& componentVec : emitterArray->GetAllComponents()) {
            for (auto& comp : componentVec) {
                if (comp.IsActive()) {
                    workEmitters_.push_back(&comp);
                }
            }
        }
        if (workEmitters_.empty()) {
            return; // アクティブなエミッターがない場合は何もしない
        }
    }

    /// ==========================================
    // PerFrame Buffer Update
    /// ==========================================
    perFrameBuffer_->deltaTime = Engine::GetInstance()->GetDeltaTime();
    perFrameBuffer_->time      = perFrameBuffer_->deltaTime;
    perFrameBuffer_.ConvertToBuffer();

    /// ==========================================
    // Emit CS
    /// ==========================================
    StartCS(emitGpuParticlePso_);
    for (auto& emitter : workEmitters_) {
        if (!emitter) {
            continue; // エミッターが無効な場合はスキップ
        }
        EmitParticle(emitter);
    }

    /// ==========================================
    // Update CS
    /// ==========================================
    StartCS(updateGpuParticlePso_);
    for (auto& emitter : workEmitters_) {
        if (!emitter) {
            continue; // エミッターが無効な場合はスキップ
        }
        UpdateParticle(emitter);
    }

    ExecuteCS();
}

void GpuParticleEmitterWorkSystem::Finalize() {
    if (dxCommand_) {
        dxCommand_->Finalize();
        dxCommand_.reset();
    }

    if (perFrameBuffer_) {
        perFrameBuffer_.Finalize();
    }

    if (emitGpuParticlePso_) {
        emitGpuParticlePso_ = nullptr;
    }
    if (updateGpuParticlePso_) {
        updateGpuParticlePso_ = nullptr;
    }
}

void GpuParticleEmitterWorkSystem::UpdateParticle(GpuParticleEmitter* _emitter) {
    auto& commandList = dxCommand_->GetCommandList();

    commandList->SetComputeRootDescriptorTable(
        particlesDataIndex,
        _emitter->GetParticleUavDescriptor().GetGpuHandle());

    commandList->SetComputeRootDescriptorTable(
        freeIndexBufferIndex,
        _emitter->GetFreeIndexUavDescriptor().GetGpuHandle());

    commandList->SetComputeRootDescriptorTable(
        freeListBufferIndex,
        _emitter->GetFreeListUavDescriptor().GetGpuHandle());

    dxCommand_->ResourceBarrier(
        _emitter->GetParticleResource().GetResource(),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

    D3D12_RESOURCE_BARRIER particleResourceBarrier = {};
    particleResourceBarrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    particleResourceBarrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;

    dxCommand_->ResourceDirectBarrier(
        _emitter->GetFreeListResource().GetResource(), particleResourceBarrier);

    dxCommand_->ResourceDirectBarrier(
        _emitter->GetFreeIndexResource().GetResource(), particleResourceBarrier);

    UINT dispatchCount = (_emitter->GetParticleSize() + 1023) / 1024;
    commandList->Dispatch(
        dispatchCount, // 1ワークグループあたり1024頂点を処理
        1, 1);
}

void GpuParticleEmitterWorkSystem::EmitParticle(GpuParticleEmitter* _emitter) {
    auto& commandList = dxCommand_->GetCommandList();

    auto& emitterData = _emitter->GetShapeBufferDataRef();
    emitterData.frequency -= perFrameBuffer_->deltaTime;
    emitterData.isEmit = 0;

    if (emitterData.frequency < 0.0f) {
        emitterData.frequency = emitterData.frequencyTime;
        emitterData.isEmit    = 1;
    }

    auto& buffer = _emitter->GetShapeBuffer();
    buffer.ConvertToBuffer();

    buffer.SetForComputeRootParameter(
        dxCommand_->GetCommandList(), kEmitterShapeIndex);

    buffer.ConvertToBuffer();
    buffer.SetForComputeRootParameter(
        dxCommand_->GetCommandList(), kEmitterShapeIndex);

    commandList->SetComputeRootDescriptorTable(
        particlesDataIndex,
        _emitter->GetParticleUavDescriptor().GetGpuHandle());

    commandList->SetComputeRootDescriptorTable(
        freeIndexBufferIndex,
        _emitter->GetFreeIndexUavDescriptor().GetGpuHandle());

    commandList->SetComputeRootDescriptorTable(
        freeListBufferIndex,
        _emitter->GetFreeListUavDescriptor().GetGpuHandle());

    D3D12_RESOURCE_BARRIER particleResourceBarrier = {};
    particleResourceBarrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    particleResourceBarrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;

    dxCommand_->ResourceDirectBarrier(
        _emitter->GetParticleResource().GetResource(), particleResourceBarrier);

    dxCommand_->ResourceDirectBarrier(
        _emitter->GetFreeListResource().GetResource(), particleResourceBarrier);

    dxCommand_->ResourceDirectBarrier(
        _emitter->GetFreeIndexResource().GetResource(), particleResourceBarrier);

    commandList->Dispatch(1, 1, 1); // Emitter 一つにつき 1 スレッドで処理
}

void GpuParticleEmitterWorkSystem::CreatePso() {
    CreateEmitGpuParticlePso();
    CreateUpdateGpuParticlePso();
}

void GpuParticleEmitterWorkSystem::CreateEmitGpuParticlePso() {
    constexpr const char* psoKey          = "EmitGpuParticle.CS";
    constexpr int32_t kRootParameterCount = 5;

    if (emitGpuParticlePso_) {
        return; // PSOが既に作成されている場合は何もしない
    }

    ShaderManager* shaderManager = ShaderManager::GetInstance();
    DxDevice* dxDevice           = Engine::GetInstance()->GetDxDevice();

    if (shaderManager->IsRegisteredPipelineStateObj(psoKey)) {
        emitGpuParticlePso_ = shaderManager->GetPipelineStateObj(psoKey);
        return; // PSOが既に登録されている場合はそれを使用
    }

    // PSOが登録されていない場合は新規に作成

    /// ==========================================
    // Shader 読み込み
    /// ==========================================
    shaderManager->LoadShader(psoKey, kShaderDirectory, L"cs_6_0");

    ShaderInfo shaderInfo{};
    shaderInfo.csKey = psoKey;

#pragma region "ROOT_PARAMETER"
    D3D12_ROOT_PARAMETER rootParameters[kRootParameterCount]{};
    rootParameters[particlesDataIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[particlesDataIndex].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[particlesDataIndex]);

    D3D12_DESCRIPTOR_RANGE particlesDataDescriptorRange[1]            = {};
    particlesDataDescriptorRange[0].BaseShaderRegister                = 0; // u0
    particlesDataDescriptorRange[0].NumDescriptors                    = 1;
    particlesDataDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    particlesDataDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(
        particlesDataDescriptorRange, 1, particlesDataIndex);

    rootParameters[freeIndexBufferIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[freeIndexBufferIndex].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[freeIndexBufferIndex]);

    D3D12_DESCRIPTOR_RANGE freeIndexDescriptorRange[1]            = {};
    freeIndexDescriptorRange[0].BaseShaderRegister                = 1; // u1
    freeIndexDescriptorRange[0].NumDescriptors                    = 1;
    freeIndexDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    freeIndexDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(
        freeIndexDescriptorRange, 1, freeIndexBufferIndex);

    rootParameters[freeListBufferIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[freeListBufferIndex].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[freeListBufferIndex]);

    D3D12_DESCRIPTOR_RANGE freeListDescriptorRange[1]            = {};
    freeListDescriptorRange[0].BaseShaderRegister                = 2; // u2
    freeListDescriptorRange[0].NumDescriptors                    = 1;
    freeListDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    freeListDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(
        freeListDescriptorRange, 1, freeListBufferIndex);

    rootParameters[kEmitterShapeIndex].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[kEmitterShapeIndex].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[kEmitterShapeIndex].Descriptor.ShaderRegister = 0; // b0
    shaderInfo.pushBackRootParameter(rootParameters[kEmitterShapeIndex]);

    rootParameters[perFrameBufferIndex].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[perFrameBufferIndex].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[perFrameBufferIndex].Descriptor.ShaderRegister = 1; // b1
    shaderInfo.pushBackRootParameter(rootParameters[perFrameBufferIndex]);

#pragma endregion

    /// ==========================================
    // PSOの作成
    /// ==========================================
    emitGpuParticlePso_ = shaderManager->CreatePso(psoKey, shaderInfo, dxDevice->device_);
}

void GpuParticleEmitterWorkSystem::CreateUpdateGpuParticlePso() {
    constexpr const char* psoKey          = "UpdateGpuParticle.CS";
    constexpr int32_t kRootParameterCount = 5;

    if (updateGpuParticlePso_) {
        return; // PSOが既に作成されている場合は何もしない
    }

    ShaderManager* shaderManager = ShaderManager::GetInstance();
    DxDevice* dxDevice           = Engine::GetInstance()->GetDxDevice();

    if (shaderManager->IsRegisteredPipelineStateObj(psoKey)) {
        updateGpuParticlePso_ = shaderManager->GetPipelineStateObj(psoKey);
        return; // PSOが既に登録されている場合はそれを使用
    }

    // PSOが登録されていない場合は新規に作成

    /// ==========================================
    // Shader 読み込み
    /// ==========================================
    shaderManager->LoadShader(psoKey, kShaderDirectory, L"cs_6_0");

    ShaderInfo shaderInfo{};
    shaderInfo.csKey = psoKey;

#pragma region "ROOT_PARAMETER"
    D3D12_ROOT_PARAMETER rootParameters[kRootParameterCount]{};
    rootParameters[particlesDataIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[particlesDataIndex].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[particlesDataIndex]);

    D3D12_DESCRIPTOR_RANGE particlesDataDescriptorRange[1]            = {};
    particlesDataDescriptorRange[0].BaseShaderRegister                = 0; // u0
    particlesDataDescriptorRange[0].NumDescriptors                    = 1;
    particlesDataDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    particlesDataDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(
        particlesDataDescriptorRange, 1, particlesDataIndex);

    rootParameters[freeIndexBufferIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[freeIndexBufferIndex].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[freeIndexBufferIndex]);

    D3D12_DESCRIPTOR_RANGE freeIndexDescriptorRange[1]            = {};
    freeIndexDescriptorRange[0].BaseShaderRegister                = 1; // u1
    freeIndexDescriptorRange[0].NumDescriptors                    = 1;
    freeIndexDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    freeIndexDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(
        freeIndexDescriptorRange, 1, freeIndexBufferIndex);

    rootParameters[freeListBufferIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[freeListBufferIndex].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[freeListBufferIndex]);

    D3D12_DESCRIPTOR_RANGE freeListDescriptorRange[1]            = {};
    freeListDescriptorRange[0].BaseShaderRegister                = 2; // u2
    freeListDescriptorRange[0].NumDescriptors                    = 1;
    freeListDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    freeListDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(
        freeListDescriptorRange, 1, freeListBufferIndex);

    rootParameters[kEmitterShapeIndex].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[kEmitterShapeIndex].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[kEmitterShapeIndex].Descriptor.ShaderRegister = 0; // b0
    shaderInfo.pushBackRootParameter(rootParameters[kEmitterShapeIndex]);

    rootParameters[perFrameBufferIndex].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[perFrameBufferIndex].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[perFrameBufferIndex].Descriptor.ShaderRegister = 1; // b0
    shaderInfo.pushBackRootParameter(rootParameters[perFrameBufferIndex]);

#pragma endregion

    /// ==========================================
    // PSOの作成
    /// ==========================================
    updateGpuParticlePso_ = shaderManager->CreatePso(psoKey, shaderInfo, dxDevice->device_);
}

void GpuParticleEmitterWorkSystem::StartCS(PipelineStateObj* _pso) {
    if (!_pso) {
        LOG_ERROR("emitGpuParticlePso is not created for SkinningAnimationSystem");
        return;
    }

    dxCommand_->GetCommandList()->SetPipelineState(_pso->pipelineState.Get());
    dxCommand_->GetCommandList()->SetComputeRootSignature(_pso->rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = {
        Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    dxCommand_->GetCommandList()->SetDescriptorHeaps(1, ppHeaps);

    // emit も update も使うため ここで Set
    perFrameBuffer_.SetForComputeRootParameter(dxCommand_->GetCommandList(), perFrameBufferIndex);
}

void GpuParticleEmitterWorkSystem::ExecuteCS() {
    HRESULT hr;
    DxFence* fence = Engine::GetInstance()->GetDxFence();

    // コマンドの受付終了 -----------------------------------
    hr = dxCommand_->Close();
    if (FAILED(hr)) {
        LOG_ERROR("Failed to close command list. HRESULT: {}", std::to_string(hr));
        assert(false);
    }
    //----------------------------------------------------

    ///===============================================================
    /// コマンドリストの実行
    ///===============================================================
    dxCommand_->ExecuteCommand();
    ///===============================================================

    ///===============================================================
    /// コマンドリストの実行を待つ
    ///===============================================================
    UINT64 fenceVal = fence->Signal(dxCommand_->GetCommandQueue());
    fence->WaitForFence(fenceVal);
    ///===============================================================

    ///===============================================================
    /// リセット
    ///===============================================================
    dxCommand_->CommandReset();
    ///===============================================================
}
