#pragma once

/// stl
#include <algorithm>
#include <vector>

/// ECS
// entity
#include "entity/Entity.h"
#include "entity/EntityHandle.h"
// component
#include "component/ComponentArray.h"
#include "component/ComponentHandle.h"
#include "component/ComponentRepository.h"
// system
#include "system/SystemCategory.h"

/// util
#include "deltaTime/DeltaTime.h"

namespace OriGine {

class Scene;
class EntityRepository;

/// <summary>
/// System Interface
/// </summary>
class ISystem {
public:
    ISystem(SystemCategory _category, int32_t _priority = 0)
        : category_(_category), priority_(_priority) {}

    virtual ~ISystem() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    virtual void Initialize() = 0;
    /// <summary>
    /// 終了化処理
    /// </summary>
    virtual void Finalize() = 0;

    /// <summary>
    /// 更新処理
    /// </summary>
    virtual void Run();
    /// <summary>
    /// 編集処理
    /// </summary>
    virtual void Edit();

    /// <summary>
    /// 無効Entityの削除処理
    /// </summary>
    void EraseDeadEntity();

protected:
    /// <summary>
    /// Run() から呼ばれる処理
    /// </summary>
    virtual void Update();
    /// <summary>
    /// 更新処理 (Entity単位)
    /// </summary>
    /// <param name="_entity"></param>
    virtual void UpdateEntity([[maybe_unused]] EntityHandle _handle) {}

    //==========================================
    // ECS Accessors (Handle based)
    //==========================================
    /// <summary>
    /// エンティティを生成する
    /// </summary>
    EntityHandle CreateEntity(const std::string& _dataType, bool _isUnique = false);

    /// <summary>
    /// エンティティを取得する
    /// </summary>
    /// <param name="_entity"></param>
    /// <returns></returns>
    Entity* GetEntity(EntityHandle _entity);

    /// <summary>
    /// ユニークエンティティを生成する
    /// </summary>
    /// <param name="_dataType"></param>
    /// <returns></returns>
    EntityHandle GetUniqueEntity(const std::string& _dataType);

    /// <summary>
    /// コンポーネント配列を取得する
    /// </summary>
    /// <param name="_typeName"></param>
    /// <returns></returns>
    IComponentArray* GetComponentArray(const std::string& _typeName);

    /// <summary>
    /// コンポーネントを取得する
    /// </summary>
    /// <typeparam name="ComponentType"></typeparam>
    /// <param name="_entity"></param>
    /// <returns></returns>
    template <IsComponent ComponentType>
    ComponentType* GetComponent(ComponentHandle _handle);

    /// <summary>
    /// コンポーネントを取得する
    /// </summary>
    /// <typeparam name="ComponentType"></typeparam>
    /// <param name="_entity"></param>
    /// <returns></returns>
    template <IsComponent ComponentType>
    ComponentType* GetComponent(EntityHandle _handle, int32_t _index = 0);

    /// <summary>
    /// コンポーネント配列を取得する
    /// </summary>
    /// <typeparam name="ComponentType"></typeparam>
    /// <param name="_entity"></param>
    /// <returns></returns>
    template <IsComponent ComponentType>
    std::vector<ComponentType>& GetComponents(EntityHandle _entity);

    /// <summary>
    /// コンポーネント配列を取得する
    /// </summary>
    /// <typeparam name="ComponentType"></typeparam>
    /// <returns></returns>
    template <IsComponent ComponentType>
    ComponentArray<ComponentType>* GetComponentArray();

    /// <summary>
    /// コンポーネントを追加する
    /// </summary>
    /// <typeparam name="ComponentType"></typeparam>
    /// <param name="_entity"></param>
    /// <returns></returns>
    template <IsComponent ComponentType>
    ComponentHandle AddComponent(EntityHandle _entity);

