#pragma once

/// stl
#include <assert.h>
#include <memory>
// container
#include <set>
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

    virtual void Update() = 0;

public:
    virtual void resize(uint32_t _size)                                         = 0;
    virtual void registerEntity(uint32_t _entityIndex, int32_t _entitySize = 1) = 0;

    // 追加: IComponent を引数にとる addComponent 関数
    virtual void addComponent(const GameEntity* _hostEntity, const IComponent& _component) = 0;

    // エンティティの全コンポーネントを削除する（個別削除を行う場合はインデックス等の指定が必要）
    virtual void clearComponent(const GameEntity* _hostEntity) = 0;
};

///====================================================================================
// ComponentArray
///====================================================================================
/// <summary>
/// コンポーネントを管理するクラス
/// </summary>
template <IsComponent componentType>
class ComponentArray : public IComponentArray {
public:
    using ComponentType = componentType;

    ComponentArray()          = default;
    virtual ~ComponentArray() = default;

private:
    // エンティティID -> vector<unique_ptr<componentType>> に変更
    std::vector<std::vector<componentType>> components_;

public:
    // エンティティの全コンポーネントの取得
    std::vector<componentType>* getComponents(const GameEntity* _entity) {
        return uint32_t(components_[_entity->getID()].size()) == 0 ? nullptr : &components_[_entity->getID()];
    }
    // エンティティの指定したインデックスのコンポーネントの取得
    componentType* getComponent(const GameEntity* _entity, uint32_t _index = 0) {
        return uint32_t(components_[_entity->getID()].size()) <= _index ? nullptr : &components_[_entity->getID()][_index];
    }

    void resize(uint32_t _size) override {
        components_.resize(_size);
    }

    //------------------------------------------------------------------------------------------
    // Add,Register 追加系
    //------------------------------------------------------------------------------------------
    virtual void registerEntity(uint32_t _entityIndex, int32_t _entitySize = 1) {
        components_[_entityIndex] = std::vector<componentType>();
    }
    virtual void addComponent(const GameEntity* _hostEntity, const componentType& _component) {
        uint32_t entityId = _hostEntity->getID();
        components_[entityId].push_back(_component);
    }
    // 追加: IComponentArray のインターフェースを満たすための addComponent オーバーライド
    void addComponent(const GameEntity* _hostEntity, const IComponent& _component) override {
        const componentType* comp = dynamic_cast<const componentType*>(&_component);
        assert(comp != nullptr && "Invalid component type passed to addComponent");
        addComponent(_hostEntity, *comp);
    }

    //------------------------------------------------------------------------------------------
    // Remove,Clear 削除系
    //------------------------------------------------------------------------------------------
    // エンティティの全コンポーネントを削除する（個別削除を行う場合はインデックス等の指定が必要）
    void clearComponent(const GameEntity* _hostEntity) {
        uint32_t entityId = _hostEntity->getID();
        components_[entityId].clear();
    }

    void removeComponent(const GameEntity* _hostEntity, componentType* _component) {
        uint32_t entityId = _hostEntity->getID();
        auto& vec         = components_[entityId];
        vec.erase(std::remove_if(vec.begin(), vec.end(), [_component](const componentType& comp) {
            return &comp == _component;
        }),
            vec.end());
    }
};
