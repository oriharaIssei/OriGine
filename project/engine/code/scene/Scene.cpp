#include "Scene.h"

/// engine
#define ENGINE_INCLUDE
#include "scene/SceneManager.h"
#define RESOURCE_DIRECTORY
// directX12
#include "directX12/RenderTexture.h"

// Ecs
#include "ECS/Entity.h"
#include "system/ISystem.h"

#include "engine/EngineInclude.h"

Scene::Scene(const std::string& _name) {
    name_ = _name;
}

Scene::~Scene() {}

void Scene::Initialize() {
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
    sceneView_->setTextureName(name_ + "_SceneView");
}

void Scene::Update() {
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
    sceneView_->PreDraw();
    systemRunner_->UpdateCategory<SystemCategory::Render>();
    sceneView_->PostDraw();

    int32_t postRenderInt = static_cast<int32_t>(SystemCategory::PostRender);
    if (systemRunner_->getActiveSystems()[postRenderInt].empty() || !systemRunner_->getCategoryActivity(SystemCategory::PostRender)) {
        return;
    }
    sceneView_->PreDraw();
    sceneView_->DrawTexture();
    systemRunner_->UpdateCategory<SystemCategory::PostRender>();
    sceneView_->PostDraw();
}

void Scene::Finalize() {
    systemRunner_->AllUnregisterSystem(true);
    entityRepository_->Finalize();
    componentRepository_->clear();

    systemRunner_.reset();
    componentRepository_.reset();
    entityRepository_.reset();

    if (sceneView_) {
        sceneView_->Finalize();
        sceneView_.reset();
    }
    auto rtvHeap = Engine::getInstance()->getRtvHeap();
    if (rtvHeap) {
    }
}

void Scene::ExecuteDeleteEntities() {
    for (int32_t entityID : deleteEntities_) {
        deleteEntity(entityID);
    }
    deleteEntities_.clear();
}

void Scene::addDeleteEntity(int32_t entityId) {
    if (entityId < 0) {
        LOG_ERROR("Scene::addDeleteEntity: Invalid entity ID: {}", entityId);
        return;
    }
    deleteEntities_.push_back(entityId);
}

void Scene::deleteEntity(int32_t entityId) {
    GameEntity* entity = entityRepository_->getEntity(entityId);
    if (!entity || !entity->isAlive()) {
        LOG_ERROR("Failed Delte Entity : {}", entityId);
        return;
    }
    // コンポーネント を削除
    componentRepository_->deleteEntity(entity);
    // システムからエンティティを削除
    systemRunner_->removeEntityFromAllSystems(entity);
    // エンティティを削除
    entityRepository_->removeEntity(entityId);
}

const EntityRepository* Scene::getEntityRepository() const { return entityRepository_.get(); }
EntityRepository* Scene::getEntityRepositoryRef() { return entityRepository_.get(); }

const ComponentRepository* Scene::getComponentRepository() const { return componentRepository_.get(); }
ComponentRepository* Scene::getComponentRepositoryRef() { return componentRepository_.get(); }

const SystemRunner* Scene::getSystemRunner() const { return systemRunner_.get(); }
SystemRunner* Scene::getSystemRunnerRef() { return systemRunner_.get(); }

GameEntity* Scene::getEntity(int32_t entityId) const {
    return entityRepository_->getEntity(entityId);
}

GameEntity* Scene::getUniqueEntity(const std::string& _dataType) const {
    if (!_dataType.empty()) {
        return entityRepository_->getUniqueEntity(_dataType);
    }
    LOG_ERROR("Scene::getUniqueEntity: Data type is empty.");
    return nullptr;
}

bool Scene::addComponent(const std::string& _compTypeName, int32_t _entityId, bool _doInitialize) {
    GameEntity* entity = entityRepository_->getEntity(_entityId);
    if (!entity) {
        LOG_ERROR("Scene::addComponent: Entity with ID '{}' not found.", _entityId);
        return false;
    }
    componentRepository_->addComponent(_compTypeName, entity, _doInitialize);
    return true;
}

bool Scene::removeComponent(const std::string& _compTypeName, int32_t _entityId, int32_t _componentIndex) {
    GameEntity* entity = entityRepository_->getEntity(_entityId);
    if (!entity) {
        LOG_ERROR("Scene::removeComponent: Entity with ID '{}' not found.", _entityId);
        return false;
    }
    componentRepository_->removeComponent(_compTypeName, entity, _componentIndex);
    return true;
}

ISystem* Scene::getSystem(const std::string& _systemTypeName) const {
    if (systemRunner_) {
        return systemRunner_->getSystem(_systemTypeName);
    }
    LOG_ERROR("Scene::getSystem: SystemRunner is not initialized.");
    return nullptr;
}

bool Scene::registerSystem(const std::string& _systemTypeName, int32_t _priority, bool _activity) {
    if (systemRunner_) {
        systemRunner_->registerSystem(_systemTypeName, _priority, _activity);
        return true;
    }
    LOG_ERROR("Scene::registerSystem: SystemRunner is not initialized.");
    return false;
}

bool Scene::unregisterSystem(const std::string& _systemTypeName) {
    if (systemRunner_) {
        systemRunner_->unregisterSystem(_systemTypeName);
        return true;
    }
    LOG_ERROR("Scene::unregisterSystem: SystemRunner is not initialized.");
    return false;
}
