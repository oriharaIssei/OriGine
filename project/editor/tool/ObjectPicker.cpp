#include "ObjectPicker.h"

/// engine
#include "Engine.h"

#include "camera/CameraManager.h"

// directX12
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

/// ECS
#include "entity/Entity.h"
// component
#include "component/ComponentRepository.h"

#include "component/renderer/MeshRenderer.h"
#include "component/renderer/primitive/BoxRenderer.h"
#include "component/renderer/primitive/CylinderRenderer.h"
#include "component/renderer/primitive/PlaneRenderer.h"
#include "component/renderer/primitive/RingRenderer.h"
#include "component/renderer/primitive/SphereRenderer.h"

using namespace OriGine;

ObjectPicker::ObjectPicker() {}
ObjectPicker::~ObjectPicker() {}

void ObjectPicker::Initialize() {
    auto& device = Engine::GetInstance()->GetDxDevice()->device_;

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    raytracingScene_ = std::make_unique<RaytracingScene>();
    raytracingScene_->Initialize();

    sceneDataBuffer_.CreateBuffer(device);

    constexpr size_t initialPickedObjectBufferSize = 1;
    pickedObjectBuffer_.CreateBuffer(initialPickedObjectBufferSize);

    CreatePSO();
}

void ObjectPicker::Finalize() {
    if (pso_) {
        pso_ = nullptr;
    }

    pickedObjectBuffer_.Finalize();
    sceneDataBuffer_.Finalize();

    dxCommand_->Finalize();

    if (raytracingScene_) {
        raytracingScene_->Finalize();
        raytracingScene_ = nullptr;
    }
}

void ObjectPicker::Activate(Scene* _scene) {
    isActive_ = true;
    scene_    = _scene;
}

void ObjectPicker::Deactivate() {
    isActive_ = false;
    scene_    = nullptr;
}

EntityHandle ObjectPicker::PickedObject(const CameraTransform& _cameraTransform, const Vec2f& _clickPos, const Vec2f& _screenResolution) {
    if (!scene_) {
        LOG_ERROR("Scene is nullptr.");
        return EntityHandle{};
    }

    if (_clickPos[X] < 0.f || _clickPos[X] > _screenResolution[X] || _clickPos[Y] < 0.f || _clickPos[Y] > _screenResolution[Y]) {
        LOG_ERROR("Click position is out of screen bounds.");
        return EntityHandle{};
    }

    EntityHandle resultHandle;

    DispatchMeshForRaytracing();
    UpdateRaytracingScene();

    if (raytracingScene_->IsEmpty()) {
        LOG_ERROR("Raytracing scene is empty.");
        return EntityHandle{};
    }

    // レイトレーシングシーンを使用してピック処理を実行
    auto& commandList = dxCommand_->GetCommandList();
    commandList->SetComputeRootSignature(pso_->rootSignature.Get());
    commandList->SetPipelineState(pso_->pipelineState.Get());

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);

    if (!pickedObjectBuffer_.openData_.empty()) {
        pickedObjectBuffer_.openData_[0].objectIndex = -1;
    }

    // シーンデータの更新
    Matrix4x4 invVpvpMat = _cameraTransform.viewMat * _cameraTransform.projectionMat * MakeMatrix4x4::ViewPort(0.f, 0.f, _screenResolution[X], _screenResolution[Y], 0.f, 1.f);
    invVpvpMat           = invVpvpMat.inverse();

    Vec3f nearPos = ScreenToWorld(_clickPos, 0.f, invVpvpMat);
    Vec3f farPos  = ScreenToWorld(_clickPos, 1.f, invVpvpMat);

    sceneDataBuffer_.openData_.cameraNear   = _cameraTransform.nearZ;
    sceneDataBuffer_.openData_.cameraFar    = _cameraTransform.farZ;
    sceneDataBuffer_.openData_.rayOrigin    = nearPos;
    sceneDataBuffer_.openData_.rayDirection = Vec3f(farPos - nearPos).normalize();
    sceneDataBuffer_.ConvertToBuffer();
    sceneDataBuffer_.SetForComputeRootParameter(commandList, 0);

    // pickedObjectBuffer_ の初期化
    pickedObjectBuffer_.SetForComputePipeline(commandList, 1);

    // レイトレーシングのディスパッチ
    // RaytracingSceneのセット
    if (!raytracingScene_->IsEmpty()) {
        commandList->SetComputeRootShaderResourceView(2, raytracingScene_->GetTlasResource()->GetGPUVirtualAddress());
    }

    // ディスパッチ
    commandList->Dispatch(1, 1, 1);

    dxCommand_->Close();
    dxCommand_->ExecuteCommandAndWait();
    dxCommand_->CommandReset();

    // 結果の取得
    pickedObjectBuffer_.ConvertFromBuffer(dxCommand_.get());

    int32_t pickedIndex = pickedObjectBuffer_.openData_[0].objectIndex;
    if (pickedIndex != -1) {
        auto it = indexToEntityMap_.find(pickedIndex);
        if (it != indexToEntityMap_.end()) {
            resultHandle = it->second;
        }
    }
    indexToEntityMap_.clear();

    return resultHandle;
}

