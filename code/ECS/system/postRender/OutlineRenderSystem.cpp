#include "OutlineRenderSystem.h"

/// engine
#include "camera/CameraManager.h"
#include "scene/Scene.h"
// asset
#include "asset/TextureAsset.h"

/// ECS
// component
#include "component/renderer/MeshRenderer.h"
#include "component/renderer/primitive/BoxRenderer.h"
#include "component/renderer/primitive/CylinderRenderer.h"
#include "component/renderer/primitive/PlaneRenderer.h"
#include "component/renderer/primitive/RingRenderer.h"
#include "component/renderer/primitive/SphereRenderer.h"

using namespace OriGine;

namespace {
static const std::string kVSName = "Outline.VS";
static const std::string kPSName = "Outline.PS";
}

/// <summary>
/// コンストラクタ
/// </summary>
OriGine::OutlineRenderSystem::OutlineRenderSystem() {}
/// <summary>
/// デストラクタ
/// </summary>
OriGine::OutlineRenderSystem::~OutlineRenderSystem() {}

/// <summary>
/// 初期化
/// </summary>
void OriGine::OutlineRenderSystem::Initialize() {
    CreatePSO();

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    if (!activeEntries_.empty()) {
        activeEntries_.clear();
    }
}

/// <summary>
/// 終了処理
/// </summary>
void OriGine::OutlineRenderSystem::Finalize() {
    if (!activeEntries_.empty()) {
        activeEntries_.clear();
    }

    dxCommand_->Finalize();
}

