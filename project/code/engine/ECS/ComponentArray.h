#pragma once

/// stl
#include <memory>
// container
#include <map>
#include <vector>
// utility
#include <concepts>

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
};

///====================================================================================
// ComponentArray
///====================================================================================
/// <summary>
/// コンポーネントを継承しているかどうかを判定する
/// </summary>
template <typename componentType>
concept IsComponent = std::derived_from<componentType, IComponent>;

/// <summary>
/// コンポーネントを管理するクラス
/// </summary>
template <IsComponent componentType>
class ComponentArray {
public:
    using ComponentType = componentType;

    ComponentArray() {
        components_.resize(capacity_);
    };
    virtual ~ComponentArray() = default;

    void Update();
private:
    /// <summary>
    /// エンティティIDからコンポーネントのインデックスを取得する
    /// </summary>
    std::map<uint32_t, uint32_t> entityToIndexMap_;

    /// <summary>
    /// コンポーネントの配列
    /// </summary>
    std::vector<std::unique_ptr<componentType>> components_;

    uint32_t capacity_ = 0;

public:
    /// <summary>
    /// コンポーネントを追加する
    /// </summary>
    /// <param name="_hostEntity">エンティティ</param>
    /// <param name="_component">コンポーネント</param>
    void addComponent(const GameEntity* _hostEntity,const std::unique_ptr<componentType>& _component) {
        uint32_t entityId = _hostEntity->getID();
        // エンティティIDがコンポーネントのサイズ以上の場合はリサイズする
        if (entityId >= components_.size()) {
            components_.resize(entityId * 2);
        }

        // コンポーネントを追加する
        components_[entityId] = std::move(_component);
    }
    /// <summary>
    /// コンポーネントを削除する
    /// </summary>
    /// <param name="_entity">エンティティID</param>
    void removeComponent(const GameEntity* _hostEntity) {
        uint32_t entityId = _hostEntity->getID();
        // エンティティIDがコンポーネントのサイズ以上の場合はリサイズする
        if (entityId >= components_.size()) {
            components_.resize(entityId * 2);
        }
        // コンポーネントを削除する
        components_[entityId] = nullptr;
    }
    /// <summary>
    /// コンポーネントを取得する
    /// </summary>
    /// <param name="_entity">エンティティ</param>
    /// <returns>コンポーネント</returns>
    componentType* getComponent(const GameEntity* _entity) {
        // コンポーネントを取得する
        return components_[_entity->getID()].get();
    }
};

template <IsComponent componentType>
inline void ComponentArray<componentType>::Update() {
    std::erase_if(components_, [](componentType* _component) {
        return !_component;
    });

    for (auto& component : components_) {
        if (component) {
            component->Update();
        }
    }
}