    /// <summary>
    /// コンポーネントを追加する
    /// </summary>
    /// <typeparam name="ComponentType"></typeparam>
    /// <param name="_entity"></param>
    /// <returns></returns>
    ComponentHandle AddComponent(EntityHandle _entity, const ::std::string& _typeName);

protected:
    std::vector<EntityHandle> entities_;

#ifndef _RELEASE
    DeltaTime deltaTimer_;
#endif

private:
    Scene* scene_                             = nullptr;
    EntityRepository* entityRepository_       = nullptr;
    ComponentRepository* componentRepository_ = nullptr;

    SystemCategory category_;
    int32_t priority_ = 0;
    bool isActive_    = false;

public:
    //==========================================
    // accessor
    //==========================================
    Scene* GetScene() const { return scene_; }
    void SetScene(Scene* _scene);

    SystemCategory GetCategory() const { return category_; }

    const std::vector<EntityHandle>& GetEntities() const { return entities_; }
    int32_t GetEntityCount() const { return static_cast<int32_t>(entities_.size()); }

    bool HasEntity(EntityHandle _entity) const {
        return std::find_if(
                   entities_.begin(),
                   entities_.end(),
                   [&](const EntityHandle& e) { return e.uuid == _entity.uuid; })
               != entities_.end();
    }

    void AddEntity(EntityHandle _entity) {
        if (!HasEntity(_entity)) {
            entities_.push_back(_entity);
        }
    }

    void RemoveEntity(EntityHandle _entity) {
        entities_.erase(
            std::remove_if(
                entities_.begin(),
                entities_.end(),
                [&](const EntityHandle& e) { return e.uuid == _entity.uuid; }),
            entities_.end());
    }

    void ClearEntities() {
        entities_.clear();
    }

    void SetPriority(int32_t _priority) { priority_ = _priority; }
    int32_t GetPriority() const { return priority_; }

    bool IsActive() const { return isActive_; }
    void SetIsActive(bool _isActive) { isActive_ = _isActive; }
};

template <IsComponent ComponentType>
inline ComponentType* ISystem::GetComponent(ComponentHandle _handle) {
    if (!componentRepository_) {
        LOG_ERROR("ComponentRepository is not set.");
        return nullptr;
    }
    return componentRepository_->GetComponent<ComponentType>(_handle);
}

/// <summary>
/// コンポーネントを取得する (非推奨 ComponentHandleの使用を推奨します)
/// </summary>
/// <typeparam name="ComponentType"></typeparam>
/// <param name="_handle"></param>
/// <param name="_index"></param>
/// <returns></returns>
template <IsComponent ComponentType>
inline ComponentType* ISystem::GetComponent(EntityHandle _handle, int32_t _index) {
    auto* componentArray = GetComponentArray<ComponentType>();
    if (!componentArray) {
        LOG_ERROR("ComponentArray is not found.");
        return nullptr;
    }
    return componentArray->GetComponent(_handle, _index);
}

template <IsComponent ComponentType>
inline std::vector<ComponentType>& ISystem::GetComponents(EntityHandle _entity) {
    auto* componentArray = GetComponentArray<ComponentType>();
    if (!componentArray) {
        LOG_ERROR("ComponentArray is not found.");
        // ダミーの空配列を返す
        static std::vector<ComponentType> emptyComponents;
        return emptyComponents;
    }
    return componentArray->GetComponents(_entity);
}

template <IsComponent ComponentType>
inline ComponentArray<ComponentType>* ISystem::GetComponentArray() {
    if (!componentRepository_) {
        LOG_ERROR("ComponentRepository is not set.");
        return nullptr;
    }
    return componentRepository_->GetComponentArray<ComponentType>();
}

template <IsComponent ComponentType>
inline ComponentHandle ISystem::AddComponent(EntityHandle _entity) {
    return GetComponentArray<ComponentType>()->AddComponent(scene_, _entity);
}

// Systemを継承しているかどうか
template <typename T>
concept IsSystem = ::std::is_base_of<ISystem, T>::value;

} // namespace OriGine
