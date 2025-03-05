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
#include "Entity.h"

///====================================================================================
// Interface
///====================================================================================
/// <summary>
/// ComponentArray の Interface << テンプレートを 一括で管理するためのインターフェース
/// </summary>
class IComponentArray {
public:
    IComponentArray()          = default;
    virtual ~IComponentArray() = default;

    virtual void Init(uint32_t _size) = 0;

    virtual void SaveComponent(GameEntity* _entity, BinaryWriter& _writer) = 0;
    virtual void LoadComponent(GameEntity* _entity, BinaryReader& _reader) = 0;

public:
    virtual void clear() = 0;
    // エンティティの全コンポーネントを削除する（個別削除を行う場合はインデックス等の指定が必要）
    virtual void clearComponent(GameEntity* _hostEntity) = 0;

    virtual IComponent* getComponent(GameEntity* _entity, uint32_t _index = 0)         = 0;
    virtual void registerEntity(GameEntity* _entity, int32_t _entitySize = 1)          = 0;
    virtual void addComponent(GameEntity* _hostEntity, IComponent* _component)         = 0;
    virtual void addComponent(GameEntity* _hostEntity)                                 = 0;
    virtual void removeComponent(GameEntity* _hostEntity, int32_t _componentIndex = 1) = 0;
    virtual void deleteEntity(GameEntity* _hostEntity)                                 = 0;
};