/// <summary>
/// PSO作成
/// </summary>
void OriGine::OutlineRenderSystem::CreatePSO() {
    const std::string kPsoKey = "Outline";

    ShaderManager* shaderManager = ShaderManager::GetInstance();
    DxDevice* dxDevice           = Engine::GetInstance()->GetDxDevice();

    // 登録されているかどうかをチェック
    if (shaderManager->IsRegisteredPipelineStateObj(kPsoKey)) {
        pso_ = shaderManager->GetPipelineStateObj(kPsoKey);

        //! TODO : 自動化
        transformBufferIndex_    = 0;
        cameraBufferIndex_       = 1;
        materialBufferIndex_     = 2;
        textureBufferIndex_      = 3;
        outlineParamBufferIndex_ = 4;
    }

    ///=================================================
    /// shader読み込み
    ///=================================================
    shaderManager->LoadShader(kVSName);
    shaderManager->LoadShader(kPSName, kShaderDirectory, L"ps_6_0");

    ///=================================================
    /// shader情報の設定
    ///=================================================
    ShaderInfo texShaderInfo{};
    texShaderInfo.vsKey = kVSName;
    texShaderInfo.psKey = kPSName;

#pragma region "RootParameter"
    // 各ルートパラメータと Outline.VS.hlsl / Outline.PS.hlsl 側のレジスタの対応:
    //   [0] Transform      -> VS b0 (gWorldTransform.world)
    //   [1] CameraTransform-> ALL b2 (Object3dTextureColor.hlsli の gViewProjection)
    //   [2] Material       -> PS b0 (gMaterial)
    //   [3] Texture        -> PS t0 (gTexture)
    //   [4] outlineBuffer  -> VS b1 (gOutlineParam: outlineWidth, outlineColor) を意図
    D3D12_ROOT_PARAMETER rootParameter[5]{};
    // Transform ... 0
    rootParameter[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameter[0].Descriptor.ShaderRegister = 0;
    transformBufferIndex_                      = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[0]);
    // CameraTransform ... 1
    rootParameter[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter[1].Descriptor.ShaderRegister = 2;
    cameraBufferIndex_                         = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[1]);
    // Material ... 2
    rootParameter[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[2].Descriptor.ShaderRegister = 0;
    materialBufferIndex_                       = (int32_t)texShaderInfo.pushBackRootParameter(rootParameter[2]);
    // Texture ... 3
    // DescriptorTable を使う
    // NOTE: 下記 pushBackRootParameter(rootParameter[7]) は rootParameter[3] を渡す意図と思われるが、
    //       実際には rootParameter は要素数5([0]~[4])の配列であり [7] は範囲外アクセスになっている。
    //       ロジック修正は本コメント付与作業の対象外のため変更していない。要レビュー。
    rootParameter[3].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    textureBufferIndex_               = static_cast<int32_t>(texShaderInfo.pushBackRootParameter(rootParameter[7]));
    // outlineBuffer ... 4
    // NOTE: 次行は rootParameter[4].Descriptor.ShaderRegister を設定する意図と思われるが、
    //       実際には rootParameter[2](Material用)の ShaderRegister を書き換えてしまっている。
    //       さらに直後の pushBackRootParameter(rootParameter[8]) も配列範囲外アクセス。
    //       いずれもロジック修正は対象外のため変更していない。要レビュー。
    rootParameter[4].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[4].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameter[2].Descriptor.ShaderRegister = 1;
    outlineParamBufferIndex_                   = static_cast<int32_t>(texShaderInfo.pushBackRootParameter(rootParameter[8]));

    D3D12_DESCRIPTOR_RANGE textureRange[1] = {};
    textureRange[0].BaseShaderRegister     = 0;
    textureRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    textureRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    textureRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    texShaderInfo.SetDescriptorRange2Parameter(textureRange, 1, textureBufferIndex_);

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
    texShaderInfo.pushBackSamplerDesc(staticSampler);
    /// Sampler
    ///=================================================

#pragma region "InputElement"
    D3D12_INPUT_ELEMENT_DESC inputElementDesc = {};
    inputElementDesc.SemanticName             = "POSITION"; /*Semantics*/
    inputElementDesc.SemanticIndex            = 0; /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format                   = DXGI_FORMAT_R32G32B32A32_FLOAT; // float 4
    inputElementDesc.AlignedByteOffset        = D3D12_APPEND_ALIGNED_ELEMENT;
    texShaderInfo.pushBackInputElementDesc(inputElementDesc);

    inputElementDesc.SemanticName      = "TEXCOORD"; /*Semantics*/
    inputElementDesc.SemanticIndex     = 0;
    inputElementDesc.Format            = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    texShaderInfo.pushBackInputElementDesc(inputElementDesc);

    inputElementDesc.SemanticName      = "NORMAL"; /*Semantics*/
    inputElementDesc.SemanticIndex     = 0;
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    texShaderInfo.pushBackInputElementDesc(inputElementDesc);

    inputElementDesc.SemanticName      = "COLOR"; /*Semantics*/
    inputElementDesc.SemanticIndex     = 0; /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDesc.Format            = DXGI_FORMAT_R32G32B32A32_FLOAT; // float 4
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    texShaderInfo.pushBackInputElementDesc(inputElementDesc);

#pragma endregion

    ///=================================================
    /// BlendMode ごとの Psoを作成
    ///=================================================

    // 背面法アウトライン: 頂点シェーダで法線方向に頂点を外側へ膨らませたモデルを描画し、
    // 表面(FRONT)を消して裏返った面だけを見せることで、輪郭部分にだけ拡大コピーがはみ出して見える
    texShaderInfo.changeCullMode(D3D12_CULL_MODE_FRONT);
    pso_ = shaderManager->CreatePso(kPsoKey, texShaderInfo, dxDevice->device_);
}

/// <summary>
/// レンダリング開始処理
/// </summary>
void OriGine::OutlineRenderSystem::RenderStart() {
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand_->GetCommandList();

    /// target の設定
    // renderTarget_ はダブルバッファ構成。PreDraw() でフロントバッファを書き込み対象にし、
    // DrawTexture() で直前のバックバッファ内容をブリットしてから、このあとアウトライン用PSOへ切り替える
    renderTarget_->PreDraw();
    renderTarget_->DrawTexture();

    // PSOとRootSignatureの設定(パラメーターを設定するため,とりあえずPSOをセット)
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);

    // Cameraのセット (RootParameter[cameraBufferIndex_] = b2 の gViewProjection)
    CameraManager* cameraManager = CameraManager::GetInstance();
    cameraManager->DataConvertToBuffer(GetScene());
    cameraManager->SetBufferForRootParameter(GetScene(), commandList, cameraBufferIndex_);
}

