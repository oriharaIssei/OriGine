#include "OutlineRenderSystem.h"

/// engine
#include "camera/CameraManager.h"
#include "scene/Scene.h"

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

OriGine::OutlineRenderSystem::OutlineRenderSystem() {}
OriGine::OutlineRenderSystem::~OutlineRenderSystem() {}

void OriGine::OutlineRenderSystem::Initialize() {
    CreatePSO();

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    if (!activeEntries_.empty()) {
        activeEntries_.clear();
    }
}

void OriGine::OutlineRenderSystem::Finalize() {
    if (!activeEntries_.empty()) {
        activeEntries_.clear();
    }

    dxCommand_->Finalize();
}

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
    rootParameter[3].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    textureBufferIndex_               = static_cast<int32_t>(texShaderInfo.pushBackRootParameter(rootParameter[7]));
    // outlineBuffer ... 4
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

    // カリングなし
    texShaderInfo.changeCullMode(D3D12_CULL_MODE_FRONT);
    pso_ = shaderManager->CreatePso(kPsoKey, texShaderInfo, dxDevice->device_);
}

void OriGine::OutlineRenderSystem::RenderStart() {
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand_->GetCommandList();

    /// target の設定
    renderTarget_->PreDraw();
    renderTarget_->DrawTexture();

    // PSOとRootSignatureの設定(パラメーターを設定するため,とりあえずPSOをセット)
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);

    // Cameraのセット
    CameraManager* cameraManager = CameraManager::GetInstance();
    cameraManager->DataConvertToBuffer(GetScene());
    cameraManager->SetBufferForRootParameter(GetScene(), commandList, cameraBufferIndex_);
}

void OriGine::OutlineRenderSystem::Rendering() {
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommand_->GetCommandList();

    RenderStart();

    for (auto& entry : activeEntries_) {
        entry.outlineComp->paramData.SetForRootParameter(commandList, cameraBufferIndex_);

        // modelMesh
        for (auto& modelMesh : entry.modelMeshRenderers) {
            uint32_t index = 0;

            auto& meshGroup = modelMesh->GetMeshGroup();
            for (auto& mesh : *meshGroup) {
                D3D12_GPU_DESCRIPTOR_HANDLE textureHandle       = TextureManager::GetDescriptorGpuHandle(modelMesh->GetTextureNumber(index));
                const IConstantBuffer<Transform>& meshTransform = modelMesh->GetTransformBuff(index);
                auto& materialBuff                              = modelMesh->GetMaterialBuff(index);
                Material* material                              = nullptr;
                ComponentHandle materialHandle                  = modelMesh->GetMaterialHandle(index);

                // ============================= Viewのセット ============================= //
                commandList->IASetVertexBuffers(0, 1, &mesh.GetVBView());
                commandList->IASetIndexBuffer(&mesh.GetIBView());

                // ============================= Transformのセット ============================= //
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

                materialBuff.SetForRootParameter(commandList, materialBufferIndex_);

                // ============================= テクスチャの設定 ============================= //

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

                D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = TextureManager::GetDescriptorGpuHandle(primMesh->GetTextureIndex());

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

void OriGine::OutlineRenderSystem::RenderEnd() {
    /// target の設定
    renderTarget_->PostDraw();
}

void OriGine::OutlineRenderSystem::DispatchComponent(EntityHandle _entity) {
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

bool OriGine::OutlineRenderSystem::ShouldSkipPostRender() const {
    return activeEntries_.empty();
}
