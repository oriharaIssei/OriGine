#include "ParticleRenderSystem.h"

/// engine
#include "Engine.h"
#include "scene/SceneManager.h"

// directX12
#include "directX12/DxDevice.h"

// module
#include "camera/CameraManager.h"

// asset
#include "asset/AssetSystem.h"
#include "asset/TextureAsset.h"

// math
#include "math/Matrix4x4.h"

using namespace OriGine;

/// <summary>
/// コンストラクタ
/// </summary>
ParticleRenderSystem::ParticleRenderSystem() : BaseRenderSystem() {}

/// <summary>
/// デストラクタ
/// </summary>
ParticleRenderSystem::~ParticleRenderSystem() {}

/// <summary>
/// 初期化
/// </summary>
void ParticleRenderSystem::Initialize() {
    BaseRenderSystem::Initialize();

    for (size_t i = 0; i < kBlendNum; ++i) {
        activeEmittersByBlendMode_[i].reserve(100);
    }
}

/// <summary>
/// パイプラインステートオブジェクト（PSO）を作成する
/// </summary>
void ParticleRenderSystem::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::GetInstance();

    ///=================================================
    /// shader読み込み
    ///=================================================

    shaderManager->LoadShader("Particle.VS");
    shaderManager->LoadShader("Particle.PS", kShaderDirectory, L"ps_6_0");

    ///=================================================
    /// shader情報の設定
    ///=================================================
    ShaderInfo shaderInfo;
    shaderInfo.vsKey = "Particle.VS";
    shaderInfo.psKey = "Particle.PS";

#pragma region "RootParameter"
    D3D12_ROOT_PARAMETER rootParameter[4]{};
    // 0 ... ParticleData
    rootParameter[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameter[0].Descriptor.ShaderRegister = 0;
    shaderInfo.pushBackRootParameter(rootParameter[0]);

    D3D12_DESCRIPTOR_RANGE structuredRange[1]            = {};
    structuredRange[0].BaseShaderRegister                = 0;
    structuredRange[0].NumDescriptors                    = 1;
    structuredRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    structuredRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(structuredRange, 1, 0);
    // 1 ... ViewProjection
    rootParameter[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter[1].Descriptor.ShaderRegister = 1;
    shaderInfo.pushBackRootParameter(rootParameter[1]);
    // 2 ... Material
    rootParameter[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[2].Descriptor.ShaderRegister = 0;
    shaderInfo.pushBackRootParameter(rootParameter[2]);
    // 3 ... Texture
    // DescriptorTable を使う
    rootParameter[3].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t rootParameterIndex         = shaderInfo.pushBackRootParameter(rootParameter[3]);

    D3D12_DESCRIPTOR_RANGE descriptorRange[1]            = {};
    descriptorRange[0].BaseShaderRegister                = 0;
    descriptorRange[0].NumDescriptors                    = 1;
    descriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(descriptorRange, 1, rootParameterIndex);

#pragma endregion

#pragma region "Input Element"
    D3D12_INPUT_ELEMENT_DESC inputElementDesc = {};
    inputElementDesc.SemanticName             = "POSITION"; /*Semantics*/
    inputElementDesc.SemanticIndex            = 0; /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format                   = DXGI_FORMAT_R32G32B32A32_FLOAT; // float 4
    inputElementDesc.AlignedByteOffset        = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDesc);
    inputElementDesc.SemanticName      = "TEXCOORD"; /*Semantics*/
    inputElementDesc.SemanticIndex     = 0; /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format            = DXGI_FORMAT_R32G32_FLOAT; // float 2
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDesc);
    inputElementDesc.SemanticName      = "NORMAL"; /*Semantics*/
    inputElementDesc.SemanticIndex     = 0; /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32_FLOAT; // float 4
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDesc);
#pragma endregion

    ///=================================================
    /// Sampler
    D3D12_STATIC_SAMPLER_DESC staticSampler = {};
    staticSampler.Filter                    = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイナリニアフィルタ
    // 0 ~ 1 の間をリピート
    staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

    staticSampler.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
    staticSampler.MinLOD           = 0;
    staticSampler.MaxLOD           = D3D12_FLOAT32_MAX;
    staticSampler.ShaderRegister   = 0;
    staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    shaderInfo.pushBackSamplerDesc(staticSampler);
    /// Sampler
    ///=================================================

    ///=================================================
    /// BlendMode ごとの Pso作成
    ///=================================================
    for (size_t i = 0; i < kBlendNum; ++i) {
        shaderInfo.blendMode_ = BlendMode(i);
        psoByBlendMode_[i]    = shaderManager->CreatePso("Particle_" + kBlendModeStr[i], shaderInfo, Engine::GetInstance()->GetDxDevice()->device_);
    }
}

/// <summary>
/// エンティティのエミッターを登録する
/// </summary>
/// <param name="_entity">対象のエンティティハンドル</param>
void ParticleRenderSystem::DispatchRenderer(EntityHandle _entity) {
    // 有効なEmitterなら登録する
    // 無効ならreturn
    if (!_entity.IsValid()) {
        return;
    }

    Transform* transform           = GetComponent<Transform>(_entity);
    std::vector<ParticleSystem>& emitters = GetComponents<ParticleSystem>(_entity);

    if (emitters.empty()) {
        return;
    }

    for (auto& emitter : emitters) {
        if (!emitter.IsActive() || emitter.ParticleIsEmpty()) {
            return;
        }

        // 親が設定されていなければ_entityがもつTransformを設定する
        if (!emitter.GetParent()) {
            emitter.SetParent(transform);
        }

        BlendMode blend    = emitter.GetBlendMode();
        int32_t blendIndex = static_cast<int32_t>(blend);
        activeEmittersByBlendMode_[blendIndex].push_back(&emitter);
    }
}

