#include "DissolveEffect.h"

/// engine
#include "asset/AssetSystem.h"
#include "asset/TextureAsset.h"
#include "Engine.h"

// component
#include "component/effect/post/DissolveEffectParam.h"

// directX12
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

using namespace OriGine;

/// <summary>
/// コンストラクタ
/// </summary>
DissolveEffect::DissolveEffect() : BasePostRenderingSystem() {}

/// <summary>
/// デストラクタ
/// </summary>
DissolveEffect::~DissolveEffect() {}

/// <summary>
/// 初期化
/// </summary>
void DissolveEffect::Initialize() {
    BasePostRenderingSystem::Initialize();
}

/// <summary>
/// 終了処理
/// </summary>
void DissolveEffect::Finalize() {
    BasePostRenderingSystem::Finalize();
    pso_ = nullptr;
}

/// <summary>
/// PSO作成
/// </summary>
void DissolveEffect::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("Dissolve.PS", kShaderDirectory, L"ps_6_0");
    ShaderInformation shaderInfo{};
    shaderInfo.vsKey = "FullScreen.VS";
    shaderInfo.psKey = "Dissolve.PS";

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
    sampler.ShaderRegister   = 0; // Dissolve.PS.hlsl 側の `SamplerState gSampler : register(s0)` に対応
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    shaderInfo.pushBackSamplerDesc(sampler);

    ///================================================
    /// RootParameter の設定
    ///================================================
    // Texture だけ
    // ここで積んだ順番がそのままルートパラメータ番号(0,1,2,3)になる。
    // Dissolve.PS.hlsl 側のレジスタ番号と対応付けて管理する必要がある。
    D3D12_ROOT_PARAMETER rootParameter[4]    = {};
    D3D12_DESCRIPTOR_RANGE sceneViewRange[1] = {};
    sceneViewRange[0].BaseShaderRegister     = 0;
    sceneViewRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    sceneViewRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    sceneViewRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // RootParameter[0] = t0 (gSceneTexture / 合成前のシーン全体のテクスチャ)
    // DescriptorTable を使う
    rootParameter[0].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t sceneViewParamIdx          = shaderInfo.pushBackRootParameter(rootParameter[0]);
    shaderInfo.SetDescriptorRange2Parameter(sceneViewRange, 1, sceneViewParamIdx);

    D3D12_DESCRIPTOR_RANGE effectTexRange[1] = {};
    effectTexRange[0].BaseShaderRegister     = 1;
    effectTexRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    effectTexRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    effectTexRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // RootParameter[1] = t1 (gDissolveTexture / ディゾルブのマスクとして使うテクスチャ)
    // DescriptorTable を使う
    rootParameter[1].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t effectTexParamIdx          = shaderInfo.pushBackRootParameter(rootParameter[1]);
    shaderInfo.SetDescriptorRange2Parameter(effectTexRange, 1, effectTexParamIdx);

    // RootParameter[2] = b0 (gDissolveParam / threshold, edgeWidth, outLineColor)
    // ShaderRegister を明示していないため既定値の 0 が使われる
    rootParameter[2].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    shaderInfo.pushBackRootParameter(rootParameter[2]);

    // RootParameter[3] = b1 (gMaterial / 色とUV変換行列)。b0 と衝突しないよう ShaderRegister を 1 に明示
    rootParameter[3].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[3].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[3].Descriptor.ShaderRegister = 1;
    shaderInfo.pushBackRootParameter(rootParameter[3]);

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

    pso_ = shaderManager->CreatePso("DissolveEffect", shaderInfo, Engine::GetInstance()->GetDxDevice()->device_);
}

/// <summary>
/// ポストレンダリングをスキップするかどうか
/// </summary>
/// <returns>描画データがない場合は true</returns>
bool DissolveEffect::ShouldSkipPostRender() const {
    return activeRenderingData_.empty();
}

/// <summary>
/// レンダリング開始処理
/// </summary>
void DissolveEffect::RenderStart() {
    auto& commandList = dxCommand_->GetCommandList();

    /// target の設定
    // renderTarget_ を書き込み可能な状態(RENDER_TARGET)へ遷移させ、RTV/DSVをセットする
    renderTarget_->PreDraw();

    /// pso Set
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // SRV/CBV/UAV 用のディスクリプタヒープをコマンドリストにバインドする。
    // このヒープをセットしないと SetGraphicsRootDescriptorTable で参照するハンドルが無効になる
    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
}

