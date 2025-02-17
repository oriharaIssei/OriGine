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

    ComponentArray()          = default;
    virtual ~ComponentArray() = default;

    void Update();

private:
    // エンティティID -> vector<unique_ptr<componentType>> に変更
    std::map<uint32_t, std::vector<std::unique_ptr<componentType>>> components_;

public:
    // コンポーネントを追加する：指定エンティティの vector に push_back
    void addComponent(const GameEntity* _hostEntity, std::unique_ptr<componentType> _component) {
        uint32_t entityId = _hostEntity->getID();
        // ...existing code...
        components_[entityId].push_back(std::move(_component));
    }
    // エンティティの全コンポーネントを削除する（個別削除を行う場合はインデックス等の指定が必要）
    void removeComponent(const GameEntity* _hostEntity) {
        uint32_t entityId = _hostEntity->getID();
        // ...existing code...
        components_.erase(entityId);
    }
    // エンティティの全コンポーネントの生ポインタを返す
    std::vector<componentType*> getComponents(const GameEntity* _entity) {
        std::vector<componentType*> result;
        auto it = components_.find(_entity->getID());
        if (it != components_.end()) {
            for (auto& comp : it->second)
                result.push_back(comp.get());
        }
        return result;
    }
};

template <IsComponent componentType>
inline void ComponentArray<componentType>::Update() {
    // 各エンティティのvectorを確認し、nullptr の要素を削除した後、各 component の Update を呼び出す
    for (auto it = components_.begin(); it != components_.end();) {
        auto& vec = it->second;
        vec.erase(std::remove_if(vec.begin(), vec.end(), [](const std::unique_ptr<componentType>& comp) {
            return comp == nullptr;
        }),
            vec.end());
        for (auto& comp : vec) {
            if (comp) {
                comp->Update();
            }
        }
        // エンティティのvectorが空の場合は map から削除
        if (vec.empty()) {
            it = components_.erase(it);
        } else {
            ++it;
        }
    }
}
