#include "GpuParticleEmitterWorkSystem.h"

/// engine
#define ENGINE_INCLUDE
#define DELTA_TIME
#include "EngineInclude.h"
// DirectX12
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/DxFence.h"
#include "directX12/ShaderManager.h"

/// ECS
// component
#include "component/effect/particle/gpuParticle/GpuParticle.h"

GpuParticleEmitterWorkSystem::GpuParticleEmitterWorkSystem() : ISystem(SystemCategory::Effect) {}

GpuParticleEmitterWorkSystem::~GpuParticleEmitterWorkSystem() {}

void GpuParticleEmitterWorkSystem::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
    perFrameBuffer_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

    CreatePso();
}

void GpuParticleEmitterWorkSystem::Update() {
    if (entityIDs_.empty()) {
        return;
    }
    ISystem::eraseDeadEntity();

    /// ==========================================
    // PerFrame Buffer Update
    /// ==========================================
    perFrameBuffer_->deltaTime = getMainDeltaTime();
    perFrameBuffer_->time      = perFrameBuffer_->deltaTime;
    perFrameBuffer_.ConvertToBuffer();

    /// ==========================================
    // Emit CS
    /// ==========================================
    StartCS(emitGpuParticlePso_);
    for (auto& id : entityIDs_) {
        GameEntity* entity = getEntity(id);
        if (!entity) {
            continue;
        }
        EmitParticle(entity);
    }
    ExecuteCS();

    /// ==========================================
    // Update CS
    /// ==========================================
    StartCS(updateGpuParticlePso_);
    for (auto& id : entityIDs_) {
        GameEntity* entity = getEntity(id);
        if (!entity) {
            continue;
        }
        UpdateEntity(entity);
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

void GpuParticleEmitterWorkSystem::UpdateEntity(GameEntity* entity) {
    auto& commandList = dxCommand_->getCommandList();
    auto emitterVec   = getComponents<GpuParticleEmitter>(entity);

    for (auto emitterItr = emitterVec->begin();
        emitterItr != emitterVec->end();
        ++emitterItr) {
        auto& emitter = *emitterItr;

        if (!emitter.isActive()) {
            continue; // パーティクルエミッターがアクティブでない場合はスキップ
        }

        commandList->SetComputeRootDescriptorTable(
            particlesDataIndex,
            emitter.getParticleUavDescriptor()->getGpuHandle());

        commandList->SetComputeRootDescriptorTable(
            freeIndexBufferIndex,
            emitter.getFreeIndexUavDescriptor()->getGpuHandle());

        commandList->SetComputeRootDescriptorTable(
            freeListBufferIndex,
            emitter.getFreeListUavDescriptor()->getGpuHandle());

        dxCommand_->ResourceBarrier(
            emitter.getParticleResource().getResource(),
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        D3D12_RESOURCE_BARRIER particleResourceBarrier = {};
        particleResourceBarrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        particleResourceBarrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;

        dxCommand_->ResourceDirectBarrier(
            emitter.getFreeListResource().getResource(), particleResourceBarrier);

        dxCommand_->ResourceDirectBarrier(
            emitter.getFreeIndexResource().getResource(), particleResourceBarrier);

        UINT dispatchCount = (emitter.getParticleSize() + 1023) / 1024;
        commandList->Dispatch(
            dispatchCount, // 1ワークグループあたり1024頂点を処理
            1, 1);
    }
}

void GpuParticleEmitterWorkSystem::EmitParticle(GameEntity* entity) {
    auto& commandList = dxCommand_->getCommandList();
    auto emitterVec   = getComponents<GpuParticleEmitter>(entity);

    if (!emitterVec) {
        return;
    }

    for (auto emitterItr = emitterVec->begin();
        emitterItr != emitterVec->end();
        ++emitterItr) {
        auto& emitter = *emitterItr;

        if (!emitter.isActive()) {
            continue; // パーティクルエミッターがアクティブでない場合はスキップ
        }

        auto& emitterData = emitter.getShapeBufferDataRef();
        emitterData.frequency -= perFrameBuffer_->deltaTime;
        emitterData.isEmit = 0;
        if (emitterData.frequency < 0.0f) {
            emitterData.frequency = emitterData.frequencyTime;
            emitterData.isEmit    = 1;
        }

        emitter.getShapeBuffer().ConvertToBuffer();

        emitter.getShapeBuffer().SetForComputeRootParameter(
            dxCommand_->getCommandList(), emitterShapeIndex);

        emitter.getShapeBuffer().ConvertToBuffer();
        emitter.getShapeBuffer().SetForComputeRootParameter(
            dxCommand_->getCommandList(), emitterShapeIndex);

        commandList->SetComputeRootDescriptorTable(
            particlesDataIndex,
            emitter.getParticleUavDescriptor()->getGpuHandle());

        commandList->SetComputeRootDescriptorTable(
            freeIndexBufferIndex,
            emitter.getFreeIndexUavDescriptor()->getGpuHandle());

        commandList->SetComputeRootDescriptorTable(
            freeListBufferIndex,
            emitter.getFreeListUavDescriptor()->getGpuHandle());

        D3D12_RESOURCE_BARRIER particleResourceBarrier = {};
        particleResourceBarrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        particleResourceBarrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;

        dxCommand_->ResourceDirectBarrier(
            emitter.getParticleResource().getResource(), particleResourceBarrier);

        dxCommand_->ResourceDirectBarrier(
            emitter.getFreeListResource().getResource(), particleResourceBarrier);

        dxCommand_->ResourceDirectBarrier(
            emitter.getFreeIndexResource().getResource(), particleResourceBarrier);

        commandList->Dispatch(1, 1, 1); // Emitter 一つにつき 1 スレッドで処理
    }
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

    ShaderManager* shaderManager = ShaderManager::getInstance();
    DxDevice* dxDevice           = Engine::getInstance()->getDxDevice();

    if (shaderManager->IsRegistertedPipelineStateObj(psoKey)) {
        emitGpuParticlePso_ = shaderManager->getPipelineStateObj(psoKey);
        return; // PSOが既に登録されている場合はそれを使用
    }

    // PSOが登録されていない場合は新規に作成

    /// ==========================================
    // Shader 読み込み
    /// ==========================================
    shaderManager->LoadShader(psoKey, shaderDirectory, L"cs_6_0");

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
    shaderInfo.setDescriptorRange2Parameter(
        particlesDataDescriptorRange, 1, particlesDataIndex);

    rootParameters[freeIndexBufferIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[freeIndexBufferIndex].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[freeIndexBufferIndex]);

    D3D12_DESCRIPTOR_RANGE freeIndexDescriptorRange[1]            = {};
    freeIndexDescriptorRange[0].BaseShaderRegister                = 1; // u1
    freeIndexDescriptorRange[0].NumDescriptors                    = 1;
    freeIndexDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    freeIndexDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.setDescriptorRange2Parameter(
        freeIndexDescriptorRange, 1, freeIndexBufferIndex);

    rootParameters[freeListBufferIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[freeListBufferIndex].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[freeListBufferIndex]);

    D3D12_DESCRIPTOR_RANGE freeListDescriptorRange[1]            = {};
    freeListDescriptorRange[0].BaseShaderRegister                = 2; // u2
    freeListDescriptorRange[0].NumDescriptors                    = 1;
    freeListDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    freeListDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.setDescriptorRange2Parameter(
        freeListDescriptorRange, 1, freeListBufferIndex);

    rootParameters[perFrameBufferIndex].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[perFrameBufferIndex].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[perFrameBufferIndex].Descriptor.ShaderRegister = 1; // b1
    shaderInfo.pushBackRootParameter(rootParameters[perFrameBufferIndex]);

    rootParameters[emitterShapeIndex].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[emitterShapeIndex].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[emitterShapeIndex].Descriptor.ShaderRegister = 0; // b0
    shaderInfo.pushBackRootParameter(rootParameters[emitterShapeIndex]);

#pragma endregion

    /// ==========================================
    // PSOの作成
    /// ==========================================
    emitGpuParticlePso_ = shaderManager->CreatePso(psoKey, shaderInfo, dxDevice->getDevice());
}

void GpuParticleEmitterWorkSystem::CreateUpdateGpuParticlePso() {
    constexpr const char* psoKey          = "UpdateGpuParticle.CS";
    constexpr int32_t kRootParameterCount = 4;

    if (updateGpuParticlePso_) {
        return; // PSOが既に作成されている場合は何もしない
    }

    ShaderManager* shaderManager = ShaderManager::getInstance();
    DxDevice* dxDevice           = Engine::getInstance()->getDxDevice();

    if (shaderManager->IsRegistertedPipelineStateObj(psoKey)) {
        updateGpuParticlePso_ = shaderManager->getPipelineStateObj(psoKey);
        return; // PSOが既に登録されている場合はそれを使用
    }

    // PSOが登録されていない場合は新規に作成

    /// ==========================================
    // Shader 読み込み
    /// ==========================================
    shaderManager->LoadShader(psoKey, shaderDirectory, L"cs_6_0");

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
    shaderInfo.setDescriptorRange2Parameter(
        particlesDataDescriptorRange, 1, particlesDataIndex);

    rootParameters[freeIndexBufferIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[freeIndexBufferIndex].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[freeIndexBufferIndex]);

    D3D12_DESCRIPTOR_RANGE freeIndexDescriptorRange[1]            = {};
    freeIndexDescriptorRange[0].BaseShaderRegister                = 1; // u1
    freeIndexDescriptorRange[0].NumDescriptors                    = 1;
    freeIndexDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    freeIndexDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.setDescriptorRange2Parameter(
        freeIndexDescriptorRange, 1, freeIndexBufferIndex);

    rootParameters[freeListBufferIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[freeListBufferIndex].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[freeListBufferIndex]);

    D3D12_DESCRIPTOR_RANGE freeListDescriptorRange[1]            = {};
    freeListDescriptorRange[0].BaseShaderRegister                = 2; // u2
    freeListDescriptorRange[0].NumDescriptors                    = 1;
    freeListDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    freeListDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.setDescriptorRange2Parameter(
        freeListDescriptorRange, 1, freeListBufferIndex);

    rootParameters[perFrameBufferIndex].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[perFrameBufferIndex].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[perFrameBufferIndex].Descriptor.ShaderRegister = 0; // b0
    shaderInfo.pushBackRootParameter(rootParameters[perFrameBufferIndex]);
#pragma endregion

    /// ==========================================
    // PSOの作成
    /// ==========================================
    updateGpuParticlePso_ = shaderManager->CreatePso(psoKey, shaderInfo, dxDevice->getDevice());
}

void GpuParticleEmitterWorkSystem::StartCS(PipelineStateObj* _pso) {
    if (!_pso) {
        LOG_ERROR("emitGpuParticlePso is not created for SkinningAnimationSystem");
        return;
    }

    dxCommand_->getCommandList()->SetPipelineState(_pso->pipelineState.Get());
    dxCommand_->getCommandList()->SetComputeRootSignature(_pso->rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = {
        Engine::getInstance()->getSrvHeap()->getHeap().Get()};
    dxCommand_->getCommandList()->SetDescriptorHeaps(1, ppHeaps);

    // emit も update も使うため ここで Set
    perFrameBuffer_.SetForComputeRootParameter(dxCommand_->getCommandList(), perFrameBufferIndex);
}

void GpuParticleEmitterWorkSystem::ExecuteCS() {
    HRESULT hr;
    DxFence* fence = Engine::getInstance()->getDxFence();

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
    fence->Signal(dxCommand_->getCommandQueue());
    fence->WaitForFence();
    ///===============================================================

    ///===============================================================
    /// リセット
    ///===============================================================
    dxCommand_->CommandReset();
    ///===============================================================
}
