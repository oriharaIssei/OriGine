#pragma once

/// stl
#include <memory>
#include <string>

#include <logger/Logger.h>

/// engine
// directX12
class RenderTexture;
/// ECS
class EntityRepository;
#include "component/ComponentArray.h"
class SystemRunner;
class ISystem;
enum class SystemCategory;

class Scene {
    friend class SceneSerializer;

public:
    Scene(const std::string& _name);
    virtual ~Scene();

    // シーンの初期化
    virtual void Initialize();
    // シーンの更新
    virtual void Update();
    // シーンの描画
    virtual void Render();
    // シーンの終了処理
    virtual void Finalize();

protected:
    void ExecuteDeleteEntities();

protected:
    std::string name_                         = "NULL";
    std::unique_ptr<RenderTexture> sceneView_ = nullptr;

    std::unique_ptr<EntityRepository> entityRepository_       = nullptr;
    std::unique_ptr<ComponentRepository> componentRepository_ = nullptr;
    std::unique_ptr<SystemRunner> systemRunner_               = nullptr;

    std::list<int32_t> deleteEntities_; // 削除予定のエンティティIDリスト

public:
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

    GameEntity* getEntity(int32_t entityId) const;
    GameEntity* getUniqueEntity(const std::string& _dataType) const;

    /// ==========================================
    // Component 関係
    /// =========================================
    template <IsComponent ComponentType>
    ComponentType* getComponent(int32_t entityId, uint32_t index = 0) const {
        return componentRepository_->getComponent<ComponentType>(entityId, index);
    }
    IComponentArray* getComponentArray(const std::string& componentTypeName) const {
        return componentRepository_->getComponentArray(componentTypeName);
    }

    bool addComponent(const std::string& _compTypeName, int32_t _entityId, bool _doInitialize = true);

    template <IsComponent ComponentType>
    ComponentArray<ComponentType>* getComponentArray() const {
        return componentRepository_->getComponentArray<ComponentType>();
    }

    /// ==========================================
    // System 関係
    /// ==========================================

    ISystem* getSystem(const std::string& _systemTypeName, SystemCategory _category) const;

    bool registerSystem(const std::string& _systemTypeName, bool _activity   = true);
    bool unregisterSystem(const std::string& _systemTypeName, bool _activity = true);
};
