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

public:
    virtual void clear() = 0;
    // エンティティの全コンポーネントを削除する（個別削除を行う場合はインデックス等の指定が必要）
    virtual void clearComponent(GameEntity* _hostEntity) = 0;

    virtual void registerEntity(GameEntity* _entity, int32_t _entitySize = 1)          = 0;
    virtual void addComponent(GameEntity* _hostEntity, IComponent* _component)         = 0;
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
    componentType* getComponent(GameEntity* _entity, uint32_t _index = 0) {
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
            entityIndexBind_[const_cast<GameEntity*>(_entity)] = freeIdx;
        } else {
        }
    }

    void add(GameEntity* _hostEntity, const componentType& _component) {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            registerEntity(_hostEntity);
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

    //------------------------------------------------------------------------------------------
    // Remove,Clear 削除系
    //------------------------------------------------------------------------------------------
    void clear() override {
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
        components_[index].clear();
    }

    void removeComponent(GameEntity* _hostEntity, int32_t _componentIndex = 1) override {
        auto it = entityIndexBind_.find(_hostEntity);
        if (it == entityIndexBind_.end()) {
            return;
        }
        uint32_t index = it->second;
        auto& vec      = components_[index];
        vec.erase(std::remove_if(vec.begin(), vec.end(), [vec, _componentIndex](const componentType& comp) {
            return &vec[_componentIndex] == &comp;
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
