#pragma once

/// stl
#include <assert.h>
#include <memory>
// container
#include <map>
#include <vector>
// utility
#include <concepts>
#include <stdint.h>

/// engine
// ECS
#include "component/IComponent.h"
#include "entity/Entity.h"

/// external
#include "logger/Logger.h"

namespace OriGine {

static constexpr uint32_t kDdefaultComponentArraySize = 100;
static constexpr uint32_t kDdefaultComponentSize      = 6;

//====================================================================
// IComponentArray Interface
//====================================================================
class IComponentArray {
public:
    IComponentArray()          = default;
    virtual ~IComponentArray() = default;

    virtual void Initialize(uint32_t _size = kDdefaultComponentArraySize) = 0;
    virtual void Finalize()                                               = 0;

    /// <summary>
    /// 指定したエンティティのコンポーネントを _json に保存する
    /// </summary>
    /// <param name="_entity">保存するコンポーネントをを持つエンティティ</param>
    /// <param name="_json">保存先</param>
    virtual void SaveComponent(OriGine::Entity* _entity, nlohmann::json& _json) const = 0;
    /// <summary>
    /// 指定したエンティティの指定したインデックスのコンポーネントを _json に保存する
    /// </summary>
    /// <param name="_entity">保存するコンポーネントを持つエンティティ</param>
    /// <param name="_compIndex">保存するコンポーネントのインデックス</param>
    /// <param name="_json">保存先</param>
    virtual void SaveComponent(OriGine::Entity* _entity, int32_t _compIndex, nlohmann::json& _json) const = 0;

    virtual void LoadComponent(OriGine::Entity* _entity, const nlohmann::json& _json)                     = 0;
    virtual void LoadComponent(OriGine::Entity* _entity, int32_t _compIndex, const nlohmann::json& _json) = 0;

    virtual void ReserveEntity(OriGine::Entity* _hostEntity, int32_t _entitySize) = 0;
    virtual void ResizeEntity(OriGine::Entity* _hostEntity, int32_t _entitySize)  = 0;

    virtual void Clear()                                                            = 0;
    virtual void ClearComponent(OriGine::Entity* _hostEntity)                       = 0;
    virtual int32_t GetComponentSize(OriGine::Entity* _entity)                      = 0;
    virtual IComponent* GetComponent(OriGine::Entity* _entity, uint32_t _index = 0) = 0;

    virtual IComponent* GetFrontComponent(OriGine::Entity* _entity) {
        return GetComponent(_entity, 0);
    }
    virtual IComponent* GetBackComponent(OriGine::Entity* _entity) = 0;

    virtual void RegisterEntity(OriGine::Entity* _entity, int32_t _entitySize = 1, bool _doInitialize = true)     = 0;
    virtual int32_t AddComponent(OriGine::Entity* _hostEntity, IComponent* _component, bool _doInitialize = true) = 0;
    virtual int32_t AddComponent(OriGine::Entity* _hostEntity, bool _doInitialize = true)                         = 0;

    virtual void InsertComponent(OriGine::Entity* _hostEntity, IComponent* _component, int32_t _index) = 0;
    virtual void InsertComponent(OriGine::Entity* _hostEntity, int32_t _index)                         = 0;

    virtual void RemoveComponent(OriGine::Entity* _hostEntity, int32_t _componentIndex = 0) = 0;
    virtual void RemoveBackComponent(OriGine::Entity* _hostEntity)                          = 0;
    virtual void DeleteEntity(OriGine::Entity* _hostEntity)                                 = 0;

    virtual bool HasEntity(OriGine::Entity* _hostEntity) const         = 0;
    virtual int32_t EntityCapacity(OriGine::Entity* _hostEntity) const = 0;
};

//====================================================================
// ComponentArray
//====================================================================
template <IsComponent componentType>
class ComponentArray : public IComponentArray {
public:
    using ComponentType = componentType;

    ComponentArray()           = default;
    ~ComponentArray() override = default;

    // ─────────────────────────────
    //  基本関数
    // ─────────────────────────────
    void Initialize(uint32_t _size) override;
    void Finalize() override;

    // ─────────────────────────────
    //  エンティティ操作
    // ─────────────────────────────
    void RegisterEntity(OriGine::Entity* _entity, int32_t _entitySize = 1, bool _doInitialize = true) override;
    void ReserveEntity(OriGine::Entity* _hostEntity, int32_t _size) override;
    void ResizeEntity(OriGine::Entity* _hostEntity, int32_t _size) override;
    void DeleteEntity(OriGine::Entity* _hostEntity) override;

