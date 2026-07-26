#include "GrayscaleEffect.h"

/// engine
#include "Engine.h"
#include "scene/Scene.h"

// directX12
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

using namespace OriGine;

/// <summary>
/// コンストラクタ
/// </summary>
GrayscaleEffect::GrayscaleEffect() : BasePostRenderingSystem() {}

/// <summary>
/// デストラクタ
/// </summary>
GrayscaleEffect::~GrayscaleEffect() {}

/// <summary>
/// 初期化
/// </summary>
void GrayscaleEffect::Initialize() {
    constexpr int32_t kDefGrayscaleCompCount = 10; // グレースケールエフェクトに使用するコンポーネントの想定最大数 (必要に応じて調整する)
    BasePostRenderingSystem::Initialize();
    grayscaleComps_.reserve(kDefGrayscaleCompCount);
}

/// <summary>
/// 終了処理
/// </summary>
void GrayscaleEffect::Finalize() {
    // BasePostRenderingSystem::Finalize() とは異なり dxCommand_ の nullptr チェックを行っていない点に注意
    dxCommand_->Finalize();
    dxCommand_.reset();
    pso_ = nullptr;
}

/// <summary>
/// PSO作成
/// </summary>
void GrayscaleEffect::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("Grayscale.PS", kShaderDirectory, L"ps_6_0");
    ShaderInformation shaderInfo{};
    shaderInfo.vsKey = "FullScreen.VS";
    shaderInfo.psKey = "Grayscale.PS";

    ///================================================
    /// Sampler の設定
    ///================================================
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter                    = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイナリニアフィルタ
    // 0 ~ 1 の間をリピート
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

    sampler.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
    sampler.MinLOD           = 0;
    sampler.MaxLOD           = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister   = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    shaderInfo.pushBackSamplerDesc(sampler);

    ///================================================
    /// RootParameter の設定
    ///================================================
    // Texture だけ
    // RootParameter[0] = b0 (gGrayScaleBuff / グレースケール化の適用度 grayscaleAmount)
    // ShaderRegister を明示していないため既定値の 0 が使われる
    D3D12_ROOT_PARAMETER rootParameter[2] = {};

    rootParameter[0].ParameterType        = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[0].ShaderVisibility     = D3D12_SHADER_VISIBILITY_PIXEL;
    shaderInfo.pushBackRootParameter(rootParameter[0]);

    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister     = 0;
    descriptorRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // RootParameter[1] = t0 (gTexture / 変換前のシーンテクスチャ)
    // DescriptorTable を使う
    rootParameter[1].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t rootParameterIndex         = shaderInfo.pushBackRootParameter(rootParameter[1]);
    shaderInfo.SetDescriptorRange2Parameter(descriptorRange, 1, rootParameterIndex);

    ///================================================
    /// InputElement の設定
    ///================================================

    // 特に使わない

    ///================================================
    /// depthStencil の設定
    ///================================================
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = false;
    shaderInfo.SetDepthStencilDesc(depthStencilDesc);

    pso_ = shaderManager->CreatePso("GrayscaleEffect", shaderInfo, Engine::GetInstance()->GetDxDevice()->device_);
}

/// <summary>
/// レンダリング開始処理
/// </summary>
void GrayscaleEffect::RenderStart() {
    auto& commandList = dxCommand_->GetCommandList();

    // Setting

    // renderTarget_ はダブルバッファ構成になっており、PreDraw() でフロントバッファをクリア&書き込み対象にする。
    // DrawTexture() は renderTarget_ 自身が持つブリット用PSOで、直前のバックバッファの内容を
    // 新しいフロントバッファへコピーしておく(この後グレースケールPSOへ切り替えて上書き描画する)。
    renderTarget_->PreDraw();
    renderTarget_->DrawTexture();

    // ここから本エフェクト(グレースケール)用の PSO / ルートシグネチャに切り替える
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);

    // RootParameter[1](t0) に DrawTexture() 済みの現在のバックバッファ内容をバインド
    commandList->SetGraphicsRootDescriptorTable(1, renderTarget_->GetBackBufferSrvHandle());
}

/// <summary>
/// レンダリング処理
/// </summary>
void GrayscaleEffect::Rendering() {
    auto& commandList = dxCommand_->GetCommandList();

    for (auto* grayscaleComp : grayscaleComps_) {
        RenderStart();

        // RootParameter[0](b0) にグレースケール適用度(grayscaleAmount)を送る
        grayscaleComp->GetConstantBuffer().SetForRootParameter(commandList.Get(), 0);

        // 全画面三角形2枚(6頂点)でグレースケール変換を適用
        commandList->DrawInstanced(6, 1, 0, 0);

        RenderEnd();
    }

    grayscaleComps_.clear();
}

/// <summary>
/// レンダリング終了処理
/// </summary>
void GrayscaleEffect::RenderEnd() {
    // フロントバッファを読み取り可能な状態へ遷移し、フロント/バックのインデックスを入れ替える
    renderTarget_->PostDraw();
}

/// <summary>
/// コンポーネントの割り当て
/// </summary>
/// <param name="_owner">エンティティ</param>
void OriGine::GrayscaleEffect::DispatchComponent(const EntityHandle& _owner) {
    auto& comps = GetComponents<GrayscaleComponent>(_owner);
    if (comps.empty()) {
        return;
    }

    for (auto& comp : comps) {
        if (comp.IsEnabled()) {
            comp.GetConstantBuffer().ConvertToBuffer();
            grayscaleComps_.push_back(&comp);
        }
    }
}