/// <summary>
/// レンダリング処理
/// </summary>
void OriGine::OutlineRenderSystem::Rendering() {
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand_->GetCommandList();

    RenderStart();

    for (auto& entry : activeEntries_) {
        // NOTE: outlineWidth/outlineColor (Outline.VS.hlsl の gOutlineParam, 本来は
        //       outlineParamBufferIndex_ に対応するはず)を、ここでは cameraBufferIndex_ に
        //       バインドしている。RenderStart() で一度カメラ用バッファを同じスロットにセット済みのため、
        //       この呼び出しでカメラ用バッファの内容がアウトラインパラメータで上書きされてしまう。
        //       ロジック修正は本コメント付与作業の対象外のため変更していない。要レビュー。
        entry.outlineComp->paramData.SetForRootParameter(commandList, cameraBufferIndex_);

        // modelMesh
        for (auto& modelMesh : entry.modelMeshRenderers) {
            uint32_t index = 0;

            auto& meshGroup = modelMesh->GetMeshGroup();
            for (auto& mesh : *meshGroup) {
                D3D12_GPU_DESCRIPTOR_HANDLE textureHandle       = AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(modelMesh->GetTextureIndex(index)).srv.GetGpuHandle();
                const IConstantBuffer<Transform>& meshTransform = modelMesh->GetTransformBuff(index);
                auto& materialBuff                              = modelMesh->GetMaterialBuff(index);
                Material* material                              = nullptr;
                ComponentHandle materialHandle                  = modelMesh->GetMaterialHandle(index);

                // ============================= Viewのセット ============================= //
                commandList->IASetVertexBuffers(0, 1, &mesh.GetVBView());
                commandList->IASetIndexBuffer(&mesh.GetIBView());

                // ============================= Transformのセット ============================= //
                // RootParameter[transformBufferIndex_] = VS b0 (gWorldTransform)
                meshTransform.ConvertToBuffer();
                meshTransform.SetForRootParameter(commandList, transformBufferIndex_);

                // ============================= Materialのセット ============================= //
                material = GetComponent<Material>(materialHandle);
                if (material) {
                    material->UpdateUvMatrix();
                    materialBuff.ConvertToBuffer(*material);

                    if (material->hasCustomTexture()) {
                        textureHandle = material->GetCustomTexture()->srv_.GetGpuHandle();
                    }
                }

                // RootParameter[materialBufferIndex_] = PS b0 (gMaterial)
                materialBuff.SetForRootParameter(commandList, materialBufferIndex_);

                // ============================= テクスチャの設定 ============================= //
                // RootParameter[textureBufferIndex_] = PS t0 (gTexture)。
                // マテリアルが専用テクスチャを持つ場合はそちらを優先する
                commandList->SetGraphicsRootDescriptorTable(
                    textureBufferIndex_, textureHandle);

                // ============================= 描画 ============================= //
                commandList->DrawIndexedInstanced(UINT(mesh.GetIndexSize()), 1, 0, 0, 0);

                ++index;
            }
        }
        // primitive Mesh
        for (auto& primMesh : entry.primitiveMeshRenderers) {
            for (const auto& mesh : *primMesh->GetMeshGroup()) {
                auto& transformBuff = primMesh->GetTransformBuff();
                auto& materialBuff  = primMesh->GetMaterialBuff();

                D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(primMesh->GetTextureIndex()).srv.GetGpuHandle();

                Material* material    = nullptr;
                int32_t materialIndex = primMesh->GetMaterialIndex();
                // ============================= Materialのセット ============================= //
                if (materialIndex >= 0) {
                    material = GetComponent<Material>(primMesh->GetHostEntityHandle(), static_cast<uint32_t>(materialIndex));
                    if (material) {
                        material->UpdateUvMatrix();
                        materialBuff.ConvertToBuffer(*material);

                        if (material->hasCustomTexture()) {
                            textureHandle = material->GetCustomTexture()->srv_.GetGpuHandle();
                        }
                    }
                }

                // ============================= テクスチャの設定 ============================= //

                commandList->SetGraphicsRootDescriptorTable(
                    textureBufferIndex_,
                    textureHandle);

                // ============================= Viewのセット ============================= //
                commandList->IASetVertexBuffers(0, 1, &mesh.GetVBView());
                commandList->IASetIndexBuffer(&mesh.GetIBView());

                // ============================= Transformのセット ============================= //
                transformBuff->UpdateMatrix();
                transformBuff.ConvertToBuffer();
                transformBuff.SetForRootParameter(commandList, transformBufferIndex_);

                // ============================= Materialのセット ============================= //
                materialBuff.SetForRootParameter(commandList, materialBufferIndex_);

                // ============================= 描画 ============================= //
                commandList->DrawIndexedInstanced(UINT(mesh.GetIndexSize()), 1, 0, 0, 0);
            }
        }
    }

    RenderEnd();

    activeEntries_.clear();
}