/// <summary>
/// レンダリング処理
/// </summary>
void DissolveEffect::Rendering() {
    auto& commandList = dxCommand_->GetCommandList();

    for (const auto& renderingData : activeRenderingData_) {
        /// 描画 開始
        // エンティティ1体ごとに PreDraw/PostDraw を挟んでいるため、
        // 前段のディゾルブ結果を含んだ状態のバックバッファを次のエンティティの入力(t0)として使える
        RenderStart();

        // RootParameter[1] (t1: gDissolveTexture) にマスク用テクスチャをバインド
        commandList->SetGraphicsRootDescriptorTable(1,
            renderingData.srvHandle);

        auto& dissParam = renderingData.dissolveParam;
        // CPU側の値をGPU定数バッファへ書き込んでから、RootParameter[2](b0)/[3](b1) にセットする
        dissParam->GetDissolveBuffer().ConvertToBuffer();
        dissParam->GetDissolveBuffer().SetForRootParameter(dxCommand_->GetCommandList(), 2);
        dissParam->GetMaterialBuffer().SetForRootParameter(dxCommand_->GetCommandList(), 3);
        /// 描画 呼び出し
        // 現在のバックバッファ(直前までの描画結果)を RootParameter[0](t0) の入力として渡す
        RenderCall(renderTarget_->GetBackBufferSrvHandle());

        // 描画 終了
        RenderEnd();
    }

    /// アクティブなレンダリングデータのクリア
    activeRenderingData_.clear();
}

/// <summary>
/// レンダリング終了処理
/// </summary>
void DissolveEffect::RenderEnd() {
    // 描画 終了
    // renderTarget_ を RENDER_TARGET から次の読み取り(SRVとしてのサンプリング等)に備えた状態へ遷移させる
    renderTarget_->PostDraw();
}

/// <summary>
/// コンポーネントの割り当て
/// </summary>
/// <param name="_handle">エンティティ</param>
void DissolveEffect::DispatchComponent(const EntityHandle& _handle) {
    auto& effectParams = GetComponents<DissolveEffectParam>(_handle);

    if (effectParams.empty()) {
        return; // コンポーネントがない場合は何もしない
    }

    for (auto& param : effectParams) {
        if (!param.IsActive()) {
            continue;
        }

        DissolveEffect::RenderingData renderingData{};
        // デフォルトはパラメータに設定されたディゾルブ用テクスチャを使う
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(static_cast<size_t>(param.GetTextureIndex())).srv.GetGpuHandle();

        int32_t materialIndex = param.GetMaterialIndex();
        auto& materialBuff    = param.GetMaterialBuffer();
        if (materialIndex >= 0) {
            Material* material = GetComponent<Material>(_handle, materialIndex);
            material->UpdateUvMatrix();

            materialBuff.ConvertToBuffer(ColorAndUvTransform(material->color_, material->uvTransform_));

            // マテリアルが専用テクスチャを持つ場合は、そちらをディゾルブマスクとして優先する
            if (material->hasCustomTexture()) {
                srvHandle = material->GetCustomTexture()->srv_.GetGpuHandle();
            }
        }

        renderingData.srvHandle     = srvHandle;
        renderingData.dissolveParam = &param;
        activeRenderingData_.push_back(renderingData);
    }
}

/// <summary>
/// 描画呼び出し
/// </summary>
/// <param name="_viewHandle">描画対象のテクスチャハンドル</param>
void DissolveEffect::RenderCall(D3D12_GPU_DESCRIPTOR_HANDLE _viewHandle) {
    auto& commandList = dxCommand_->GetCommandList();

    /// ================================================
    /// Viewport の設定
    /// ================================================

    // RootParameter[0] (t0: gSceneTexture) に描画対象のテクスチャをバインド
    commandList->SetGraphicsRootDescriptorTable(0, _viewHandle);

    // 頂点バッファを使わず、頂点シェーダ側で頂点座標を生成する全画面三角形2枚(6頂点)を描画
    commandList->DrawInstanced(6, 1, 0, 0);
}
