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

static constexpr uint32_t DEFAULT_COMPONENTARRAY_SIZE = 100;

//====================================================================
// IComponentArray Interface
//====================================================================
class IComponentArray {
public:
    IComponentArray()          = default;
    virtual ~IComponentArray() = default;

    virtual void Initialize(uint32_t _size = DEFAULT_COMPONENTARRAY_SIZE) = 0;
    virtual void Finalize()                                               = 0;

    /// <summary>
    /// 指定したエンティティのコンポーネントを _json に保存する
    /// </summary>
    /// <param name="_entity">保存するコンポーネントをを持つエンティティ</param>
    /// <param name="_json">保存先</param>
    virtual void SaveComponent(Entity* _entity, nlohmann::json& _json) const = 0;
    /// <summary>
    /// 指定したエンティティの指定したインデックスのコンポーネントを _json に保存する
    /// </summary>
    /// <param name="_entity">保存するコンポーネントを持つエンティティ</param>
    /// <param name="_compIndex">保存するコンポーネントのインデックス</param>
    /// <param name="_json">保存先</param>
    virtual void SaveComponent(Entity* _entity, int32_t _compIndex, nlohmann::json& _json) const = 0;

    virtual void LoadComponent(Entity* _entity, const nlohmann::json& _json)                     = 0;
    virtual void LoadComponent(Entity* _entity, int32_t _compIndex, const nlohmann::json& _json) = 0;

    virtual void reserveEntity(Entity* _hostEntity, int32_t _entitySize) = 0;
    virtual void resizeEntity(Entity* _hostEntity, int32_t _entitySize)  = 0;

    virtual void clear()                                                   = 0;
    virtual void clearComponent(Entity* _hostEntity)                       = 0;
    virtual int32_t getComponentSize(Entity* _entity)                      = 0;
    virtual IComponent* getComponent(Entity* _entity, uint32_t _index = 0) = 0;

    virtual IComponent* getFrontComponent(Entity* _entity) {
        return getComponent(_entity, 0);
    }
    virtual IComponent* getBackComponent(Entity* _entity) = 0;

    virtual void registerEntity(Entity* _entity, int32_t _entitySize = 1, bool _doInitialize = true)     = 0;
    virtual int32_t addComponent(Entity* _hostEntity, IComponent* _component, bool _doInitialize = true) = 0;
    virtual int32_t addComponent(Entity* _hostEntity, bool _doInitialize = true)                         = 0;

    virtual void insertComponent(Entity* _hostEntity, IComponent* _component, int32_t _index) = 0;
    virtual void insertComponent(Entity* _hostEntity, int32_t _index)                         = 0;

    virtual void removeComponent(Entity* _hostEntity, int32_t _componentIndex = 0) = 0;
    virtual void removeBackComponent(Entity* _hostEntity)                          = 0;
    virtual void deleteEntity(Entity* _hostEntity)                                 = 0;

    virtual bool hasEntity(Entity* _hostEntity) const         = 0;
    virtual int32_t entityCapacity(Entity* _hostEntity) const = 0;
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
    void registerEntity(Entity* _entity, int32_t _entitySize = 1, bool _doInitialize = true) override;
    void reserveEntity(Entity* _hostEntity, int32_t _size) override;
    void resizeEntity(Entity* _hostEntity, int32_t _size) override;
    void deleteEntity(Entity* _hostEntity) override;

    // ─────────────────────────────
    //  コンポーネント追加・挿入
    // ─────────────────────────────
    int32_t addComponent(Entity* _hostEntity, IComponent* _component, bool _doInitialize = true) override;
    int32_t addComponent(Entity* _hostEntity, bool _doInitialize = true) override;
    int32_t add(Entity* _hostEntity, const componentType& _component, bool _doInitialize = true);

    void insertComponent(Entity* _hostEntity, IComponent* _component, int32_t _index) override;
    void insertComponent(Entity* _hostEntity, int32_t _index) override;

    // ─────────────────────────────
    //  コンポーネント削除・クリア
    // ─────────────────────────────
    void removeComponent(Entity* _hostEntity, int32_t _componentIndex = 0) override;
    void removeBackComponent(Entity* _hostEntity) override;
    void clearComponent(Entity* _hostEntity) override;
    void clear() override;

