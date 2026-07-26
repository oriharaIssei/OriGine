#include "SpeedlineEffect.h"

/// engine
#include "Engine.h"
#include "scene/SceneManager.h"
#include "asset/AssetSystem.h"
// asset
#include "asset/TextureAsset.h"

// directX12
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

using namespace OriGine;

/// <summary>
/// コンストラクタ
/// </summary>
SpeedlineEffect::SpeedlineEffect() : BasePostRenderingSystem() {}

/// <summary>
/// デストラクタ
/// </summary>
SpeedlineEffect::~SpeedlineEffect() {}

/// <summary>
/// 初期化
/// </summary>
void SpeedlineEffect::Initialize() {
    BasePostRenderingSystem::Initialize();
}

/// <summary>
/// 終了処理
/// </summary>
void SpeedlineEffect::Finalize() {
    if (dxCommand_) {
        dxCommand_->Finalize();
        dxCommand_.reset();
    }
    pso_ = nullptr;
}

/// <summary>
/// PSO作成
/// </summary>
void SpeedlineEffect::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("Speedline.PS", kShaderDirectory, L"ps_6_0");
    ShaderInformation shaderInfo{};
    shaderInfo.vsKey = "FullScreen.VS";
    shaderInfo.psKey = "Speedline.PS";

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
    D3D12_ROOT_PARAMETER rootParameter[3]             = {};
    D3D12_DESCRIPTOR_RANGE sceneTexDescriptorRange[1] = {};
    sceneTexDescriptorRange[0].BaseShaderRegister     = 0;
    sceneTexDescriptorRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    sceneTexDescriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    sceneTexDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // RootParameter[0] = t0 (gSceneTexture / 集中線を重ねる前のシーン)
    // DescriptorTable を使う
    rootParameter[0].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t rootParameterIndex         = shaderInfo.pushBackRootParameter(rootParameter[0]);
    shaderInfo.SetDescriptorRange2Parameter(sceneTexDescriptorRange, 1, rootParameterIndex);

    // RootParameter[1] = b0 (gLineEffectParam / 中心・強度・密度・色・時間・フェード)
    // ShaderRegister を明示していないため既定値の 0 が使われる
    rootParameter[1].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    shaderInfo.pushBackRootParameter(rootParameter[1]);

    D3D12_DESCRIPTOR_RANGE radialTexDescriptorRange[1] = {};
    radialTexDescriptorRange[0].BaseShaderRegister     = 1;
    radialTexDescriptorRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    radialTexDescriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    radialTexDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // RootParameter[2] = t1 (gRadialTexture / 中心からの極座標(距離,角度)でサンプリングする集中線テクスチャ)
    // DescriptorTable を使う
    rootParameter[2].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameterIndex                = shaderInfo.pushBackRootParameter(rootParameter[2]);
    shaderInfo.SetDescriptorRange2Parameter(radialTexDescriptorRange, 1, rootParameterIndex);

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

    pso_ = shaderManager->CreatePso("SpeedlineEffect", shaderInfo, Engine::GetInstance()->GetDxDevice()->device_);
}

/// <summary>
/// レンダリング開始処理
/// </summary>
void SpeedlineEffect::RenderStart() {
    auto& commandList = dxCommand_->GetCommandList();

    // renderTarget_ はダブルバッファ構成。PreDraw() でフロントバッファを書き込み対象にし、
    // DrawTexture() で直前のバックバッファ内容をブリットしてから、このあとスピードライン用PSOへ切り替える
    renderTarget_->PreDraw();
    renderTarget_->DrawTexture();

    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);

    // RootParameter[0](t0)に現在のバックバッファ(DrawTexture後の内容)をバインド
    commandList->SetGraphicsRootDescriptorTable(0, renderTarget_->GetBackBufferSrvHandle());
}

/// <summary>
/// コンポーネント描画処理
/// </summary>
/// <param name="_param">パラメータ</param>
void SpeedlineEffect::Render(SpeedlineEffectParam* _param) {
    auto& commandList = dxCommand_->GetCommandList();

    // RootParameter[1](b0)に集中線パラメータを送る。Speedline.PS.hlsl 側では、
    // ピクセルのUVを中心(screenCenterUV)からの極座標(距離dist, 角度atan2)に変換し、
    // 距離をradialUV.xに(density倍+時間でスクロール)、角度を[0,1]に正規化してradialUV.yに割り当てて
    // ラジアルテクスチャをサンプリングすることで、中心から放射状に伸びる線を表現している
    _param->GetBuffer().ConvertToBuffer();
    _param->GetBuffer().SetForRootParameter(dxCommand_->GetCommandList(), 1);

    // RootParameter[2](t1)にラジアルテクスチャをバインド
    commandList->SetGraphicsRootDescriptorTable(
        2,
        AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(_param->GetRadialTextureIndex()).srv.GetGpuHandle());

    // 全画面三角形2枚(6頂点)で描画
    commandList->DrawInstanced(6, 1, 0, 0);
}

/// <summary>
/// レンダリング処理
/// </summary>
void SpeedlineEffect::Rendering() {

    for (auto param : activeParams_) {
        // 描画開始
        RenderStart();

        // 描画
        Render(param);

        // 描画終了
        RenderEnd();
    }

    // アクティブなパラメータをクリア
    activeParams_.clear();
}

/// <summary>
/// レンダリング終了処理
/// </summary>
void SpeedlineEffect::RenderEnd() {
    // フロントバッファを読み取り可能な状態へ遷移し、フロント/バックのインデックスを入れ替える
    renderTarget_->PostDraw();
}

/// <summary>
/// コンポーネントの割り当て
/// </summary>
/// <param name="_handle">エンティティ</param>
void SpeedlineEffect::DispatchComponent(const EntityHandle& _handle) {
    auto& speedlineParams = GetComponents<SpeedlineEffectParam>(_handle);
    // 無効な場合はスルー
    if (speedlineParams.empty()) {
        return;
    }

    for (auto& param : speedlineParams) {
        if (!param.IsActive()) {
            continue;
        }
        activeParams_.push_back(&param);
    }
}

/// <summary>
/// ポストレンダリングをスキップするかどうか
/// </summary>
/// <returns>描画データがない場合は true</returns>
bool SpeedlineEffect::ShouldSkipPostRender() const {
    return activeParams_.empty();
}
