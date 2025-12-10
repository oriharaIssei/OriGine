#pragma once

/// stl
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

/// ECS
// entity
#include "entity/Entity.h"
#include "entity/EntityRepository.h"
// component
#include "component/ComponentArray.h"
#include "component/ComponentRepository.h"
#include "component/IComponent.h"
// system
#include "system/SystemCategory.h"

/// util
#include "deltaTime/DeltaTime.h"

/// external
#include "logger/Logger.h"

namespace OriGine {

/// engine
class Scene;

/// <summary>
/// System Interface
/// SystemはECSの処理単位であり, EntityとComponentを操作してゲームロジックを実装する.
/// </summary>
class ISystem {
public:
    ISystem(SystemCategory _category, int32_t _priority = 0) : category_(_category), priority_(_priority) {};
    virtual ~ISystem() = default;

    virtual void Initialize() = 0;
    /// <summary>
    /// 外部から呼び出される更新処理
    /// </summary>
    virtual void Run();
    virtual void Edit();
    virtual void Finalize() = 0;

    void EraseDeadEntity();

protected:
    /// <summary>
    /// Run()で呼び出されるSystem特有の更新処理
    /// </summary>
    virtual void Update();
    virtual void UpdateEntity([[maybe_unused]] OriGine::Entity* _entity) {}

    /// ==========================================
    // システム内で使用するであろう 便利関数群
    /// ==========================================

    OriGine::Entity* GetEntity(int32_t _entityID);
    OriGine::Entity* GetUniqueEntity(const ::std::string& _dataTypeName);
    int32_t CreateEntity(const ::std::string& _dataTypeName, bool _isUnique = false);

    IComponentArray* GetComponentArray(const ::std::string& _typeName);
    template <IsComponent ComponentType>
    ComponentArray<ComponentType>* GetComponentArray() {
        if (componentRepository_ == nullptr) {
            LOG_ERROR("ComponentRepository is not Set.");
            return nullptr;
        }
        return componentRepository_->GetComponentArray<ComponentType>();
    }

    template <IsComponent ComponentType>
    ComponentType* GetComponent(OriGine::Entity* _entity, uint32_t _index = 0) {
        if (componentRepository_ == nullptr) {
            LOG_ERROR("ComponentRepository is not Set.");
            return nullptr;
        }
        return componentRepository_->GetComponent<ComponentType>(_entity, _index);
    }
    template <IsComponent ComponentType>
    ::std::vector<ComponentType>* GetComponents(OriGine::Entity* _entity) {
        if (componentRepository_ == nullptr) {
            LOG_ERROR("ComponentRepository is not Set.");
            return nullptr;
        }
        return componentRepository_->GetComponents<ComponentType>(_entity);
    }

    void AddComponent(OriGine::Entity* _entity, const ::std::string& _typeName, IComponent* _component, bool _doInitialize = true);
    template <IsComponent ComponentType>
    void AddComponent(OriGine::Entity* _entity, ComponentType _component, bool _doInitialize = true) {
        if (componentRepository_ == nullptr) {
            LOG_ERROR("ComponentRepository is not Set.");
            return;
        }
        GetComponentArray<ComponentType>()->Add(_entity, _component, _doInitialize);
    }

protected:
    ::std::vector<int32_t> entityIDs_;

#ifndef _RELEASE
    DeltaTime deltaTimer_;
#endif //! _RELEASE

private:
    Scene* scene_                             = nullptr;
    EntityRepository* entityRepository_       = nullptr;
    ComponentRepository* componentRepository_ = nullptr;
    SystemCategory category_;

    int32_t priority_ = 0;
    bool isActive_    = false;

public: // ========================================== accessor ========================================== //
    Scene* GetScene() const {
        return scene_;
    }
    void SetScene(Scene* _scene);
    SystemCategory GetCategory() const {
        return category_;
    }
    const ::std::vector<int32_t>& GetEntityIDs() const {
        return entityIDs_;
    }
    int32_t GetEntityCount() const {
        return static_cast<int32_t>(entityIDs_.size());
    }

    bool HasEntity(const OriGine::Entity* _entity) const {
        return ::std::find(entityIDs_.begin(), entityIDs_.end(), _entity->GetID()) != entityIDs_.end();
    }

    void AddEntity(OriGine::Entity* _entity) {
        // 重複登録を防ぐ
        if (::std::find(entityIDs_.begin(), entityIDs_.end(), _entity->GetID()) != entityIDs_.end()) {
            return;
        }
        entityIDs_.push_back(_entity->GetID());
    }
    /// <summary>
    /// エンティティをシステムから削除する
    /// </summary>
    /// <param name="_entity"></param>
    void RemoveEntity(OriGine::Entity* _entity) {
        entityIDs_.erase(::std::remove(entityIDs_.begin(), entityIDs_.end(), _entity->GetID()), entityIDs_.end());
    }
    void ClearEntities() {
        entityIDs_.clear();
    }

    void SetPriority(int32_t _priority) {
        priority_ = _priority;
    }
    int32_t GetPriority() const {
        return priority_;
    }

#ifdef _DEBUG
    float GetRunningTime() const { return deltaTimer_.GetDeltaTime(); }
#endif // _DEBUG

    bool IsActive() const {
        return isActive_;
    }
    void SetIsActive(bool _isActive) {
        isActive_ = _isActive;
    }
};

// Systemを継承しているかどうか
template <typename T>
concept IsSystem = ::std::is_base_of<ISystem, T>::value;

} // namespace OriGine