    // ─────────────────────────────
    //  保存・読み込み
    // ─────────────────────────────
    void SaveComponent(Entity* _entity, nlohmann::json& _json) const override;
    void SaveComponent(Entity* _entity, int32_t _compIndex, nlohmann::json& _json) const override;
    void LoadComponent(Entity* _entity, const nlohmann::json& _json) override;
    void LoadComponent(Entity* _entity, int32_t _compIndex, const nlohmann::json& _json) override;

protected:
    std::vector<std::vector<componentType>> components_;
    std::map<int32_t, uint32_t> entityIndexBind_;

public:
    // ─────────────────────────────
    //  状態取得
    // ─────────────────────────────
    bool hasEntity(Entity* _hostEntity) const override;
    int32_t entityCapacity(Entity* _hostEntity) const override;

    // ─────────────────────────────
    //  アクセッサ
    // ─────────────────────────────
    int32_t getComponentSize(Entity* _entity) override;
    std::vector<componentType>* getComponents(Entity* _entity);
    IComponent* getComponent(Entity* _entity, uint32_t _index = 0) override;
    IComponent* getBackComponent(Entity* _entity) override;

    componentType* getDynamicComponent(Entity* _entity, uint32_t _index = 0);
    componentType* getDynamicFrontComponent(Entity* _entity);
    componentType* getDynamicBackComponent(Entity* _entity);

    std::vector<std::vector<componentType>>* getAllComponents() { return &components_; }
    const std::map<int32_t, uint32_t>& getEntityIndexBind() const { return entityIndexBind_; }
};

template <IsComponent componentType>
void ComponentArray<componentType>::Initialize(uint32_t _size) {
    components_.clear();
    entityIndexBind_.clear();
    components_.reserve(_size);
}

template <IsComponent componentType>
void ComponentArray<componentType>::Finalize() {
    clear();
}

template <IsComponent componentType>
void ComponentArray<componentType>::registerEntity(Entity* _entity, int32_t _entitySize, bool _doInitialize) {
    uint32_t index = static_cast<uint32_t>(components_.size());

    auto& added = components_.emplace_back(std::vector<componentType>());

    added.resize(_entitySize);
    if (_doInitialize) {
        for (auto& comp : added) {
            comp.Initialize(_entity);
        }
    }
    entityIndexBind_[_entity->getID()] = index;
}

template <IsComponent componentType>
void ComponentArray<componentType>::reserveEntity(Entity* _hostEntity, int32_t _size) {
    auto it = entityIndexBind_.find(_hostEntity->getID());
    // エンティティが存在しない場合は何もしない
    if (it == entityIndexBind_.end()) {
        return;
    }
    components_[it->second].reserve(_size);
}

template <IsComponent componentType>
void ComponentArray<componentType>::resizeEntity(Entity* _hostEntity, int32_t _size) {
    auto it = entityIndexBind_.find(_hostEntity->getID());
    // エンティティが存在しない場合は何もしない
    if (it == entityIndexBind_.end()) {
        return;
    }
    components_[it->second].resize(_size);
}

