#pragma once

/// stl
#include <memory>
#include <string>

#include <logger/Logger.h>

/// engine
#include "scene/SceneManager.h"
// directX12
class RenderTexture;
// input
class KeyboardInput;
class MouseInput;
class GamePadInput;
/// ECS
#include "component/ComponentArray.h"
#include "component/ComponentRepository.h"
#include "entity/EntityRepository.h"
class SystemRunner;
class ISystem;
enum class SystemCategory;

/// <summary>
/// 1場面を表すクラス
/// </summary>
class Scene final {
    friend class SceneSerializer;

public:
    Scene(const std::string& _name);
    ~Scene();

    // シーンの初期化
    void Initialize();
    // シーンの更新
    void Update();
    // シーンの描画
    void Render();
    // シーンの終了処理
    void Finalize();

    void InitializeECS();
    void InitializeSceneView();

protected:
    void ExecuteDeleteEntities();

protected:
    SceneManager* sceneManager_ = nullptr;

    std::string name_                         = "NULL";
    std::unique_ptr<RenderTexture> sceneView_ = nullptr;

    std::unique_ptr<EntityRepository> entityRepository_       = nullptr;
    std::unique_ptr<ComponentRepository> componentRepository_ = nullptr;
    std::unique_ptr<SystemRunner> systemRunner_               = nullptr;

    // input
    KeyboardInput* keyInput_ = nullptr;
    MouseInput* mouseInput_  = nullptr;
    GamePadInput* padInput_  = nullptr;

    std::list<int32_t> deleteEntities_; // 削除予定のエンティティIDリスト

    bool isActive_ = false;

public:
    bool isActive() const { return isActive_; }
    void setActive(bool _isActive) { isActive_ = _isActive; }

    SceneManager* getSceneManager() const { return sceneManager_; }
    void setSceneManager(SceneManager* _sceneManager) { sceneManager_ = _sceneManager; }

    KeyboardInput* getKeyboardInput() const { return keyInput_; }
    MouseInput* getMouseInput() const { return mouseInput_; }
    GamePadInput* getGamePadInput() const { return padInput_; }
    void setInputDevices(KeyboardInput* _keyInput, MouseInput* _mouseInput, GamePadInput* _padInput) {
        keyInput_   = _keyInput;
        mouseInput_ = _mouseInput;
        padInput_   = _padInput;
    }

    const std::string& getName() const { return name_; }
    RenderTexture* getSceneView() const { return sceneView_.get(); }

    const EntityRepository* getEntityRepository() const;
    EntityRepository* getEntityRepositoryRef();

    const ComponentRepository* getComponentRepository() const;
    ComponentRepository* getComponentRepositoryRef();

    const SystemRunner* getSystemRunner() const;
    SystemRunner* getSystemRunnerRef();

    /// <summary>
    /// __推奨__
    /// エンティティを削除予定リストに追加する
    /// </summary>
    /// <param name="entityId"></param>
    void addDeleteEntity(int32_t entityId);
    /// <summary>
    /// __非推奨__
    /// エンティティを即時削除する
    /// </summary>
    void deleteEntity(int32_t entityId);

    /// ==========================================
    // Entity 関係
    /// ==========================================
    int32_t getActiveEntityCount() const {
        return entityRepository_->getActiveEntityCount();
    }
    int32_t getInactiveEntityCount() const {
        return entityRepository_->getInactiveEntityCount();
    }

    Entity* getEntity(int32_t entityId) const;
    Entity* getUniqueEntity(const std::string& _dataType) const;

    /// ==========================================
    // Component 関係
    /// =========================================
    template <IsComponent ComponentType>
    ComponentType* getComponent(Entity* _entity, uint32_t index = 0) const;
    template <IsComponent ComponentType>
    ComponentType* getComponent(int32_t entityId, uint32_t index = 0) const;
    template <IsComponent ComponentType>
    std::vector<ComponentType>* getComponents(Entity* _entity) const {
        return componentRepository_->getComponents<ComponentType>(_entity);
    }
    template <IsComponent ComponentType>
    std::vector<ComponentType>* getComponents(int32_t entityId) const;

    IComponentArray* getComponentArray(const std::string& componentTypeName) const {
        return componentRepository_->getComponentArray(componentTypeName);
    }

    bool addComponent(const std::string& _compTypeName, int32_t _entityId, bool _doInitialize = true);
    bool removeComponent(const std::string& _compTypeName, int32_t _entityId, int32_t _componentIndex = 0);

    template <IsComponent ComponentType>
    ComponentArray<ComponentType>* getComponentArray() const {
        return componentRepository_->getComponentArray<ComponentType>();
    }

    /// ==========================================
    // System 関係
    /// ==========================================

    ISystem* getSystem(const std::string& _systemTypeName) const;

    bool registerSystem(const std::string& _systemTypeName, int32_t _priority = 0, bool _activity = true);
    bool unregisterSystem(const std::string& _systemTypeName);
};

template <IsComponent ComponentType>
inline ComponentType* Scene::getComponent(Entity* _entity, uint32_t index) const {
    if (!_entity) {
        LOG_ERROR("Entity is null. EntityName :{}", nameof<ComponentType>());
        return nullptr;
    }
    return componentRepository_->getComponent<ComponentType>(_entity, index);
}

template <IsComponent ComponentType>
inline ComponentType* Scene::getComponent(int32_t entityId, uint32_t index) const {
    Entity* entity = entityRepository_->getEntity(entityId);
    if (!entity) {
        LOG_ERROR("Entity with ID {} not found.", entityId);
        return nullptr;
    }
    return componentRepository_->getComponent<ComponentType>(entity, index);
}

template <IsComponent ComponentType>
inline std::vector<ComponentType>* Scene::getComponents(int32_t entityId) const {
    Entity* entity = entityRepository_->getEntity(entityId);
    if (!entity) {
        LOG_ERROR("Entity with ID {} not found.", entityId);
        return {};
    }
    return componentRepository_->getComponents<ComponentType>(entity);
}