/// <summary>
/// レンダリング開始時の共通設定
/// </summary>
void ParticleRenderSystem::StartRender() {
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand_->GetCommandList();
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
}

/// <summary>
/// 指定されたブレンドモードでパーティクルを描画する
/// </summary>
/// <param name="_blend">ブレンドモード</param>
/// <param name="_isCulling">カリングを有効にするかどうか（未使用）</param>
void ParticleRenderSystem::RenderingBy(BlendMode _blend, bool /*_isCulling*/) {
    int32_t blendIndex = static_cast<int32_t>(_blend);
    auto& emitters     = activeEmittersByBlendMode_[blendIndex];

    // skip check
    if (emitters.empty()) {
        return;
    }

    auto& commandList = dxCommand_->GetCommandList();

    // PSOセット
    commandList->SetPipelineState(psoByBlendMode_[blendIndex]->pipelineState.Get());
    // RootSignatureセット
    commandList->SetGraphicsRootSignature(psoByBlendMode_[blendIndex]->rootSignature.Get());

    CameraManager::GetInstance()->SetBufferForRootParameter(GetScene(), commandList, 1);

    Matrix4x4 viewMat = CameraManager::GetInstance()->GetTransform(GetScene()).viewMat;
    for (auto* emitter : emitters) {
        if (emitter == nullptr) {
            continue;
        }
        if (emitter->particles_.empty()) {
            continue;
        }

        Matrix4x4 scaleMat     = MakeMatrix4x4::Identity();
        Matrix4x4 rotateMat    = MakeMatrix4x4::Identity();
        Matrix4x4 translateMat = MakeMatrix4x4::Identity();

        if (emitter->particleIsBillBoard_) {
            // カメラの回転行列を取得し、平行移動成分をゼロにする
            Matrix4x4 cameraRotation = viewMat;
            cameraRotation[3][0]     = 0.0f;
            cameraRotation[3][1]     = 0.0f;
            cameraRotation[3][2]     = 0.0f;
            cameraRotation[3][3]     = 1.0f;
            Matrix4x4 billboardMat   = cameraRotation.inverse();

            for (size_t i = 0; i < emitter->particles_.size(); i++) {
                scaleMat                                            = MakeMatrix4x4::Scale(emitter->structuredTransform_.openData_[i].scale);
                translateMat                                        = MakeMatrix4x4::Translate(emitter->structuredTransform_.openData_[i].translate);
                emitter->structuredTransform_.openData_[i].worldMat = scaleMat * billboardMat * translateMat;
                emitter->structuredTransform_.openData_[i].uvMat    = emitter->particles_[i]->GetTransform().uvMat;
                emitter->structuredTransform_.openData_[i].color    = emitter->particles_[i]->GetTransform().color;
            }
        } else {
            for (size_t i = 0; i < emitter->particles_.size(); i++) {
                scaleMat                                            = MakeMatrix4x4::Scale(emitter->structuredTransform_.openData_[i].scale);
                rotateMat                                           = MakeMatrix4x4::RotateXYZ(emitter->structuredTransform_.openData_[i].rotate);
                translateMat                                        = MakeMatrix4x4::Translate(emitter->structuredTransform_.openData_[i].translate);
                emitter->structuredTransform_.openData_[i].worldMat = scaleMat * rotateMat * translateMat;
                emitter->structuredTransform_.openData_[i].uvMat    = emitter->particles_[i]->GetTransform().uvMat;
                emitter->structuredTransform_.openData_[i].color    = emitter->particles_[i]->GetTransform().color;
            }
        }

        if (emitter->emitter_.GetParent()) {
            for (size_t i = 0; i < emitter->particles_.size(); i++) {
                emitter->structuredTransform_.openData_[i].worldMat *= emitter->emitter_.GetParent()->worldMat;
            }
        }

        emitter->structuredTransform_.ConvertToBuffer();
        emitter->structuredTransform_.SetForRootParameter(commandList, 0);

        emitter->materialBuffer_.SetForRootParameter(commandList, 2);
        commandList->SetGraphicsRootDescriptorTable(
            3,
            AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(emitter->textureIndex_).srv.GetGpuHandle());

        commandList->IASetVertexBuffers(0, 1, &emitter->mesh_.GetVBView());
        commandList->IASetIndexBuffer(&emitter->mesh_.GetIBView());
        commandList->DrawIndexedInstanced(
            UINT(emitter->mesh_.GetIndexSize()),
            static_cast<UINT>(emitter->structuredTransform_.openData_.size()),
            0, 0, 0);
    }

    // 描画後クリア
    emitters.clear();
}

/// <summary>
/// 終了処理
/// </summary>
void ParticleRenderSystem::Finalize() {
    dxCommand_->Finalize();
}

/// <summary>
/// レンダリングをスキップするかどうかを判定する
/// </summary>
/// <returns>true = 描画対象なし / false = 描画対象あり</returns>
bool ParticleRenderSystem::ShouldSkipRender() const {
    for (size_t i = 0; i < kBlendNum; ++i) {
        if (!activeEmittersByBlendMode_.at(i).empty()) {
            return false;
        }
    }
    return true;
}