template <IsComponent componentType>
void ComponentArray<componentType>::deleteEntity(Entity* _hostEntity) {
    auto it = entityIndexBind_.find(_hostEntity->getID());
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
int32_t ComponentArray<componentType>::add(Entity* _hostEntity, const componentType& _component, bool _doInitialize) {
    auto it = entityIndexBind_.find(_hostEntity->getID());
    // エンティティが存在しない場合は新規登録
    if (it == entityIndexBind_.end()) {
        uint32_t index = static_cast<uint32_t>(components_.size());
        components_.push_back({_component});
        if (_doInitialize) {
            components_.back().back().Initialize(_hostEntity);
        }
        entityIndexBind_[_hostEntity->getID()] = index;
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
int32_t ComponentArray<componentType>::addComponent(Entity* _hostEntity, IComponent* _component, bool _doInitialize) {
    const componentType* comp = dynamic_cast<const componentType*>(_component);

    assert(comp && "Invalid component type passed to addComponent");

    // エンティティが存在しない場合は新規登録
    auto it = entityIndexBind_.find(_hostEntity->getID());
    if (it == entityIndexBind_.end()) {
        uint32_t index = static_cast<uint32_t>(components_.size());

        // components_ に新しいベクターを追加し、そのベクターにコンポーネントを追加
        components_.push_back({std::move(*comp)});
        if (_doInitialize) {
            components_.back().back().Initialize(_hostEntity);
        }
        entityIndexBind_[_hostEntity->getID()] = index;

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
int32_t ComponentArray<componentType>::addComponent(Entity* _hostEntity, bool _doInitialize) {
    auto it = entityIndexBind_.find(_hostEntity->getID());
    if (it == entityIndexBind_.end()) {
        registerEntity(_hostEntity, 1, _doInitialize);
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
void ComponentArray<componentType>::insertComponent(Entity* _hostEntity, IComponent* _component, int32_t _index) {
    const componentType* comp = dynamic_cast<const componentType*>(_component);
    assert(comp && "Invalid component type passed to insertComponent");
    // エンティティが存在しない場合は何もしない
    auto it = entityIndexBind_.find(_hostEntity->getID());
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
void ComponentArray<componentType>::insertComponent(Entity* _hostEntity, int32_t _index) {
    // エンティティが存在しない場合は何もしない
    auto it = entityIndexBind_.find(_hostEntity->getID());
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
void ComponentArray<componentType>::removeComponent(Entity* _hostEntity, int32_t _componentIndex) {
    // エンティティが存在しない場合は何もしない
    auto it = entityIndexBind_.find(_hostEntity->getID());
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
void ComponentArray<componentType>::removeBackComponent(Entity* _hostEntity) {
    // エンティティが存在しない場合は何もしない
    auto it = entityIndexBind_.find(_hostEntity->getID());
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
void ComponentArray<componentType>::clearComponent(Entity* _hostEntity) {
    // エンティティが存在しない場合は何もしない
    auto it = entityIndexBind_.find(_hostEntity->getID());
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
void ComponentArray<componentType>::clear() {
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
void ComponentArray<componentType>::SaveComponent(Entity* _entity, nlohmann::json& _json) const {
    // エンティティが存在しない場合は何もしない
    auto it = entityIndexBind_.find(_entity->getID());
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
void ComponentArray<componentType>::SaveComponent(Entity* _entity, int32_t _compIndex, nlohmann::json& _json) const {
    // エンティティが存在しない場合は何もしない
    auto it = entityIndexBind_.find(_entity->getID());
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
void ComponentArray<componentType>::LoadComponent(Entity* _entity, const nlohmann::json& _json) {
    // エンティティが存在しない場合は新規登録
    auto it = entityIndexBind_.find(_entity->getID());
    if (it == entityIndexBind_.end()) {
        entityIndexBind_[_entity->getID()] = static_cast<uint32_t>(components_.size());
        components_.emplace_back();
        it = entityIndexBind_.find(_entity->getID());
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
void ComponentArray<componentType>::LoadComponent(Entity* _entity, int32_t _compIndex, const nlohmann::json& _json) {
    // エンティティが存在しない場合は何もしない
    auto it = entityIndexBind_.find(_entity->getID());
    if (it == entityIndexBind_.end()) {
        LOG_ERROR("Entity not found for ID: {}", _entity->getID());
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
bool ComponentArray<componentType>::hasEntity(Entity* _hostEntity) const {
    return entityIndexBind_.contains(_hostEntity->getID());
}

template <IsComponent componentType>
int32_t ComponentArray<componentType>::entityCapacity(Entity* _hostEntity) const {
    auto it = entityIndexBind_.find(_hostEntity->getID());
    if (it == entityIndexBind_.end()) {
        return 0;
    }
    return static_cast<int32_t>(components_[it->second].capacity());
}

template <IsComponent componentType>
int32_t ComponentArray<componentType>::getComponentSize(Entity* _entity) {
    auto it = entityIndexBind_.find(_entity->getID());
    if (it == entityIndexBind_.end()) {
        return 0;
    }
    return static_cast<int32_t>(components_[it->second].size());
}

template <IsComponent componentType>
std::vector<componentType>* ComponentArray<componentType>::getComponents(Entity* _entity) {
    // エンティティが存在しない場合は nullptr を返す
    auto it = entityIndexBind_.find(_entity->getID());
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
IComponent* ComponentArray<componentType>::getComponent(Entity* _entity, uint32_t _index) {
    // エンティティが存在しない場合は nullptr を返す
    auto it = entityIndexBind_.find(_entity->getID());
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
IComponent* ComponentArray<componentType>::getBackComponent(Entity* _entity) {
    // エンティティが存在しない場合は nullptr を返す
    auto it = entityIndexBind_.find(_entity->getID());
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
componentType* ComponentArray<componentType>::getDynamicComponent(Entity* _entity, uint32_t _index) {
    // エンティティが存在しない場合は nullptr を返す
    auto it = entityIndexBind_.find(_entity->getID());
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
componentType* ComponentArray<componentType>::getDynamicFrontComponent(Entity* _entity) {
    return getDynamicComponent(_entity, 0);
}

template <IsComponent componentType>
componentType* ComponentArray<componentType>::getDynamicBackComponent(Entity* _entity) {
    // エンティティが存在しない場合は nullptr を返す
    auto it = entityIndexBind_.find(_entity->getID());
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