    // ─────────────────────────────
    //  コンポーネント追加・挿入
    // ─────────────────────────────
    int32_t AddComponent(OriGine::Entity* _hostEntity, IComponent* _component, bool _doInitialize = true) override;
    int32_t AddComponent(OriGine::Entity* _hostEntity, bool _doInitialize = true) override;
    int32_t Add(OriGine::Entity* _hostEntity, const componentType& _component, bool _doInitialize = true);

    void InsertComponent(OriGine::Entity* _hostEntity, IComponent* _component, int32_t _index) override;
    void InsertComponent(OriGine::Entity* _hostEntity, int32_t _index) override;

    // ─────────────────────────────
    //  コンポーネント削除・クリア
    // ─────────────────────────────
    void RemoveComponent(OriGine::Entity* _hostEntity, int32_t _componentIndex = 0) override;
    void RemoveBackComponent(OriGine::Entity* _hostEntity) override;
    void ClearComponent(OriGine::Entity* _hostEntity) override;
    void Clear() override;

    // ─────────────────────────────
    //  保存・読み込み
    // ─────────────────────────────
    void SaveComponent(OriGine::Entity* _entity, nlohmann::json& _json) const override;
    void SaveComponent(OriGine::Entity* _entity, int32_t _compIndex, nlohmann::json& _json) const override;
    void LoadComponent(OriGine::Entity* _entity, const nlohmann::json& _json) override;
    void LoadComponent(OriGine::Entity* _entity, int32_t _compIndex, const nlohmann::json& _json) override;

protected:
    std::vector<std::vector<componentType>> components_;
    // エンティティIDとコンポーネント配列のインデックスを紐付けるマップ
    // key: EntityID, value: components_のインデックス
    std::map<int32_t, uint32_t> entityIndexBind_;

public:
    // ─────────────────────────────
    //  状態取得
    // ─────────────────────────────
    bool HasEntity(OriGine::Entity* _hostEntity) const override;
    int32_t EntityCapacity(OriGine::Entity* _hostEntity) const override;

    // ─────────────────────────────
    //  アクセッサ
    // ─────────────────────────────
    int32_t GetComponentSize(OriGine::Entity* _entity) override;
    std::vector<componentType>* GetComponents(OriGine::Entity* _entity);
    IComponent* GetComponent(OriGine::Entity* _entity, uint32_t _index = 0) override;
    IComponent* GetBackComponent(OriGine::Entity* _entity) override;

    componentType* GetDynamicComponent(OriGine::Entity* _entity, uint32_t _index = 0);
    componentType* GetDynamicFrontComponent(OriGine::Entity* _entity);
    componentType* GetDynamicBackComponent(OriGine::Entity* _entity);