/// <summary>
/// レンダリング終了処理
/// </summary>
void OriGine::OutlineRenderSystem::RenderEnd() {
    /// target の設定
    // フロントバッファを読み取り可能な状態へ遷移し、フロント/バックのインデックスを入れ替える
    renderTarget_->PostDraw();
}

/// <summary>
/// ポストエフェクトに使用するコンポーネントを有効な場合にリスト化する等の前処理
/// </summary>
/// <param name="_entity">エンティティハンドル</param>
void OriGine::OutlineRenderSystem::DispatchComponent(const EntityHandle& _entity) {
    auto* outlineComp = GetComponent<OutlineComponent>(_entity);

    if (!outlineComp || !outlineComp->isActive) {
        return;
    }
    OutlineEntry entry;
    entry.outlineComp = outlineComp;

    auto entityTransform = GetComponent<Transform>(_entity);

    if (entityTransform) {
        entityTransform->UpdateMatrix();
    }

    auto& modelMeshRenderers = GetComponents<ModelMeshRenderer>(_entity);
    if (!modelMeshRenderers.empty()) {
        for (auto& renderer : modelMeshRenderers) {
            if (renderer.GetMeshGroup()->empty() || !renderer.IsRender()) {
                continue;
            }

            for (int32_t i = 0; i < static_cast<int32_t>(renderer.GetMeshGroup()->size()); ++i) {
                ///==============================
                /// Transformの更新 (meshごと)
                ///==============================
                auto& transform = renderer.GetTransformBuff(i);

                if (transform->parent == nullptr) {
                    transform->parent = entityTransform;
                }

                transform->UpdateMatrix();
                transform.ConvertToBuffer();
            }

            entry.modelMeshRenderers.push_back(&renderer);
        }
    }

    // 各プリミティブ形状のレンダラー(Plane/Ring/Box/Sphere/Cylinder)を共通処理でエントリーに追加するヘルパー
    auto dispatchPrimitive = [this, _entity, entityTransform, &entry](auto& renderers) {
        for (auto& renderer : renderers) {
            if (!renderer.IsRender()) {
                continue;
            }

            ///==============================
            /// Transformの更新
            ///==============================
            auto& transform = renderer.GetTransformBuff();

            if (transform->parent == nullptr) {
                transform->parent = entityTransform;
            }

            transform->UpdateMatrix();
            transform.ConvertToBuffer();

            entry.primitiveMeshRenderers.push_back(&renderer);
        }
    };

    dispatchPrimitive(GetComponents<PlaneRenderer>(_entity));
    dispatchPrimitive(GetComponents<RingRenderer>(_entity));
    dispatchPrimitive(GetComponents<BoxRenderer>(_entity));
    dispatchPrimitive(GetComponents<SphereRenderer>(_entity));
    dispatchPrimitive(GetComponents<CylinderRenderer>(_entity));

    // meshが存在しないなら、登録しない
    if (entry.modelMeshRenderers.empty() && entry.primitiveMeshRenderers.empty()) {
        return;
    }

    activeEntries_.push_back(entry);
}

/// <summary>
/// ポストレンダリングをスキップするかどうか
/// </summary>
/// <returns>true = スキップする / false = スキップしない</returns>
bool OriGine::OutlineRenderSystem::ShouldSkipPostRender() const {
    return activeEntries_.empty();
}
