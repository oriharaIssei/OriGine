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
    bool IsActive() const { return isActive_; }
    void SetActive(bool _isActive) { isActive_ = _isActive; }

    SceneManager* GetSceneManager() const { return sceneManager_; }
    void SetSceneManager(SceneManager* _sceneManager) { sceneManager_ = _sceneManager; }

    KeyboardInput* GetKeyboardInput() const { return keyInput_; }
    MouseInput* GetMouseInput() const { return mouseInput_; }
    GamePadInput* GetGamePadInput() const { return padInput_; }
    void SetInputDevices(KeyboardInput* _keyInput, MouseInput* _mouseInput, GamePadInput* _padInput) {
        keyInput_   = _keyInput;
        mouseInput_ = _mouseInput;
        padInput_   = _padInput;
    }

    const std::string& GetName() const { return name_; }
    RenderTexture* GetSceneView() const { return sceneView_.get(); }

    const EntityRepository* GetEntityRepository() const;
    EntityRepository* GetEntityRepositoryRef();

    const ComponentRepository* GetComponentRepository() const;
    ComponentRepository* GetComponentRepositoryRef();

    const SystemRunner* GetSystemRunner() const;
    SystemRunner* GetSystemRunnerRef();

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
    void DeleteEntity(int32_t entityId);

    /// ==========================================
    // Entity 関係
    /// ==========================================
    int32_t GetActiveEntityCount() const {
        return entityRepository_->GetActiveEntityCount();
    }
    int32_t GetInactiveEntityCount() const {
        return entityRepository_->GetInactiveEntityCount();
    }

    Entity* GetEntity(int32_t entityId) const;
    Entity* GetUniqueEntity(const std::string& _dataType) const;

    /// ==========================================
    // Component 関係
    /// =========================================
    template <IsComponent ComponentType>
    ComponentType* GetComponent(Entity* _entity, uint32_t index = 0) const;
    template <IsComponent ComponentType>
    ComponentType* GetComponent(int32_t entityId, uint32_t index = 0) const;
    template <IsComponent ComponentType>
    std::vector<ComponentType>* GetComponents(Entity* _entity) const {
        return componentRepository_->GetComponents<ComponentType>(_entity);
    }
    template <IsComponent ComponentType>
    std::vector<ComponentType>* GetComponents(int32_t entityId) const;

    IComponentArray* GetComponentArray(const std::string& componentTypeName) const {
        return componentRepository_->GetComponentArray(componentTypeName);
    }

    bool AddComponent(const std::string& _compTypeName, int32_t _entityId, bool _doInitialize = true);
    bool RemoveComponent(const std::string& _compTypeName, int32_t _entityId, int32_t _componentIndex = 0);

    template <IsComponent ComponentType>
    ComponentArray<ComponentType>* GetComponentArray() const {
        return componentRepository_->GetComponentArray<ComponentType>();
    }

    /// ==========================================
    // System 関係
    /// ==========================================

    ISystem* GetSystem(const std::string& _systemTypeName) const;

    bool registerSystem(const std::string& _systemTypeName, int32_t _priority = 0, bool _activity = true);
    bool unregisterSystem(const std::string& _systemTypeName);
};

template <IsComponent ComponentType>
inline ComponentType* Scene::GetComponent(Entity* _entity, uint32_t index) const {
    if (!_entity) {
        LOG_ERROR("Entity is null. EntityName :{}", nameof<ComponentType>());
        return nullptr;
    }
    return componentRepository_->GetComponent<ComponentType>(_entity, index);
}

template <IsComponent ComponentType>
inline ComponentType* Scene::GetComponent(int32_t entityId, uint32_t index) const {
    Entity* entity = entityRepository_->GetEntity(entityId);
    if (!entity) {
        LOG_ERROR("Entity with ID {} not found.", entityId);
        return nullptr;
    }
    return componentRepository_->GetComponent<ComponentType>(entity, index);
}

template <IsComponent ComponentType>
inline std::vector<ComponentType>* Scene::GetComponents(int32_t entityId) const {
    Entity* entity = entityRepository_->GetEntity(entityId);
    if (!entity) {
        LOG_ERROR("Entity with ID {} not found.", entityId);
        return {};
    }
    return componentRepository_->GetComponents<ComponentType>(entity);
}
