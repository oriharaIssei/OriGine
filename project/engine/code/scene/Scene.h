#pragma once

/// stl
#include <memory>
#include <string>

/// ECS
// entity
#include "entity/EntityRepository.h"
// component
#include "component/ComponentArray.h"
#include "component/ComponentRepository.h"

/// logger
#include <logger/Logger.h>

namespace OriGine {

/// engine
class SceneManager;
// directX12
class RenderTexture;
// input
class KeyboardInput;
class MouseInput;
class GamepadInput;

/// ECS
// system
class SystemRunner;
class ISystem;

/// <summary>
/// 1場面を表すクラス
/// </summary>
class Scene final {
    friend class SceneFactory;

public:
    Scene(const ::std::string& _name);
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

    ::std::string name_                         = "NULL";
    ::std::unique_ptr<RenderTexture> sceneView_ = nullptr;

    ::std::unique_ptr<EntityRepository> entityRepository_       = nullptr;
    ::std::unique_ptr<ComponentRepository> componentRepository_ = nullptr;
    ::std::unique_ptr<SystemRunner> systemRunner_               = nullptr;

    // input
    KeyboardInput* keyInput_ = nullptr;
    MouseInput* mouseInput_  = nullptr;
    GamepadInput* padInput_  = nullptr;

    ::std::list<EntityHandle> deleteEntities_; // 削除予定のエンティティIDリスト

    bool isActive_ = false;

public:
    bool IsActive() const { return isActive_; }
    void SetActive(bool _isActive) { isActive_ = _isActive; }

    SceneManager* GetSceneManager() const { return sceneManager_; }
    void SetSceneManager(SceneManager* _sceneManager) { sceneManager_ = _sceneManager; }

    KeyboardInput* GetKeyboardInput() const { return keyInput_; }
    MouseInput* GetMouseInput() const { return mouseInput_; }
    GamepadInput* GetGamepadInput() const { return padInput_; }
    void SetInputDevices(KeyboardInput* _keyInput, MouseInput* _mouseInput, GamepadInput* _padInput) {
        keyInput_   = _keyInput;
        mouseInput_ = _mouseInput;
        padInput_   = _padInput;
    }

    const ::std::string& GetName() const { return name_; }
    RenderTexture* GetSceneView() const { return sceneView_.get(); }

    const EntityRepository* GetEntityRepository() const;
    EntityRepository* GetEntityRepositoryRef();

    const ComponentRepository* GetComponentRepository() const;
    ComponentRepository* GetComponentRepositoryRef();

    const SystemRunner* GetSystemRunner() const;
    SystemRunner* GetSystemRunnerRef();

    /// <summary>
    /// エンティティを削除する
    /// 削除タイミングはScene処理の終了後
    /// </summary>
    /// <param name="entityId"></param>
    void AddDeleteEntity(EntityHandle entityId);

    /// ==========================================
    // Entity 関係
    /// =========================================
    Entity* GetEntity(EntityHandle _handle) const;
    EntityHandle GetUniqueEntity(const ::std::string& _dataType) const;

    /// <summary>
    /// エンティティを作成する
    /// </summary>
    /// <param name="_dataType"></param>
    /// <param name="_isUnique"></param>
    /// <returns></returns>
    EntityHandle CreateEntity(const ::std::string& _dataType, bool _isUnique = false);

    /// <summary>
    /// ユニークエンティティを登録する
    /// </summary>
    /// <returns></returns>
    bool RegisterUniqueEntity(Entity* _entity);
    /// <summary>
    /// ユニークエンティティの登録を解除する
    /// </summary>
    bool UnregisterUniqueEntity(Entity* _entity);

    /// ==========================================
    // Component 関係
    /// =========================================
    template <IsComponent ComponentType>
    ComponentType* GetComponent(EntityHandle _handle, uint32_t index = 0) const;

    template <IsComponent ComponentType>
    ::std::vector<ComponentType>& GetComponents(EntityHandle _handle) {
        return componentRepository_->GetComponents<ComponentType>(_handle);
    }

    bool AddComponent(const ::std::string& _compTypeName, EntityHandle _handle);

    template <IsComponent ComponentType>
    bool AddComponent(EntityHandle _handle) {
        return componentRepository_->AddComponent<ComponentType>(this, _handle);
    }
    bool RemoveComponent(const ::std::string& _compTypeName, EntityHandle _handle, int32_t _componentIndex = 0);

    IComponentArray* GetComponentArray(const ::std::string& componentTypeName) const {
        return componentRepository_->GetComponentArray(componentTypeName);
    }
    template <IsComponent ComponentType>
    ComponentArray<ComponentType>* GetComponentArray() const {
        return componentRepository_->GetComponentArray<ComponentType>();
    }

    /// ==========================================
    // System 関係
    /// ==========================================

    ::std::shared_ptr<ISystem> GetSystem(const ::std::string& _systemTypeName) const;

    bool RegisterSystem(const ::std::string& _systemTypeName, int32_t _priority = 0, bool _activity = true);
    bool UnregisterSystem(const ::std::string& _systemTypeName);
};

template <IsComponent ComponentType>
inline ComponentType* Scene::GetComponent(EntityHandle _handle, uint32_t index) const {
    if (!_handle.IsValid()) {
        LOG_ERROR("Entity is null. EntityName :{}", nameof<ComponentType>());
        return nullptr;
    }
    return componentRepository_->GetComponent<ComponentType>(_handle, index);
}
} // namespace OriGine