///====================================================================================
// ComponentArray
///====================================================================================
/// <summary>
/// コンポーネントを管理するクラス
/// </summary>
template <IsComponent componentType>
class ComponentArray
    : public IComponentArray {
public:
    using ComponentType = componentType;

    ComponentArray()          = default;
    virtual ~ComponentArray() = default;

    void Init(uint32_t _size) override {
        // クリア
        components_.clear();
        entityIndexBind_.clear();
        freeIndex_.clear();

        // サイズを設定
        components_.resize(_size);
        for (size_t i = 0; i < _size; ++i) {
            freeIndex_.push_back(uint32_t(i));
        }
    }

    void SaveComponent(GameEntity* _entity, BinaryWriter& _writer) override;
    void LoadComponent(GameEntity* _entity, BinaryReader& _reader) override;

protected:
    // エンティティID -> vector<unique_ptr<componentType>> に変更
    std::vector<std::vector<componentType>> components_;
    std::map<GameEntity*, uint32_t> entityIndexBind_;
    std::vector<uint32_t> freeIndex_;

public:
    // エンティティの全コンポーネントの取得
    std::vector<componentType>* getComponents(GameEntity* _entity) {
        auto it = entityIndexBind_.find(_entity);
        if (it == entityIndexBind_.end()) {
            return nullptr;
        }
        uint32_t index = it->second;
        return components_[index].empty() ? nullptr : &components_[index];
    }

    // エンティティの指定したインデックスのコンポーネントの取得
    componentType* getDynamicComponent(GameEntity* _entity, uint32_t _index = 0) {
        auto it = entityIndexBind_.find(_entity);
        if (it == entityIndexBind_.end()) {
            return nullptr;
        }
        uint32_t index = it->second;
        return components_[index].size() <= _index ? nullptr : &components_[index][_index];
    }
    IComponent* getComponent(GameEntity* _entity, uint32_t _index = 0) override {
        auto it = entityIndexBind_.find(_entity);
        if (it == entityIndexBind_.end()) {
            return nullptr;
        }
        uint32_t index = it->second;
        return components_[index].size() <= _index ? nullptr : &components_[index][_index];
    }

    //------------------------------------------------------------------------------------------
    // Add,Register 追加系
    //------------------------------------------------------------------------------------------
    void registerEntity(GameEntity* _entity, int32_t _entitySize = 1) override {
        if (!freeIndex_.empty()) {
            uint32_t freeIdx = freeIndex_.back();
            freeIndex_.pop_back();
            entityIndexBind_[_entity] = freeIdx;

            components_[entityIndexBind_[_entity]].resize(_entitySize, _entity);
            for (auto& newComp : components_[entityIndexBind_[_entity]]) {
                newComp.Init();
            }
        } else {
            uint32_t oldSize = static_cast<uint32_t>(components_.size());
            components_.resize(oldSize * 2);
            for (uint32_t i = oldSize; i < components_.size(); i++) {
                freeIndex_.push_back(i);
            }

            entityIndexBind_[const_cast<GameEntity*>(_entity)] = static_cast<uint32_t>(components_.size() - 1);
            components_[entityIndexBind_[_entity]].resize(_entitySize, _entity);
            for (auto& newComp : components_[entityIndexBind_[_entity]]) {
                newComp.Init();
            }
        }
    }

    void add(GameEntity* _hostEntity, const componentType& _component) {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            registerEntity(_hostEntity);
            it = entityIndexBind_.find(_hostEntity);
        }
        uint32_t index = it->second;
        components_[index].push_back(_component);
    }

    // 追加
    void addComponent(GameEntity* _hostEntity, IComponent* _component) override {
        const componentType* comp = dynamic_cast<const componentType*>(_component);
        assert(comp != nullptr && "Invalid component type passed to addComponent");

        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            registerEntity(_hostEntity);
            it = entityIndexBind_.find(_hostEntity);
        }
        uint32_t index = it->second;
        components_[index].push_back(*comp);
    }

    // 追加
    void addComponent(GameEntity* _hostEntity) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            registerEntity(_hostEntity);
            return;
            it = entityIndexBind_.find(_hostEntity);
        }
        uint32_t index = it->second;
        components_[index].push_back(ComponentType(_hostEntity));
        components_[index].back().Init();
    }

    //------------------------------------------------------------------------------------------
    // Remove,Clear 削除系
    //------------------------------------------------------------------------------------------
    void clear() override {
        for (auto& compArray : components_) {
            for (auto& comp : compArray) {
                comp.Finalize();
            }
        }
        components_.clear();
        entityIndexBind_.clear();
        freeIndex_.clear();
    }

    // エンティティの全コンポーネントを削除する（個別削除を行う場合はインデックス等の指定が必要）
    void clearComponent(GameEntity* _hostEntity) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            return;
        }
        uint32_t index = it->second;
        for (auto& comp : components_[index]) {
            comp.Finalize();
        }
        components_[index].clear();
    }

    void removeComponent(GameEntity* _hostEntity, int32_t _componentIndex = 1) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            return;
        }
        uint32_t index = it->second;
        auto& vec      = components_[index];
        vec.erase(std::remove_if(vec.begin(), vec.end(), [vec, _componentIndex](componentType& comp) {
            bool isRemove = &vec[_componentIndex] == &comp;
            if (isRemove) {
                comp.Finalize();
            }
            return isRemove;
        }),
            vec.end());
    }

    void deleteEntity(GameEntity* _hostEntity) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            return;
        }
        uint32_t index = it->second;
        components_[index].clear();
        freeIndex_.push_back(index);
        entityIndexBind_.erase(it);
    }
};

template <IsComponent componentType>
inline void ComponentArray<componentType>::SaveComponent(GameEntity* _entity, BinaryWriter& _writer) {
    auto it = entityIndexBind_.find(_entity);
    if (it == entityIndexBind_.end()) {
        return;
    }
    uint32_t index = it->second;
    _writer.Write<uint32_t>(static_cast<uint32_t>(components_[index].size()));
    for (auto& comp : components_[index]) {
        comp.Save(_writer);
    }
}

template <IsComponent componentType>
inline void ComponentArray<componentType>::LoadComponent(GameEntity* _entity, BinaryReader& _reader) {
    uint32_t size;
    _reader.Read<uint32_t>(size);

    registerEntity(_entity, size);

    auto& componentVec = components_[entityIndexBind_[const_cast<GameEntity*>(_entity)]];
    for (auto& comp : componentVec) {
        comp.Load(_reader);
    }
}
