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
        GameEntity* entity = getEntity(id);
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

void GpuParticleInitialize::UpdateEntity(GameEntity* _entity) {
    auto& commandList = dxCommand_->getCommandList();

    auto gpuParticleVec = getComponents<GpuParticleEmitter>(_entity);

    for (auto itr = gpuParticleVec->begin();
        itr != gpuParticleVec->end();
        ++itr) {

        GpuParticleEmitter& gpuParticleEmitter = *itr;
        if (!gpuParticleEmitter.isActive()) {
            continue; // 非アクティブなパーティクルはスキップ
        }

        commandList->SetComputeRootDescriptorTable(
            particleBufferIndex_,
            gpuParticleEmitter.getUavDescriptor()->getGpuHandle());

        UINT dispatchCount = (gpuParticleEmitter.getParticleSize() + 1023) / 1024;
        commandList->Dispatch(
            dispatchCount, // 1ワークグループあたり1024頂点を処理
            1,
            1); // X方向に分割、YとZは1

        dxCommand_->ResourceBarrier(
            gpuParticleEmitter.getResource().getResource(),
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        usingCS_ = true;
    }
}

void GpuParticleInitialize::CreatePSO() {
    constexpr const char* psoKey = "InitializeGpuParticle.CS";

    if (pso_) {
        return; // PSOが既に作成されている場合は何もしない
    }

    ShaderManager* shaderManager = ShaderManager::getInstance();
    DxDevice* dxDevice           = Engine::getInstance()->getDxDevice();

    if (shaderManager->IsRegistertedPipelineStateObj(psoKey)) {
        pso_ = shaderManager->getPipelineStateObj(psoKey);
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

    D3D12_ROOT_PARAMETER rootParameters[1]                = {};
    rootParameters[particleBufferIndex_].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[particleBufferIndex_].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[particleBufferIndex_]);

    D3D12_DESCRIPTOR_RANGE particleDescriptorRange[1]            = {};
    particleDescriptorRange[0].BaseShaderRegister                = 0; // u0
    particleDescriptorRange[0].NumDescriptors                    = 1;
    particleDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    particleDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.setDescriptorRange2Parameter(
        particleDescriptorRange, 1, particleBufferIndex_);

#pragma endregion

    /// ==========================================
    // PSOの作成
    /// ==========================================
    pso_ = shaderManager->CreatePso(psoKey, shaderInfo, dxDevice->getDevice());
};
void GpuParticleInitialize::StartCS() {
    if (!pso_) {
        LOG_ERROR("PSO is not created for SkinningAnimationSystem");
        return;
    }

    ID3D12DescriptorHeap* ppHeaps[] = {
        Engine::getInstance()->getSrvHeap()->getHeap().Get()};
    dxCommand_->getCommandList()->SetDescriptorHeaps(1, ppHeaps);

    dxCommand_->getCommandList()->SetPipelineState(pso_->pipelineState.Get());
    dxCommand_->getCommandList()->SetComputeRootSignature(pso_->rootSignature.Get());
};
void GpuParticleInitialize::ExecuteCS() {
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
