#include "DistortionEffect.h"

/// engine
#include "Engine.h"
#include "asset/AssetSystem.h"
// asset
#include "asset/TextureAsset.h"
// component
#include "component/effect/post/DistortionEffectParam.h"

// directX12
#include "directX12/DxDevice.h"

using namespace OriGine;

/// <summary>
/// コンストラクタ
/// </summary>
DistortionEffect::DistortionEffect() : BasePostRenderingSystem() {}

/// <summary>
/// デストラクタ
/// </summary>
DistortionEffect::~DistortionEffect() {}

/// <summary>
/// 初期化
/// </summary>
void DistortionEffect::Initialize() {
    BasePostRenderingSystem::Initialize();
}

/// <summary>
/// 終了処理
/// </summary>
void DistortionEffect::Finalize() {
    pso_ = nullptr;

    if (dxCommand_) {
        dxCommand_->Finalize();
        dxCommand_.reset();
        dxCommand_ = nullptr;
    }
}

/// <summary>
/// PSO作成
/// </summary>
void DistortionEffect::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::GetInstance();
    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("Distortion.PS", kShaderDirectory, L"ps_6_0");
    ShaderInformation shaderInfo{};
    shaderInfo.vsKey = "FullScreen.VS";
    shaderInfo.psKey = "Distortion.PS";

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
    // distortion Texture
    // shaderInfo.pushBackRootParameter の戻り値(挿入されたインデックス)をメンバに保持しておき、
    // 描画時に SetGraphicsRootDescriptorTable / SetForRootParameter へそのまま使う。
    // こうすることでルートパラメータの並びを変えてもここ1箇所を直せば済むようにしている。
    D3D12_ROOT_PARAMETER rootParameter[4] = {};
    D3D12_DESCRIPTOR_RANGE texRange[2]    = {};

    // distortion texture (t0) : Distortion.PS.hlsl の gDistortionTexture (UVをずらす量を格納したテクスチャ)
    texRange[0].BaseShaderRegister                = 0;
    texRange[0].NumDescriptors                    = 1;
    texRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    texRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParameter[0].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    distortionTextureIndex_           = (int32_t)shaderInfo.pushBackRootParameter(rootParameter[0]);
    shaderInfo.SetDescriptorRange2Parameter(&texRange[0], 1, distortionTextureIndex_);

    // scene texture (t1) : Distortion.PS.hlsl の gSceneViewTexture (ずらしたUVでサンプリングする元シーン)
    texRange[1].BaseShaderRegister                = 1;
    texRange[1].NumDescriptors                    = 1;
    texRange[1].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    texRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParameter[1].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    sceneTextureIndex_                = (int32_t)shaderInfo.pushBackRootParameter(rootParameter[1]);
    shaderInfo.SetDescriptorRange2Parameter(&texRange[1], 1, sceneTextureIndex_);

    // distortion param (b0) : gEffectParam (歪みのバイアス・強度)
    rootParameter[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[2].Descriptor.ShaderRegister = 0;
    distortionParamIndex_                      = (int32_t)shaderInfo.pushBackRootParameter(rootParameter[2]);

    // material (b1) : gMaterial (色 と 歪みテクスチャ サンプリング用のUV変換行列)
    rootParameter[3].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[3].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[3].Descriptor.ShaderRegister = 1;
    materialIndex_                             = (int32_t)shaderInfo.pushBackRootParameter(rootParameter[3]);

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

    pso_ = shaderManager->CreatePso("DistortionEffect", shaderInfo, Engine::GetInstance()->GetDxDevice()->device_);
}

/// <summary>
/// レンダリング開始処理
/// </summary>
void DistortionEffect::RenderStart() {
    auto& commandList = dxCommand_->GetCommandList();

    /// ----------------------------------------------------------
    /// pso Set
    /// ----------------------------------------------------------
    renderTarget_->PreDraw();

    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    dxCommand_->GetCommandList()->SetDescriptorHeaps(1, ppHeaps);
}

/// <summary>
/// レンダリング処理
/// </summary>
void DistortionEffect::Rendering() {
    auto& commandList = dxCommand_->GetCommandList();

    // 単一テクスチャ指定のエフェクト(2Dの歪みテクスチャを直接使うケース)
    for (auto& renderingData : activeRenderingData_) {
        // 開始処理
        RenderStart();

        // Set buffer
        renderingData.effectParam->GetEffectParamBuffer().ConvertToBuffer();
        // t0: 指定された歪みテクスチャ、t1: 現在のバックバッファ
        commandList->SetGraphicsRootDescriptorTable(distortionTextureIndex_, renderingData.srvHandle);
        commandList->SetGraphicsRootDescriptorTable(sceneTextureIndex_, renderTarget_->GetBackBufferSrvHandle());
        renderingData.effectParam->GetEffectParamBuffer().SetForRootParameter(commandList, distortionParamIndex_);
        renderingData.effectParam->GetMaterialBuffer().SetForRootParameter(commandList, materialIndex_);

        // Draw
        commandList->DrawInstanced(6, 1, 0, 0);

        // 終了処理
        RenderEnd();
    }

    // 描画データクリア
    activeRenderingData_.clear();
}

/// <summary>
/// レンダリング終了処理
/// </summary>
void DistortionEffect::RenderEnd() {
    // 終了処理
    // renderTarget_ を RENDER_TARGET から読み取り可能な状態へ遷移させる
    renderTarget_->PostDraw();
}

/// <summary>
/// コンポーネントの割り当て
/// </summary>
/// <param name="_handle">エンティティ</param>
void DistortionEffect::DispatchComponent(const EntityHandle& _handle) {
    auto& distortionEffectParams = GetComponents<DistortionEffectParam>(_handle);
    if (distortionEffectParams.empty()) {
        return;
    }

    for (auto& effectParam : distortionEffectParams) {
        if (!effectParam.GetIsActive()) {
            continue;
        }

        // 単一テクスチャを使用する場合
        RenderingData renderingData{};
        renderingData.effectParam = &effectParam;
        renderingData.srvHandle   = AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(effectParam.GetTextureIndex()).srv.GetGpuHandle();

        // マテリアル情報の更新
        int32_t materialIndex = effectParam.GetMaterialIndex();
        auto& materialBuff    = effectParam.GetMaterialBuffer();
        if (materialIndex >= 0) {
            Material* material = GetComponent<Material>(_handle, materialIndex);
            material->UpdateUvMatrix();

            materialBuff.ConvertToBuffer(ColorAndUvTransform(material->color_, material->uvTransform_));

            if (material->hasCustomTexture()) {
                renderingData.srvHandle = material->GetCustomTexture()->srv_.GetGpuHandle();
            }
        }
        // 追加
        activeRenderingData_.emplace_back(renderingData);
    }
}

/// <summary>
/// ポストレンダリングをスキップするかどうか
/// </summary>
/// <returns>描画データがない場合は true</returns>
bool DistortionEffect::ShouldSkipPostRender() const {
    return activeRenderingData_.empty();
}
