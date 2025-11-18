#include "GpuParticleInitialize.h"

/// engine
// directX12 Object
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/DxFence.h"
#include "directX12/ShaderManager.h"

/// ECS
#include "component/effect/particle/gpuParticle/GpuParticle.h"

GpuParticleInitialize::GpuParticleInitialize()
    : ISystem(SystemCategory::Initialize) {}

GpuParticleInitialize::~GpuParticleInitialize() {}

void GpuParticleInitialize::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
    CreatePSO();
}

void GpuParticleInitialize::Update() {
    if (entityIDs_.empty()) {
        return;
    }
    ISystem::eraseDeadEntity();

    usingCS_ = false;

    StartCS();
    for (auto& id : entityIDs_) {
        Entity* entity = GetEntity(id);
        UpdateEntity(entity);
    }
    if (usingCS_) {
        ExecuteCS();
    }
}

void GpuParticleInitialize::Finalize() {
    if (dxCommand_) {
        dxCommand_->Finalize();
        dxCommand_.reset();
    }
    pso_ = nullptr;
}

void GpuParticleInitialize::UpdateEntity(Entity* _entity) {
    auto& commandList = dxCommand_->GetCommandList();

    auto gpuParticleVec = GetComponents<GpuParticleEmitter>(_entity);

    for (auto itr = gpuParticleVec->begin();
        itr != gpuParticleVec->end();
        ++itr) {

        GpuParticleEmitter& gpuParticleEmitter = *itr;
        if (!gpuParticleEmitter.IsActive()) {
            continue; // 非アクティブなパーティクルはスキップ
        }

        commandList->SetComputeRootDescriptorTable(
            particleBufferIndex_,
            gpuParticleEmitter.GetParticleUavDescriptor().GetGpuHandle());

        commandList->SetComputeRootDescriptorTable(
            freeIndexBufferIndex_,
            gpuParticleEmitter.GetFreeIndexUavDescriptor().GetGpuHandle());

        commandList->SetComputeRootDescriptorTable(
            freeListBufferIndex_,
            gpuParticleEmitter.GetFreeListUavDescriptor().GetGpuHandle());

        gpuParticleEmitter.GetShapeBuffer().ConvertToBuffer();
        gpuParticleEmitter.GetShapeBuffer().SetForComputeRootParameter(
            commandList.Get(),
            emitterShapeIndex);

        UINT dispatchCount = (gpuParticleEmitter.GetParticleSize() + 1023) / 1024;
        commandList->Dispatch(
            dispatchCount, // 1ワークグループあたり1024頂点を処理
            1,
            1); // X方向に分割、YとZは1

        usingCS_ = true;
    }
}

void GpuParticleInitialize::CreatePSO() {
    constexpr const char* psoKey = "InitializeGpuParticle.CS";

    if (pso_) {
        return; // PSOが既に作成されている場合は何もしない
    }

    ShaderManager* shaderManager = ShaderManager::GetInstance();
    DxDevice* dxDevice           = Engine::GetInstance()->GetDxDevice();

    if (shaderManager->IsRegisteredPipelineStateObj(psoKey)) {
        pso_ = shaderManager->GetPipelineStateObj(psoKey);
        return; // PSOが既に登録されている場合はそれを使用
    }

    // PSOが登録されていない場合は新規に作成

    /// ==========================================
    // Shader 読み込み
    /// ==========================================
    shaderManager->LoadShader(psoKey, shaderDirectory, L"cs_6_0");

    /// ==========================================
    // PSO 設定
    /// ==========================================
    ShaderInfo shaderInfo{};
    shaderInfo.csKey = psoKey;

#pragma region "ROOT_PARAMETER"

    D3D12_ROOT_PARAMETER rootParameters[4]                = {};
    rootParameters[particleBufferIndex_].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[particleBufferIndex_].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[particleBufferIndex_]);

    D3D12_DESCRIPTOR_RANGE particleDescriptorRange[1]            = {};
    particleDescriptorRange[0].BaseShaderRegister                = 0; // u0
    particleDescriptorRange[0].NumDescriptors                    = 1;
    particleDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    particleDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(
        particleDescriptorRange, 1, particleBufferIndex_);

    rootParameters[freeIndexBufferIndex_].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[freeIndexBufferIndex_].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[freeIndexBufferIndex_]);

    D3D12_DESCRIPTOR_RANGE freeIndexDescriptorRange[1]            = {};
    freeIndexDescriptorRange[0].BaseShaderRegister                = 1; // u1
    freeIndexDescriptorRange[0].NumDescriptors                    = 1;
    freeIndexDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    freeIndexDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(
        freeIndexDescriptorRange, 1, freeIndexBufferIndex_);

    rootParameters[freeListBufferIndex_].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[freeListBufferIndex_].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[freeListBufferIndex_]);

    D3D12_DESCRIPTOR_RANGE freeListDescriptorRange[1]            = {};
    freeListDescriptorRange[0].BaseShaderRegister                = 2; // u2
    freeListDescriptorRange[0].NumDescriptors                    = 1;
    freeListDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    freeListDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(
        freeListDescriptorRange, 1, freeListBufferIndex_);

    rootParameters[emitterShapeIndex].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[emitterShapeIndex].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[emitterShapeIndex].Descriptor.ShaderRegister = 0; // b0
    shaderInfo.pushBackRootParameter(rootParameters[emitterShapeIndex]);

#pragma endregion

    /// ==========================================
    // PSOの作成
    /// ==========================================
    pso_ = shaderManager->CreatePso(psoKey, shaderInfo, dxDevice->device_);
};
void GpuParticleInitialize::StartCS() {
    if (!pso_) {
        LOG_ERROR("PSO is not created for SkinningAnimationSystem");
        return;
    }

    ID3D12DescriptorHeap* ppHeaps[] = {
        Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    dxCommand_->GetCommandList()->SetDescriptorHeaps(1, ppHeaps);

    dxCommand_->GetCommandList()->SetPipelineState(pso_->pipelineState.Get());
    dxCommand_->GetCommandList()->SetComputeRootSignature(pso_->rootSignature.Get());
};
void GpuParticleInitialize::ExecuteCS() {
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
    fence->Signal(dxCommand_->GetCommandQueue());
    fence->WaitForFence();
    ///===============================================================

    ///===============================================================
    /// リセット
    ///===============================================================
    dxCommand_->CommandReset();
    ///===============================================================
}
