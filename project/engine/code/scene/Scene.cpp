#include "Scene.h"

/// engine
#define ENGINE_INCLUDE
#define RESOURCE_DIRECTORY
#include "engine/EngineInclude.h"

#include "scene/SceneFactory.h"

#include "winApp/WinApp.h"

// camera
#include "camera/CameraManager.h"

// directX12
#include "directX12/RenderTexture.h"

/// ECS
#include "entity/Entity.h"
// component
#include "component/animation/SkinningAnimationComponent.h"
#include "component/ComponentRepository.h"

#include "component/renderer/ModelMeshRenderer.h"
#include "component/renderer/primitive/BoxRenderer.h"
#include "component/renderer/primitive/CylinderRenderer.h"
#include "component/renderer/primitive/PlaneRenderer.h"
#include "component/renderer/primitive/RingRenderer.h"
#include "component/renderer/primitive/SphereRenderer.h"

// system
#include "system/ISystem.h"
#include "system/SystemRunner.h"

namespace OriGine {

Scene::Scene(const ::std::string& _name) : name_(_name) {}
Scene::~Scene() {}

void Scene::Initialize() {
    isActive_ = true;

    CameraManager::GetInstance()->RegisterSceneCamera(this);

    InitializeSceneView();

    InitializeECS();

    InitializeRaytracingScene();

    /// scene の情報をJsonから変換する(Entity,Component,System)
    SceneFactory factory = SceneFactory();
    factory.BuildSceneByName(this, name_);

    systemRunner_->UpdateCategory<SystemCategory::Initialize>();
}

void Scene::InitializeECS() {
    entityRepository_ = ::std::make_unique<EntityRepository>();
    entityRepository_->Initialize();
    componentRepository_ = ::std::make_unique<ComponentRepository>();
    systemRunner_        = ::std::make_unique<SystemRunner>(this);
}

void Scene::InitializeSceneView() {
    sceneView_ = ::std::make_unique<RenderTexture>();

    sceneView_->Initialize(Config::Rendering::kSwapChainBufferCount, Engine::GetInstance()->GetWinApp()->GetWindowSize(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Config::Rendering::kDefaultClearColor);
    sceneView_->SetTextureName(name_ + "_SceneView");
}

void Scene::InitializeRaytracingScene() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    if (!raytracingScene_) {
        raytracingScene_ = std::make_unique<RaytracingScene>();
    }

    raytracingScene_->Initialize();

    if (!meshForRaytracing_.empty()) {
        meshForRaytracing_.clear();
    }
    if (!rayTracingInstances_.empty()) {
        rayTracingInstances_.clear();
    }
}

void Scene::Update() {
    // 削除予定のエンティティを削除
    ExecuteDeleteEntities();

    if (!systemRunner_) {
        return;
    }
    systemRunner_->UpdateCategory<SystemCategory::Input>();
    systemRunner_->UpdateCategory<SystemCategory::StateTransition>();
    systemRunner_->UpdateCategory<SystemCategory::Movement>();
    systemRunner_->UpdateCategory<SystemCategory::Collision>();
    systemRunner_->UpdateCategory<SystemCategory::Effect>();
}

void Scene::Render() {
    // worldの描画
    sceneView_->PreDraw();
    DispatchMeshForRaytracing();
    // UpdateRaytracingScene();
    systemRunner_->UpdateCategory<SystemCategory::Render>();
    sceneView_->PostDraw();

    // ポストレンダリング
    int32_t postRenderInt = static_cast<int32_t>(SystemCategory::PostRender);
    if (systemRunner_->GetActiveSystems()[postRenderInt].empty() || !systemRunner_->GetCategoryActivity(SystemCategory::PostRender)) {
        return;
    }
    systemRunner_->UpdateCategory<SystemCategory::PostRender>();
}

void Scene::Finalize() {
    systemRunner_->AllUnregisterSystem(true);
    entityRepository_->Finalize();
    componentRepository_->Clear();

    systemRunner_.reset();
    componentRepository_.reset();
    entityRepository_.reset();

    if (raytracingScene_) {
        raytracingScene_->Finalize();
        raytracingScene_.reset();
    }

    if (sceneView_) {
        sceneView_->Finalize();
        sceneView_.reset();
    }

    isActive_ = false;

    CameraManager::GetInstance()->UnregisterSceneCamera(this);
}

void Scene::ExecuteDeleteEntities() {
    for (EntityHandle entityID : deleteEntities_) {
        if (!entityID.IsValid()) {
            LOG_ERROR("Failed Delte Entity : {}", uuids::to_string(entityID.uuid));
            return;
        }
        // コンポーネント を削除
        componentRepository_->RemoveEntity(entityID);
        // システムからエンティティを削除
        systemRunner_->RemoveEntityFromAllSystems(entityID);
        // エンティティを削除
        entityRepository_->RemoveEntity(entityID);
    }
    deleteEntities_.clear();
}

void Scene::DispatchMeshForRaytracing() {
    const auto& modelRendererComponentArray = componentRepository_->GetComponentArray<ModelMeshRenderer>();
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

                    auto* material = componentRepository_->GetComponent<Material>(meshRenderer.GetMaterialHandle(meshIdx));
                    if (!material || !material->enableLighting_) {
                        continue;
                    }

                    entry.meshHandle = meshRenderer.GetMeshHandle(meshIdx);
                    entry.worldMat   = meshRenderer.GetTransform(meshIdx).worldMat;
                    entry.isDynamic  = MeshIsDynamic(this, slot.owner, meshRenderer.GetMeshRaytracingType(meshIdx), true);
                    meshForRaytracing_.push_back(entry);
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

                    Material* material = GetComponent<Material>(slot.owner, meshRenderer.GetMaterialIndex());
                    if (!material || !material->enableLighting_) {
                        continue;
                    }

                    entry.meshHandle = meshRenderer.GetMeshHandle(meshIdx);
                    entry.worldMat   = meshRenderer.GetTransformBuff()->worldMat;
                    entry.isDynamic  = MeshIsDynamic(this, slot.owner, meshRenderer.GetMeshRaytracingType(meshIdx));
                    meshForRaytracing_.push_back(entry);
                }
            }
        }
    };

    dispatchPrimitiveRenderers(componentRepository_->GetComponentArray<BoxRenderer>());
    dispatchPrimitiveRenderers(componentRepository_->GetComponentArray<CylinderRenderer>());
    dispatchPrimitiveRenderers(componentRepository_->GetComponentArray<PlaneRenderer>());
    dispatchPrimitiveRenderers(componentRepository_->GetComponentArray<RingRenderer>());
    dispatchPrimitiveRenderers(componentRepository_->GetComponentArray<SphereRenderer>());
}

