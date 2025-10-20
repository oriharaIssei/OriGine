#pragma once

/// stl
#include <memory>
#include <string>
#include <vector>

/// engine
#include "scene/Scene.h"
// ECS
#include "component/ComponentArray.h"
#include "component/IComponent.h"
#include "entity/Entity.h"
#include "ECS/system/SystemCategory.h"

/// external
#include "logger/Logger.h"
/// util
#include "util/deltaTime/DeltaTime.h"

/// <summary>
/// System Interface
/// SystemはECSの処理単位であり, EntityとComponentを操作してゲームロジックを実装する.
/// </summary>
class ISystem {
public:
    ISystem(SystemCategory _category, int32_t _priority = 0) : category_(_category), priority_(_priority) {};
    virtual ~ISystem() = default;

    virtual void Initialize() = 0;
    virtual void Update();
    virtual void Edit();
    virtual void Finalize() = 0;

    void eraseDeadEntity();

protected:
    virtual void UpdateEntity([[maybe_unused]] Entity* _entity) {}

    /// ==========================================
    // システム内で使用するであろう 便利関数群
    /// ==========================================

    Entity* getEntity(int32_t _entityID);
    Entity* getUniqueEntity(const std::string& _dataTypeName);
    int32_t CreateEntity(const std::string& _dataTypeName, bool _isUnique = false);

    IComponentArray* getComponentArray(const std::string& _typeName);
    template <IsComponent ComponentType>
    ComponentArray<ComponentType>* getComponentArray() {
        if (scene_ == nullptr) {
            LOG_ERROR("ComponentRepository is not set.");
            return nullptr;
        }
        return scene_->getComponentRepositoryRef()->getComponentArray<ComponentType>();
    }

    template <IsComponent ComponentType>
    ComponentType* getComponent(Entity* _entity, uint32_t _index = 0) {
        if (scene_ == nullptr) {
            LOG_ERROR("ComponentRepository is not set.");
            return nullptr;
        }
        return scene_->getComponentRepositoryRef()->getComponent<ComponentType>(_entity, _index);
    }
    template <IsComponent ComponentType>
    std::vector<ComponentType>* getComponents(Entity* _entity) {
        if (scene_ == nullptr) {
            LOG_ERROR("ComponentRepository is not set.");
            return nullptr;
        }
        return scene_->getComponentRepositoryRef()->getComponents<ComponentType>(_entity);
    }

    void addComponent(Entity* _entity, const std::string& _typeName, IComponent* _component, bool _doInitialize = true);
    template <IsComponent ComponentType>
    void addComponent(Entity* _entity, ComponentType _component, bool _doInitialize = true) {
        if (scene_ == nullptr) {
            LOG_ERROR("ComponentRepository is not set.");
            return;
        }
        getComponentArray<ComponentType>()->add(_entity, _component, _doInitialize);
    }

protected:
    std::vector<int32_t> entityIDs_;

#ifdef _DEBUG
    DeltaTime deltaTimer_;
    float runningTime_ = 0.0f; // deltaTime
#endif // _DEBUG

private:
    Scene* scene_  = nullptr;
    bool isActive_ = false;
    SystemCategory category_;

    int32_t priority_ = 0;

public: // ========================================== accessor ========================================== //
    Scene* getScene() const {
        return scene_;
    }
    void setScene(Scene* _scene) {
        scene_ = _scene;
    }
    SystemCategory getCategory() const {
        return category_;
    }
    const std::vector<int32_t>& getEntityIDs() const {
        return entityIDs_;
    }
    int32_t getEntityCount() const {
        return static_cast<int32_t>(entityIDs_.size());
    }

    bool hasEntity(const Entity* _entity) const {
        return std::find(entityIDs_.begin(), entityIDs_.end(), _entity->getID()) != entityIDs_.end();
    }

    void addEntity(Entity* _entity) {
        // 重複登録を防ぐ
        if (std::find(entityIDs_.begin(), entityIDs_.end(), _entity->getID()) != entityIDs_.end()) {
            return;
        }
        entityIDs_.push_back(_entity->getID());
    }
    void removeEntity(Entity* _entity) {
        entityIDs_.erase(std::remove(entityIDs_.begin(), entityIDs_.end(), _entity->getID()), entityIDs_.end());
    }
    void clearEntities() {
        entityIDs_.clear();
    }

    void setPriority(int32_t _priority) {
        priority_ = _priority;
    }
    int32_t getPriority() const {
        return priority_;
    }

#ifdef _DEBUG
    float getRunningTime() const { return runningTime_; }
#endif // _DEBUG

    bool isActive() const {
        return isActive_;
    }
    void setIsActive(bool _isActive) {
        isActive_ = _isActive;
    }
};

// Systemを継承しているかどうか
template <typename T>
concept IsSystem = std::is_base_of<ISystem, T>::value;

