#include "Scene.h"

/// engine
#define ENGINE_INCLUDE
#include "scene/SceneManager.h"
#define RESOURCE_DIRECTORY
// directX12
#include "directX12/RenderTexture.h"

// Ecs
#include "entity/Entity.h"
#include "system/ISystem.h"

#include "component/ComponentRepository.h"
#include "engine/EngineInclude.h"
#include "system/SystemRunner.h"

Scene::Scene(const std::string& _name) : name_(_name) {}

Scene::~Scene() {}

void Scene::Initialize() {
    isActive_ = true;

    InitializeSceneView();

    InitializeECS();

    SceneSerializer thisSerializer = SceneSerializer(this);
    thisSerializer.Deserialize();

    systemRunner_->UpdateCategory<SystemCategory::Initialize>();
}

void Scene::InitializeECS() {
    entityRepository_ = std::make_unique<EntityRepository>();
    entityRepository_->Initialize();
    componentRepository_ = std::make_unique<ComponentRepository>();
    systemRunner_        = std::make_unique<SystemRunner>(this);
}

void Scene::InitializeSceneView() {
    sceneView_ = std::make_unique<RenderTexture>();
    sceneView_->Initialize(2, Vec2f(1280.f, 720.f));
    sceneView_->SetTextureName(name_ + "_SceneView");
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

    if (sceneView_) {
        sceneView_->Finalize();
        sceneView_.reset();
    }

    isActive_ = false;
}

void Scene::ExecuteDeleteEntities() {
    for (int32_t entityID : deleteEntities_) {
        DeleteEntity(entityID);
    }
    deleteEntities_.clear();
}

void Scene::AddDeleteEntity(int32_t entityId) {
    if (entityId < 0) {
        LOG_ERROR("Scene::addDeleteEntity: Invalid entity ID: {}", entityId);
        return;
    }
    deleteEntities_.push_back(entityId);
}

void Scene::DeleteEntity(int32_t entityId) {
    Entity* entity = entityRepository_->GetEntity(entityId);
    if (!entity || !entity->IsAlive()) {
        LOG_ERROR("Failed Delte Entity : {}", entityId);
        return;
    }
    // コンポーネント を削除
    componentRepository_->DeleteEntity(entity);
    // システムからエンティティを削除
    systemRunner_->RemoveEntityFromAllSystems(entity);
    // エンティティを削除
    entityRepository_->RemoveEntity(entityId);
}

const EntityRepository* Scene::GetEntityRepository() const { return entityRepository_.get(); }
EntityRepository* Scene::GetEntityRepositoryRef() { return entityRepository_.get(); }

const ComponentRepository* Scene::GetComponentRepository() const { return componentRepository_.get(); }
ComponentRepository* Scene::GetComponentRepositoryRef() { return componentRepository_.get(); }

const SystemRunner* Scene::GetSystemRunner() const { return systemRunner_.get(); }
SystemRunner* Scene::GetSystemRunnerRef() { return systemRunner_.get(); }

Entity* Scene::GetEntity(int32_t entityId) const {
    return entityRepository_->GetEntity(entityId);
}

Entity* Scene::GetUniqueEntity(const std::string& _dataType) const {
    if (!_dataType.empty()) {
        return entityRepository_->GetUniqueEntity(_dataType);
    }
    LOG_ERROR("Scene::GetUniqueEntity: Data type is empty.");
    return nullptr;
}

bool Scene::AddComponent(const std::string& _compTypeName, int32_t _entityId, bool _doInitialize) {
    Entity* entity = entityRepository_->GetEntity(_entityId);
    if (!entity) {
        LOG_ERROR("Scene::AddComponent: Entity with ID '{}' not found.", _entityId);
        return false;
    }
    componentRepository_->AddComponent(_compTypeName, entity, _doInitialize);
    return true;
}

bool Scene::RemoveComponent(const std::string& _compTypeName, int32_t _entityId, int32_t _componentIndex) {
    Entity* entity = entityRepository_->GetEntity(_entityId);
    if (!entity) {
        LOG_ERROR("Scene::RemoveComponent: Entity with ID '{}' not found.", _entityId);
        return false;
    }
    componentRepository_->RemoveComponent(_compTypeName, entity, _componentIndex);
    return true;
}

ISystem* Scene::GetSystem(const std::string& _systemTypeName) const {
    if (systemRunner_) {
        return systemRunner_->GetSystem(_systemTypeName);
    }
    LOG_ERROR("Scene::GetSystem: SystemRunner is not initialized.");
    return nullptr;
}

bool Scene::RegisterSystem(const std::string& _systemTypeName, int32_t _priority, bool _activity) {
    if (systemRunner_) {
        systemRunner_->RegisterSystem(_systemTypeName, _priority, _activity);
        return true;
    }
    LOG_ERROR("Scene::RegisterSystem: SystemRunner is not initialized.");
    return false;
}

bool Scene::UnregisterSystem(const std::string& _systemTypeName) {
    if (systemRunner_) {
        systemRunner_->UnregisterSystem(_systemTypeName);
        return true;
    }
    LOG_ERROR("Scene::UnregisterSystem: SystemRunner is not initialized.");
    return false;
}