void Scene::UpdateRaytracingScene() {
    if (!raytracingScene_) {
        return;
    }
    DispatchMeshForRaytracing();

    auto& device = Engine::GetInstance()->GetDxDevice()->device_;

    raytracingScene_->UpdateBlases(
        device.Get(),
        dxCommand_->GetCommandList().Get(),
        meshForRaytracing_);

    rayTracingInstances_.clear();

    for (auto& entry : meshForRaytracing_) {
        RayTracingInstance instance{};
        instance.matrix      = entry.worldMat;
        instance.instanceID  = 0; // インラインレイトレにはいらない
        instance.mask        = 0xFF; // TODO : on/off
        instance.hitGroupIdx = 0;
        instance.flags       = 0;
        auto* blas           = raytracingScene_->GetBlas(entry.meshHandle);

        if (!blas) {
            continue;
        }

        instance.blas = blas->GetResultResource().GetResource().Get();

        rayTracingInstances_.push_back(instance);
    }
    raytracingScene_->UpdateTlas(
        device.Get(),
        dxCommand_->GetCommandList().Get(),
        rayTracingInstances_);

    meshForRaytracing_.clear();
}

void Scene::AddDeleteEntity(EntityHandle _entityId) {
    if (!_entityId.IsValid()) {
        LOG_ERROR("Invalid entity ID: {}", uuids::to_string(_entityId.uuid));
        return;
    }
    deleteEntities_.push_back(_entityId);
}

const EntityRepository* Scene::GetEntityRepository() const { return entityRepository_.get(); }
EntityRepository* Scene::GetEntityRepositoryRef() { return entityRepository_.get(); }

const ComponentRepository* Scene::GetComponentRepository() const { return componentRepository_.get(); }
ComponentRepository* Scene::GetComponentRepositoryRef() { return componentRepository_.get(); }

const SystemRunner* Scene::GetSystemRunner() const { return systemRunner_.get(); }
SystemRunner* Scene::GetSystemRunnerRef() { return systemRunner_.get(); }

Entity* Scene::GetEntity(EntityHandle _handle) const {
    return entityRepository_->GetEntity(_handle);
}

EntityHandle Scene::GetUniqueEntity(const ::std::string& _dataType) const {
    if (!_dataType.empty()) {
        return entityRepository_->GetUniqueEntity(_dataType);
    }
    LOG_ERROR("Data type is empty.");
    return EntityHandle();
}

EntityHandle Scene::CreateEntity(const ::std::string& _dataType, bool _isUnique) {
    if (!_dataType.empty()) {
        return entityRepository_->CreateEntity(_dataType, _isUnique);
    }
    LOG_ERROR("Data type is empty.");
    return EntityHandle();
}

bool Scene::RegisterUniqueEntity(Entity* _entity) {
    if (_entity) {
        return entityRepository_->RegisterUniqueEntity(_entity);
    }
    LOG_ERROR("Entity is empty.");
    return false;
}

bool Scene::UnregisterUniqueEntity(Entity* _entity) {
    if (_entity) {
        return entityRepository_->UnregisterUniqueEntity(_entity);
    }
    LOG_ERROR("Entity is empty.");
    return false;
}

bool Scene::AddComponent(const ::std::string& _compTypeName, EntityHandle _handle) {
    if (!_handle.IsValid()) {
        LOG_ERROR("Entity with ID '{}' not found.", uuids::to_string(_handle.uuid));
        return false;
    }
    componentRepository_->AddComponent(this, _compTypeName, _handle);
    return true;
}

bool Scene::RemoveComponent(const ::std::string& _compTypeName, EntityHandle _handle, int32_t _componentIndex) {
    if (!_handle.IsValid()) {
        LOG_ERROR("Entity with ID '{}' not found.", uuids::to_string(_handle.uuid));
        return false;
    }
    componentRepository_->RemoveComponent(_compTypeName, _handle, _componentIndex);
    return true;
}

::std::shared_ptr<ISystem> Scene::GetSystem(const ::std::string& _systemTypeName) const {
    if (systemRunner_) {
        return systemRunner_->GetSystem(_systemTypeName);
    }
    LOG_ERROR("SystemRunner is not initialized.");
    return nullptr;
}

bool Scene::RegisterSystem(const ::std::string& _systemTypeName, int32_t _priority, bool _activity) {
    if (systemRunner_) {
        systemRunner_->RegisterSystem(_systemTypeName, _priority, _activity);
        return true;
    }
    LOG_ERROR("SystemRunner is not initialized.");
    return false;
}

bool Scene::UnregisterSystem(const ::std::string& _systemTypeName) {
    if (systemRunner_) {
        systemRunner_->UnregisterSystem(_systemTypeName);
        return true;
    }
    LOG_ERROR("SystemRunner is not initialized.");
    return false;
}

} // namespace OriGine
