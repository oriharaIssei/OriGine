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

/// <summary>
/// コンストラクタ
/// </summary>
GpuParticleInitialize::GpuParticleInitialize()
    : ISystem(SystemCategory::Initialize) {}

/// <summary>
/// デストラクタ
/// </summary>
GpuParticleInitialize::~GpuParticleInitialize() {}

/// <summary>
/// 初期化処理。Compute Shader実行用のコマンドリストとPSOを準備する
/// </summary>
void GpuParticleInitialize::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
    CreatePSO();
}

/// <summary>
/// 登録エンティティ全てについて、GPUパーティクルバッファの初期化ディスパッチをまとめて発行する
/// </summary>
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
    // 1件もDispatchしていない場合はコマンドリストが空のままなので、
    // わざわざExecuteCS(GPU実行~完了待ち)を行わずコストを避ける
    if (usingCS_) {
        ExecuteCS();
    }
}

/// <summary>
/// 終了処理
/// </summary>
void GpuParticleInitialize::Finalize() {
    if (dxCommand_) {
        dxCommand_->Finalize();
        dxCommand_.reset();
    }
    pso_ = nullptr;
}

/// <summary>
/// エンティティが持つ各GpuParticleEmitterについて、初期化用Compute Shaderをディスパッチし、
/// パーティクルバッファとフリーリスト(空きスロット管理用バッファ)を初期状態へ書き込む
/// </summary>
/// <param name="_handle">対象のエンティティハンドル</param>
void GpuParticleInitialize::UpdateEntity(const EntityHandle& _handle) {
    auto& commandList = dxCommand_->GetCommandList();

    auto& gpuParticleVec = GetComponents<GpuParticleEmitter>(_handle);

    for (auto itr = gpuParticleVec.begin();
        itr != gpuParticleVec.end();
        ++itr) {

        GpuParticleEmitter& gpuParticleEmitter = *itr;
        if (!gpuParticleEmitter.IsActive()) {
            continue; // 非アクティブなパーティクルはスキップ
        }

        // u0: パーティクル本体のデータ配列
        commandList->SetComputeRootDescriptorTable(
            kParticleBufferIndex_,
            gpuParticleEmitter.GetParticleUavDescriptor().GetGpuHandle());

        // u1: フリーリストのスタックポインタ(先頭インデックス)。
        // 発生時にここをデクリメントして空きスロットを1つ取り出し、消滅時にインクリメントして返却する
        commandList->SetComputeRootDescriptorTable(
            kFreeIndexBufferIndex_,
            gpuParticleEmitter.GetFreeIndexUavDescriptor().GetGpuHandle());

        // u2: 未使用パーティクルのインデックスを積んだフリーリスト本体
        commandList->SetComputeRootDescriptorTable(
            kFreeListBufferIndex_,
            gpuParticleEmitter.GetFreeListUavDescriptor().GetGpuHandle());

        gpuParticleEmitter.GetShapeBuffer().ConvertToBuffer();
        gpuParticleEmitter.GetShapeBuffer().SetForComputeRootParameter(
            commandList.Get(),
            kEmitterShapeIndex);

        // 最大パーティクル数をスレッドグループサイズ(1024)で割り切り上げ、
        // 全パーティクル分を1回のディスパッチで初期化できるグループ数を求める
        UINT dispatchCount = (gpuParticleEmitter.GetParticleSize() + 1023) / 1024;
        commandList->Dispatch(
            dispatchCount, // 1ワークグループあたり1024頂点を処理
            1,
            1); // X方向に分割、YとZは1

        usingCS_ = true;
    }
}

/// <summary>
/// パーティクル初期化用のパイプラインステートオブジェクト(PSO)を作成する
/// </summary>
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

/// <summary>
/// Compute Shaderの実行を開始する（ディスクリプタヒープ・PSO・ルートシグネチャの設定）
/// </summary>
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

/// <summary>
/// Compute Shaderを実行(コマンドを発行)する
/// </summary>
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