void ObjectPicker::CreatePSO() {
    constexpr const char* psoKey = "ObjectPicker.CS";

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
    shaderManager->LoadShader(psoKey, kShaderDirectory, L"cs_6_5");

    /// ==========================================
    // PSO 設定
    /// ==========================================
    ShaderInfo shaderInfo{};
    shaderInfo.csKey = psoKey;

#pragma region "ROOT_PARAMETER"

    D3D12_ROOT_PARAMETER rootParameters[3] = {};
    // シーンバッファー
    rootParameters[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].Descriptor.ShaderRegister = 0; // b0
    shaderInfo.pushBackRootParameter(rootParameters[0]);

    // pickedObjectBuffer
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[1].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    size_t pickedObjectBufferRootIndex = shaderInfo.pushBackRootParameter(rootParameters[1]);

    D3D12_DESCRIPTOR_RANGE particleDescriptorRange[1]            = {};
    particleDescriptorRange[0].BaseShaderRegister                = 0; // u0
    particleDescriptorRange[0].NumDescriptors                    = 1;
    particleDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    particleDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.SetDescriptorRange2Parameter(particleDescriptorRange, 1, pickedObjectBufferRootIndex);

    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[2].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_SRV;
    shaderInfo.pushBackRootParameter(rootParameters[2]);

#pragma endregion

    /// ==========================================
    // PSOの作成
    /// ==========================================
    pso_ = shaderManager->CreatePso(psoKey, shaderInfo, dxDevice->device_);
}

void OriGine::ObjectPicker::DispatchMeshForRaytracing() {
    const auto& modelRendererComponentArray = scene_->GetComponentArray<ModelMeshRenderer>();
    if (modelRendererComponentArray) {
        for (auto& slot : modelRendererComponentArray->GetSlots()) {
            if (!slot.alive) {
                continue;
            }
            for (size_t compIdx = 0; compIdx < slot.components.size(); ++compIdx) {
                auto& meshRenderer = slot.components[compIdx];
                if (!meshRenderer.IsRender()) {
                    continue;
                }

                auto& meshGroup = meshRenderer.GetMeshGroup();
                for (int32_t meshIdx = 0; meshIdx < meshGroup->size(); ++meshIdx) {
                    RaytracingMeshEntry entry{};
                    entry.mesh = &(*meshGroup)[meshIdx];
                    if (!entry.mesh || !entry.mesh->GetVertexBuffer().IsValid()) {
                        continue;
                    }
                    entry.meshHandle = meshRenderer.GetMeshHandle(meshIdx);
                    entry.worldMat   = meshRenderer.GetTransform(meshIdx).worldMat;
                    entry.isDynamic  = MeshIsDynamic(scene_, slot.owner, meshRenderer.GetMeshRaytracingType(meshIdx), true);
                    meshForRaytracing_.push_back(entry);
                    entityHandles_.push_back(slot.owner);
                }
            }
        }
    }

    auto dispatchPrimitiveRenderers = [this](const auto& primitiveRendererComponentArray) {
        if (!primitiveRendererComponentArray) {
            return;
        }
        for (auto& slot : primitiveRendererComponentArray->GetSlots()) {
            if (!slot.alive) {
                continue;
            }
            for (size_t compIdx = 0; compIdx < slot.components.size(); ++compIdx) {
                auto& meshRenderer = slot.components[compIdx];
                if (!meshRenderer.IsRender()) {
                    continue;
                }
                auto& meshGroup = meshRenderer.GetMeshGroup();
                for (int32_t meshIdx = 0; meshIdx < meshGroup->size(); ++meshIdx) {
                    RaytracingMeshEntry entry{};
                    entry.mesh = &(*meshGroup)[meshIdx];
                    if (!entry.mesh || !entry.mesh->GetVertexBuffer().IsValid()) {
                        continue;
                    }

                    entry.meshHandle = meshRenderer.GetMeshHandle(meshIdx);
                    entry.worldMat   = meshRenderer.GetTransformBuff()->worldMat;
                    entry.isDynamic  = MeshIsDynamic(scene_, slot.owner, meshRenderer.GetMeshRaytracingType(meshIdx));
                    meshForRaytracing_.push_back(entry);
                    entityHandles_.push_back(slot.owner);
                }
            }
        }
    };

    dispatchPrimitiveRenderers(scene_->GetComponentArray<BoxRenderer>());
    dispatchPrimitiveRenderers(scene_->GetComponentArray<CylinderRenderer>());
    dispatchPrimitiveRenderers(scene_->GetComponentArray<PlaneRenderer>());
    dispatchPrimitiveRenderers(scene_->GetComponentArray<RingRenderer>());
    dispatchPrimitiveRenderers(scene_->GetComponentArray<SphereRenderer>());
}

void OriGine::ObjectPicker::UpdateRaytracingScene() {
    if (meshForRaytracing_.empty()) {
        return;
    }
    auto& device      = Engine::GetInstance()->GetDxDevice()->device_;
    auto& commandList = dxCommand_->GetCommandList();
    raytracingScene_->UpdateBlases(device.Get(), commandList.Get(), meshForRaytracing_);

    int32_t instanceIdx = 0;
    for (auto& entry : meshForRaytracing_) {
        RayTracingInstance instance{};
        instance.matrix      = entry.worldMat;
        instance.instanceID  = instanceIdx; // インラインレイトレにはいらない
        instance.mask        = 0xFF;
        instance.hitGroupIdx = 0;
        instance.flags       = 0;
        auto* blas           = raytracingScene_->GetBlas(entry.meshHandle);

        if (!blas) {
            continue;
        }

        instance.blas = blas->GetResultResource().GetResource().Get();

        rayTracingInstances_.push_back(instance);
        indexToEntityMap_[instanceIdx] = entityHandles_[instanceIdx];
        ++instanceIdx;
    }

    raytracingScene_->UpdateTlas(
        device.Get(),
        dxCommand_->GetCommandList().Get(),
        rayTracingInstances_);

    entityHandles_.clear();
    meshForRaytracing_.clear();
};
