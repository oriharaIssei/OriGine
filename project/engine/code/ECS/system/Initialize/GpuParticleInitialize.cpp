#include "GpuParticleInitialize.h"

/// engine
// directX12 Object
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/DxFence.h"
#include "directX12/ShaderManager.h"

/// ECS
#include "component/effect/particle/gpuParticle/GpuParticle.h"

using namespace OriGine;

GpuParticleInitialize::GpuParticleInitialize()
    : ISystem(SystemCategory::Initialize) {}

GpuParticleInitialize::~GpuParticleInitialize() {}

void GpuParticleInitialize::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
    CreatePSO();
}

void GpuParticleInitialize::Update() {
    if (entities_.empty()) {
        return;
    }
    ISystem::EraseDeadEntity();

    usingCS_ = false;

    StartCS();
    for (auto& id : entities_) {
        UpdateEntity(id);
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

void GpuParticleInitialize::UpdateEntity(EntityHandle _handle) {
    auto& commandList = dxCommand_->GetCommandList();

    auto& gpuParticleVec = GetComponents<GpuParticleEmitter>(_handle);

    for (auto itr = gpuParticleVec.begin();
        itr != gpuParticleVec.end();
        ++itr) {

        GpuParticleEmitter& gpuParticleEmitter = *itr;
        if (!gpuParticleEmitter.IsActive()) {
            continue; // 非アクティブなパーティクルはスキップ
        }

        commandList->SetComputeRootDescriptorTable(
            kParticleBufferIndex_,
            gpuParticleEmitter.GetParticleUavDescriptor().GetGpuHandle());

        commandList->SetComputeRootDescriptorTable(
            kFreeIndexBufferIndex_,
            gpuParticleEmitter.GetFreeIndexUavDescriptor().GetGpuHandle());

        commandList->SetComputeRootDescriptorTable(
            kFreeListBufferIndex_,
            gpuParticleEmitter.GetFreeListUavDescriptor().GetGpuHandle());

        gpuParticleEmitter.GetShapeBuffer().ConvertToBuffer();
        gpuParticleEmitter.GetShapeBuffer().SetForComputeRootParameter(
            commandList.Get(),
            kEmitterShapeIndex);

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
    shaderManager->LoadShader(psoKey, kShaderDirectory, L"cs_6_0");

    /// ==========================================
    // PSO 設定
    /// ==========================================
    ShaderInfo shaderInfo{};
    shaderInfo.csKey = psoKey;

#pragma region "ROOT_PARAMETER"

    D3D12_ROOT_PARAMETER rootParameters[4]                 = {};
    rootParameters[kParticleBufferIndex_].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[kParticleBufferIndex_].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[kParticleBufferIndex_]);

    D3D12_DESCRIPTOR_RANGE particleDescriptorRange[1]            = {};
    particleDescriptorRange[0].BaseShaderRegister                = 0; // u0
    particleDescriptorRange[0].NumDescriptors                    = 1;
    particleDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    particleDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(
        particleDescriptorRange, 1, kParticleBufferIndex_);

    rootParameters[kFreeIndexBufferIndex_].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[kFreeIndexBufferIndex_].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[kFreeIndexBufferIndex_]);

    D3D12_DESCRIPTOR_RANGE freeIndexDescriptorRange[1]            = {};
    freeIndexDescriptorRange[0].BaseShaderRegister                = 1; // u1
    freeIndexDescriptorRange[0].NumDescriptors                    = 1;
    freeIndexDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    freeIndexDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(
        freeIndexDescriptorRange, 1, kFreeIndexBufferIndex_);

    rootParameters[kFreeListBufferIndex_].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[kFreeListBufferIndex_].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[kFreeListBufferIndex_]);

    D3D12_DESCRIPTOR_RANGE freeListDescriptorRange[1]            = {};
    freeListDescriptorRange[0].BaseShaderRegister                = 2; // u2
    freeListDescriptorRange[0].NumDescriptors                    = 1;
    freeListDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    freeListDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(
        freeListDescriptorRange, 1, kFreeListBufferIndex_);

    rootParameters[kEmitterShapeIndex].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[kEmitterShapeIndex].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[kEmitterShapeIndex].Descriptor.ShaderRegister = 0; // b0
    shaderInfo.pushBackRootParameter(rootParameters[kEmitterShapeIndex]);

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
    UINT64 fenceVal = fence->Signal(dxCommand_->GetCommandQueue());
    fence->WaitForFence(fenceVal);
    ///===============================================================

    ///===============================================================
    /// リセット
    ///===============================================================
    dxCommand_->CommandReset();
    ///===============================================================
}