    std::vector<std::vector<componentType>>& GetAllComponents() { return components_; }
    const std::map<int32_t, uint32_t>& GetEntityIndexBind() const { return entityIndexBind_; }
};

template <IsComponent componentType>
void ComponentArray<componentType>::Initialize(uint32_t _size) {
    components_.clear();
    entityIndexBind_.clear();
    components_.reserve(_size);
}

template <IsComponent componentType>
void ComponentArray<componentType>::Finalize() {
    Clear();
}

template <IsComponent componentType>
void ComponentArray<componentType>::RegisterEntity(OriGine::Entity* _entity, int32_t _entitySize, bool _doInitialize) {
    uint32_t index = static_cast<uint32_t>(components_.size());

    auto& added = components_.emplace_back(std::vector<componentType>());

    added.resize(_entitySize);
    if (_doInitialize) {
        for (auto& comp : added) {
            comp.Initialize(_entity);
        }
    }
    entityIndexBind_[_entity->GetID()] = index;
}

template <IsComponent componentType>
void ComponentArray<componentType>::ReserveEntity(OriGine::Entity* _hostEntity, int32_t _size) {
    auto it = entityIndexBind_.find(_hostEntity->GetID());
    // エンティティが存在しない場合は何もしない
    if (it == entityIndexBind_.end()) {
        return;
    }
    components_[it->second].reserve(_size);
}

template <IsComponent componentType>
void ComponentArray<componentType>::ResizeEntity(OriGine::Entity* _hostEntity, int32_t _size) {
    auto it = entityIndexBind_.find(_hostEntity->GetID());
    // エンティティが存在しない場合は何もしない
    if (it == entityIndexBind_.end()) {
        return;
    }
    components_[it->second].resize(_size);
}

template <IsComponent componentType>
void ComponentArray<componentType>::DeleteEntity(OriGine::Entity* _hostEntity) {
    auto it = entityIndexBind_.find(_hostEntity->GetID());
    // エンティティが存在しない場合は何もしない
    if (it == entityIndexBind_.end()) {
        return;
    }

    // コンポーネントのFinalizeを呼び出す
    uint32_t index = it->second;
    for (auto& comp : components_[index]) {
        comp.Finalize();
    }

    // コンポーネント配列とマッピングを削除
    components_.erase(components_.begin() + index);
    entityIndexBind_.erase(it);

    // インデックスを更新
    for (auto& [_, mappedIndex] : entityIndexBind_) {
        if (mappedIndex > index) {
            --mappedIndex;
        }
    }
}

template <IsComponent componentType>
int32_t ComponentArray<componentType>::Add(OriGine::Entity* _hostEntity, const componentType& _component, bool _doInitialize) {
    auto it = entityIndexBind_.find(_hostEntity->GetID());
    // エンティティが存在しない場合は新規登録
    if (it == entityIndexBind_.end()) {
        uint32_t index = static_cast<uint32_t>(components_.size());
        components_.push_back({_component});
        if (_doInitialize) {
            components_.back().back().Initialize(_hostEntity);
        }
        entityIndexBind_[_hostEntity->GetID()] = index;
        // 追加したコンポーネントのインデックスを返す
        return 0;
    }

    uint32_t index = it->second;
    components_[index].push_back(_component);
    if (_doInitialize) {
        components_[index].back().Initialize(_hostEntity);
    }
    // 追加したコンポーネントのインデックスを返す
    return static_cast<int32_t>(components_[index].size() - 1);
}

template <IsComponent componentType>
int32_t ComponentArray<componentType>::AddComponent(OriGine::Entity* _hostEntity, IComponent* _component, bool _doInitialize) {
    const componentType* comp = dynamic_cast<const componentType*>(_component);

    assert(comp && "Invalid component type passed to AddComponent");

    // エンティティが存在しない場合は新規登録
    auto it = entityIndexBind_.find(_hostEntity->GetID());
    if (it == entityIndexBind_.end()) {
        uint32_t index = static_cast<uint32_t>(components_.size());

        // components_ に新しいベクターを追加し、そのベクターにコンポーネントを追加
        components_.push_back({std::move(*comp)});
        if (_doInitialize) {
            components_.back().back().Initialize(_hostEntity);
        }
        entityIndexBind_[_hostEntity->GetID()] = index;

        return 0;
    }
    uint32_t index = it->second;
    components_[index].push_back(std::move(*comp));
    if (_doInitialize) {
        components_[index].back().Initialize(_hostEntity);
    }
    return static_cast<int32_t>(components_[index].size() - 1);
}

template <IsComponent componentType>
int32_t ComponentArray<componentType>::AddComponent(OriGine::Entity* _hostEntity, bool _doInitialize) {
    auto it = entityIndexBind_.find(_hostEntity->GetID());
    if (it == entityIndexBind_.end()) {
        RegisterEntity(_hostEntity, 1, _doInitialize);
        return 0;
    }
    uint32_t index = it->second;
    components_[index].push_back(ComponentType());
    if (_doInitialize) {
        components_[index].back().Initialize(_hostEntity);
    }
    return static_cast<int32_t>(components_[index].size() - 1);
}

template <IsComponent componentType>
void ComponentArray<componentType>::InsertComponent(OriGine::Entity* _hostEntity, IComponent* _component, int32_t _index) {
    const componentType* comp = dynamic_cast<const componentType*>(_component);
    assert(comp && "Invalid component type passed to InsertComponent");
    // エンティティが存在しない場合は何もしない
    auto it = entityIndexBind_.find(_hostEntity->GetID());
    if (it == entityIndexBind_.end()) {
        return;
    }

    // インデックスが不正な場合は何もしない
    uint32_t index = it->second;
    if (_index > static_cast<int32_t>(components_[index].size())) {
        return;
    }

    // コンポーネントを挿入
    components_[index].insert(components_[index].begin() + _index, std::move(*comp));
}

template <IsComponent componentType>
void ComponentArray<componentType>::InsertComponent(OriGine::Entity* _hostEntity, int32_t _index) {
    // エンティティが存在しない場合は何もしない
    auto it = entityIndexBind_.find(_hostEntity->GetID());
    if (it == entityIndexBind_.end()) {
        return;
    }
    // インデックスが不正な場合は何もしない
    uint32_t index = it->second;
    if (_index > static_cast<int32_t>(components_[index].size())) {
        return;
    }
    // コンポーネントを挿入
    components_[index].insert(components_[index].begin() + _index, ComponentType());
}

template <IsComponent componentType>
void ComponentArray<componentType>::RemoveComponent(OriGine::Entity* _hostEntity, int32_t _componentIndex) {
    // エンティティが存在しない場合は何もしない
    auto it = entityIndexBind_.find(_hostEntity->GetID());
    if (it == entityIndexBind_.end()) {
        return;
    }
    // インデックスが不正な場合は何もしない
    uint32_t index = it->second;
    auto& vec      = components_[index];
    if (_componentIndex < 0 || _componentIndex >= static_cast<int32_t>(vec.size())) {
        return;
    }

    // コンポーネントを削除
    vec[_componentIndex].Finalize();
    vec.erase(vec.begin() + _componentIndex);
}

template <IsComponent componentType>
void ComponentArray<componentType>::RemoveBackComponent(OriGine::Entity* _hostEntity) {
    // エンティティが存在しない場合は何もしない
    auto it = entityIndexBind_.find(_hostEntity->GetID());
    if (it == entityIndexBind_.end()) {
        return;
    }

    // 末尾コンポーネントを削除
    uint32_t index = it->second;
    auto& vec      = components_[index];
    if (!vec.empty()) {
        vec.back().Finalize();
        vec.pop_back();
    }
}

template <IsComponent componentType>
void ComponentArray<componentType>::ClearComponent(OriGine::Entity* _hostEntity) {
    // エンティティが存在しない場合は何もしない
    auto it = entityIndexBind_.find(_hostEntity->GetID());
    if (it == entityIndexBind_.end()) {
        return;
    }
    // コンポーネントをクリア
    uint32_t index = it->second;
    for (auto& comp : components_[index]) {
        comp.Finalize();
    }
    components_[index].clear();
}

template <IsComponent componentType>
void ComponentArray<componentType>::Clear() {
    // 全コンポーネントをクリア
    for (auto& compArray : components_) {
        for (auto& comp : compArray) {
            comp.Finalize();
        }
    }
    components_.clear();
    entityIndexBind_.clear();
}

template <IsComponent componentType>
void ComponentArray<componentType>::SaveComponent(OriGine::Entity* _entity, nlohmann::json& _json) const {
    // エンティティが存在しない場合は何もしない
    auto it = entityIndexBind_.find(_entity->GetID());
    if (it == entityIndexBind_.end()) {
        return;
    }
    // コンポーネントを保存
    nlohmann::json compVecJson = nlohmann::json::array();
    uint32_t index             = it->second;
    for (auto& comp : components_[index]) {
        compVecJson.emplace_back(comp);
    }
    _json[nameof<componentType>()] = compVecJson;
}

template <IsComponent componentType>
void ComponentArray<componentType>::SaveComponent(OriGine::Entity* _entity, int32_t _compIndex, nlohmann::json& _json) const {
    // エンティティが存在しない場合は何もしない
    auto it = entityIndexBind_.find(_entity->GetID());
    if (it == entityIndexBind_.end()) {
        return;
    }

    // コンポーネントを保存
    uint32_t index = it->second;
    if (_compIndex < 0 || static_cast<uint32_t>(_compIndex) >= components_[index].size()) {
        LOG_ERROR("Invalid component index: {}", _compIndex);
        return;
    }
    _json[nameof<componentType>()] = components_[index][_compIndex];
}

template <IsComponent componentType>
void ComponentArray<componentType>::LoadComponent(OriGine::Entity* _entity, const nlohmann::json& _json) {
    // エンティティが存在しない場合は新規登録
    auto it = entityIndexBind_.find(_entity->GetID());
    if (it == entityIndexBind_.end()) {
        entityIndexBind_[_entity->GetID()] = static_cast<uint32_t>(components_.size());
        components_.emplace_back();
        it = entityIndexBind_.find(_entity->GetID());
    }
    // コンポーネントを読み込み
    uint32_t index = it->second;
    components_[index].clear();
    for (const auto& compJson : _json) {
        components_[index].emplace_back(compJson.get<componentType>());
        components_[index].back().Initialize(_entity);
    }
}

template <IsComponent componentType>
void ComponentArray<componentType>::LoadComponent(OriGine::Entity* _entity, int32_t _compIndex, const nlohmann::json& _json) {
    // エンティティが存在しない場合は何もしない
    auto it = entityIndexBind_.find(_entity->GetID());
    if (it == entityIndexBind_.end()) {
        LOG_ERROR("Entity not found for ID: {}", _entity->GetID());
        return;
    }
    // コンポーネントを読み込み
    uint32_t index = it->second;
    if (_compIndex < 0 || static_cast<uint32_t>(_compIndex) >= components_[index].size()) {
        LOG_ERROR("Invalid component index: {}", _compIndex);
        return;
    }
    components_[index][_compIndex] = _json.get<componentType>();
    components_[index][_compIndex].Initialize(_entity);
}

template <IsComponent componentType>
bool ComponentArray<componentType>::HasEntity(OriGine::Entity* _hostEntity) const {
    return entityIndexBind_.contains(_hostEntity->GetID());
}

template <IsComponent componentType>
int32_t ComponentArray<componentType>::EntityCapacity(OriGine::Entity* _hostEntity) const {
    auto it = entityIndexBind_.find(_hostEntity->GetID());
    if (it == entityIndexBind_.end()) {
        return 0;
    }
    return static_cast<int32_t>(components_[it->second].capacity());
}

template <IsComponent componentType>
int32_t ComponentArray<componentType>::GetComponentSize(OriGine::Entity* _entity) {
    auto it = entityIndexBind_.find(_entity->GetID());
    if (it == entityIndexBind_.end()) {
        return 0;
    }
    return static_cast<int32_t>(components_[it->second].size());
}

template <IsComponent componentType>
std::vector<componentType>* ComponentArray<componentType>::GetComponents(OriGine::Entity* _entity) {
    // エンティティが存在しない場合は nullptr を返す
    auto it = entityIndexBind_.find(_entity->GetID());
    if (it == entityIndexBind_.end()) {
        return nullptr;
    }
    uint32_t index = it->second;
    // コンポーネントが存在しない場合は nullptr を返す
    if (components_[index].empty()) {
        return nullptr;
    }

    return &components_[index];
}

template <IsComponent componentType>
IComponent* ComponentArray<componentType>::GetComponent(OriGine::Entity* _entity, uint32_t _index) {
    // エンティティが存在しない場合は nullptr を返す
    auto it = entityIndexBind_.find(_entity->GetID());
    if (it == entityIndexBind_.end()) {
        return nullptr;
    }

    // インデックスが不正な場合は nullptr を返す
    uint32_t index = it->second;
    if (components_[index].size() <= _index) {
        return nullptr;
    }

    return &components_[index][_index];
}

template <IsComponent componentType>
IComponent* ComponentArray<componentType>::GetBackComponent(OriGine::Entity* _entity) {
    // エンティティが存在しない場合は nullptr を返す
    auto it = entityIndexBind_.find(_entity->GetID());
    if (it == entityIndexBind_.end()) {
        return nullptr;
    }

    // コンポーネントが存在しない場合は nullptr を返す
    uint32_t index = it->second;
    if (components_[index].empty()) {
        return nullptr;
    }

    return &components_[index].back();
}

template <IsComponent componentType>
componentType* ComponentArray<componentType>::GetDynamicComponent(OriGine::Entity* _entity, uint32_t _index) {
    // エンティティが存在しない場合は nullptr を返す
    auto it = entityIndexBind_.find(_entity->GetID());
    if (it == entityIndexBind_.end()) {
        return nullptr;
    }
    // インデックスが不正な場合は nullptr を返す
    uint32_t index = it->second;
    if (components_[index].size() <= _index) {
        return nullptr;
    }

    return &components_[index][_index];
}

template <IsComponent componentType>
componentType* ComponentArray<componentType>::GetDynamicFrontComponent(OriGine::Entity* _entity) {
    return GetDynamicComponent(_entity, 0);
}

template <IsComponent componentType>
componentType* ComponentArray<componentType>::GetDynamicBackComponent(OriGine::Entity* _entity) {
    // エンティティが存在しない場合は nullptr を返す
    auto it = entityIndexBind_.find(_entity->GetID());
    if (it == entityIndexBind_.end()) {
        return nullptr;
    }
    // コンポーネントが存在しない場合は nullptr を返す
    uint32_t index = it->second;
    if (components_[index].empty()) {
        return nullptr;
    }
    return &components_[index].back();
}

} // namespace OriGine
