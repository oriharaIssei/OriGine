#pragma once

/// stl
#include <algorithm>
#include <memory>
#include <string>
// container
#include <array>
#include <map>
#include <queue>
#include <vector>

/// engine
// ECS
#include "component/ComponentArray.h"
#include "Entity.h"
#include "system/ISystem.h"
// util
#include "util/nameof.h"

class EntityComponentSystemManager {
public:
    static EntityComponentSystemManager* getInstance() {
        static EntityComponentSystemManager instance;
        return &instance;
    }

    void Initialize();
    void Run();
    void Finalize();

    void ComponentArraysInitialize();

    void RunInitialize() ;

private:
    EntityComponentSystemManager() {}
    ~EntityComponentSystemManager() {}
    EntityComponentSystemManager(const EntityComponentSystemManager&)            = delete;
    EntityComponentSystemManager* operator=(const EntityComponentSystemManager&) = delete;

    void ShowEntityStack();

private:
    /// <summary>
    /// エンティティ配列
    /// </summary>
    std::vector<GameEntity> entities_;
    std::vector<uint32_t> freeEntityIndex_;
    std::map<std::string, GameEntity*> uniqueEntities_;
    uint32_t entityCapacity_ = 100;

    std::queue<GameEntity*> deleteEntityQueue_;

    /// <summary>
    /// コンポーネント配列
    /// </summary>
    std::map<std::string, std::unique_ptr<IComponentArray>> componentArrays_;

    /// <summary>
    /// システム配列
    /// </summary>
    std::array<std::map<std::string, std::unique_ptr<ISystem>>, int32_t(SystemType::Count)> systems_;

    std::array<std::vector<ISystem*>, int32_t(SystemType::Count)> priorityOrderSystems_;

public: // ============== accessor ==============//
    void resize(uint32_t _newSize);

    // --------------------------------------------------------------------------------------
    //  Entity
    // --------------------------------------------------------------------------------------

    /// <summary>
    /// エンティティの容量を取得する
    /// </summary>
    /// <returns></returns>
    uint32_t getEntityCapacity() const {
        return entityCapacity_;
    }

    /// <summary>
    /// エンティティを作成する
    /// </summary>
    uint32_t registerEntity(const std::string& _entityDataType) {
        if (freeEntityIndex_.empty()) {
            // 容量に空きが無い場合, 容量を増やす
            // 2倍の容量にリサイズ
            this->resize(entityCapacity_ * 2);

            for (auto& [componentID, componentArray] : componentArrays_) {
                componentArray->Initialize(entityCapacity_);
            }
        }
        uint32_t index = freeEntityIndex_.back();
        freeEntityIndex_.pop_back();

        entities_[index]          = GameEntity(_entityDataType, index);
        entities_[index].isAlive_ = true;

        return index;
    }

    /// <summary>
    /// エンティティを取得する
    /// </summary>
    GameEntity* getEntity(int32_t _entityIndex) {
        return &entities_[_entityIndex];
    }

    GameEntity* getUniqueEntity(const std::string& _dataTypeName) const {
        auto itr = uniqueEntities_.find(_dataTypeName);
        if (itr == uniqueEntities_.end()) {
            return nullptr;
        }
        return itr->second;
    }
    bool registerUniqueEntity(const std::string& _dataTypeName, GameEntity* _entity) {
        _entity->isUnique_ = true;

        if (uniqueEntities_.find(_dataTypeName) != uniqueEntities_.end()) {
            return false;
        }

        uniqueEntities_[_dataTypeName] = _entity;
        return true;
    }
    void removeUniqueEntity(const std::string& _dataTypeName) {
        if (uniqueEntities_[_dataTypeName]) {
            uniqueEntities_[_dataTypeName]->isUnique_ = false;
        }
        uniqueEntities_.erase(_dataTypeName);
    }

    const std::vector<GameEntity>& getEntities() const {
        return entities_;
    }

    /// <summary>
    /// 有効なエンティティが後ろに来るようにソート
    /// </summary>
    void sortBackActiveEntities() {
        // isAlive が true のエンティティを前に安定した順序で持ってくる
        auto mid = std::stable_partition(entities_.begin(), entities_.end(), [](const GameEntity& entity) {
            return !entity.isAlive_;
        });

        uint32_t index = 0;
        freeEntityIndex_.clear();

        // アクティブなエンティティに新しいIDを付与し、非アクティブなインデックスを freeEntityIndex_ に追加
        for (auto it = entities_.begin(); it != entities_.end(); ++it) {
            if (it >= mid) {
                it->id_ = int32_t(index++);
            } else {
                freeEntityIndex_.push_back(index++);
                it->id_ = -1;
            }
        }
    }

    /// <summary>
    /// エンティティにコンポーネントを追加する
    /// </summary>
    template <IsComponent componentType>
    void addComponent(uint32_t _entityIndex, const componentType& _component) {
        ComponentArray<componentType>* componentArray = getComponentArray<componentType>();
        componentArray->add(&entities_[_entityIndex], _component);
    }

    /// <summary>
    /// エンティティを削除する
    /// </summary>
    void destroyEntity(GameEntity* _entityIndex) {
        deleteEntityQueue_.push(_entityIndex);
    }

    void clearEntity() {
        entities_.clear();
        freeEntityIndex_.clear();
    }

    /// <summary>
    /// 生きているエンティティを削除する
    /// </summary>
    void clearAliveEntities() {
        auto removedItr = std::remove_if(entities_.begin(), entities_.end(), [](const GameEntity& entity) { return entity.isAlive_; });
        for (auto& itr = removedItr; itr != entities_.end(); ++removedItr) {
            freeEntityIndex_.push_back(itr->id_);

            // エンティティを無効化
            itr->id_       = -1;
            itr->dataType_ = "UNKNOWN";
            itr->isAlive_  = false;
        }
    }

    // --------------------------------------------------------------------------------------
    //  Component
    // --------------------------------------------------------------------------------------
    template <IsComponent componentType>
    void registerComponent() {
        std::string typeName = nameof<componentType>();
        if (componentArrays_.find(typeName) == componentArrays_.end()) {
            componentArrays_[typeName] = std::make_unique<ComponentArray<componentType>>();
            componentArrays_[typeName]->Initialize(entityCapacity_);
        }
    }

    template <IsComponent componentType>
    ComponentArray<componentType>* getComponentArray() {
        std::string typeName = nameof<componentType>();
        if (componentArrays_.find(typeName) == componentArrays_.end()) {
            componentArrays_[typeName] = std::make_unique<ComponentArray<componentType>>();
            componentArrays_[typeName]->Initialize(entityCapacity_);
        }
        return dynamic_cast<ComponentArray<componentType>*>(componentArrays_[typeName].get());
    }

    template <IsComponent componentType>
    std::vector<componentType>* getComponents(GameEntity* _entity) {
        ComponentArray<componentType>* componentArray = getComponentArray<componentType>();
        if (componentArray == nullptr) {
            return nullptr;
        }
        return componentArray->getComponents(_entity);
    }

    template <IsComponent componentType>
    componentType* getComponent(GameEntity* _entity, uint32_t _index) {
        ComponentArray<componentType>* componentArray = getComponentArray<componentType>();
        if (componentArray == nullptr) {
            return nullptr;
        }
        return componentArray->getDynamicComponent(_entity, _index);
    }

    IComponentArray* getComponentArray(const std::string& _typeName) const {
        auto itr = componentArrays_.find(_typeName);
        if (itr == componentArrays_.end()) {
            return nullptr;
        }
        return itr->second.get();
    }
    const std::map<std::string, std::unique_ptr<IComponentArray>>& getComponentArrayMap() const {
        return componentArrays_;
    }

    void FinalizeComponentArrays() {
        for (auto& [componentID, componentArray] : componentArrays_) {
            componentArray->Finalize();
        }
    }

    /// <summary>
    /// 登録したコンポーネント配列を消す
    /// </summary>
    void clearComponentArrays() {
        componentArrays_.clear();
    }

    /// <summary>
    /// 全コンポーネントを削除する(登録された ComponentArrayの種類は保持したまま)
    /// </summary>
    void clearComponents() {
        for (auto& [componentID, componentArray] : componentArrays_) {
            componentArray->clear();
        }
    }

    // --------------------------------------------------------------------------------------
    //  System
    // --------------------------------------------------------------------------------------

    const std::array<std::map<std::string, std::unique_ptr<ISystem>>, int32_t(SystemType::Count)>& getSystems() const {
        return systems_;
    }

    const std::map<std::string, std::unique_ptr<ISystem>>& getSystemsBy(SystemType _systemType) const {
        return systems_[int32_t(_systemType)];
    }

    const std::vector<ISystem*>& getPriorityOrderSystems(SystemType _systemType) const {
        return priorityOrderSystems_[int32_t(_systemType)];
    }
    std::vector<ISystem*>& customPriorityOrderSystems(SystemType _systemType) {
        return priorityOrderSystems_[int32_t(_systemType)];
    }

    template <IsSystem SystemDataType>
    SystemDataType* getSystem() const {
        std::string typeName = nameof<SystemDataType>();
        // 各 SystemType 毎のマップから型名で検索
        for (const auto& systemMap : systems_) {
            auto itr = systemMap.find(typeName);
            if (itr != systemMap.end()) {
                return static_cast<SystemDataType*>(itr->second.get());
            }
        }
        return nullptr;
    }

    template <IsSystem SystemDataType, typename... Args>
    void registerSystem(Args... _args) {

        // システムの名前を取得
        std::string typeName = nameof<SystemDataType>();

        // 登録する インスタンスを作成
        std::unique_ptr<SystemDataType> system = std::make_unique<SystemDataType>(_args...);
        system->Initialize();

        // システムのタイプを取得
        SystemType systemType   = system->getSystemType();
        int32_t systemTypeIndex = int32_t(systemType);

        // システムを登録
        if (systems_[systemTypeIndex].find(typeName) == systems_[int32_t(systemType)].end()) {
            systems_[systemTypeIndex][typeName] = std::move(system);
            priorityOrderSystems_[systemTypeIndex].push_back(systems_[systemTypeIndex][typeName].get());
            return;
        }
    }

    void SortPriorityOrderSystems() {
        for (int32_t systemTypeIndex = 0; systemTypeIndex < int32_t(SystemType::Count); ++systemTypeIndex) {
            // sort
            if (priorityOrderSystems_[systemTypeIndex].size() < 2) {
                continue;
            }
            std::sort(
                priorityOrderSystems_[systemTypeIndex].begin(),
                priorityOrderSystems_[systemTypeIndex].end(),
                [](ISystem* a, ISystem* b) { return a->getPriority() < b->getPriority(); });
        }
    }
    void SortPriorityOrderSystems(int32_t _systemTypeIndex) {
        if (priorityOrderSystems_[_systemTypeIndex].size() < 1) {
            return;
        }
        std::sort(
            priorityOrderSystems_[_systemTypeIndex].begin(),
            priorityOrderSystems_[int32_t(_systemTypeIndex)].end(),
            [](ISystem* a, ISystem* b) { return a->getPriority() < b->getPriority(); });
    }

    void FinalizeSystems() {
        for (auto& systemMap : systems_) {
            for (auto& [systemID, system] : systemMap) {
                system->Finalize();
            }
        }
    }

    void clearSystem() {
        for (auto& systemMap : systems_) {
            systemMap.clear();
        }
        for (auto& prioritySystems : priorityOrderSystems_) {
            prioritySystems.clear();
        }
    }
};

using ECSManager = EntityComponentSystemManager;

template <IsComponent ComponentType>
inline ComponentType* getComponent(GameEntity* _entity, int32_t _index = 0) {
    return ECSManager::getInstance()->getComponent<ComponentType>(_entity, _index);
}

template <IsComponent ComponentType>
inline std::vector<ComponentType>* getComponents(GameEntity* _entity) {
    return ECSManager::getInstance()->getComponents<ComponentType>(_entity);
}

template <IsComponent... ComponentArgs>
inline GameEntity* CreateEntity(const std::string& _dataType, ComponentArgs... _args) {
    uint32_t entityIndex = ECSManager::getInstance()->registerEntity(_dataType);
    GameEntity* entity   = ECSManager::getInstance()->getEntity(entityIndex);

    (ECSManager::getInstance()->template addComponent<ComponentArgs>(entityIndex, _args), ...);

    // コンポーネントの初期化
    ([&entity]<typename T>() {
        auto components = ECSManager::getInstance()->template getComponents<T>(entity);
        if (components) {
            for (auto& component : *components) {
                component.Initialize(entity);
            }
        }
    }.template operator()<ComponentArgs>(),
        ...);

    return entity;
}

inline GameEntity* getUniqueEntity(const std::string& _dataTypeName) {
    return ECSManager::getInstance()->getUniqueEntity(_dataTypeName);
}

inline bool registerUniqueEntity(const std::string& _dataTypeName, GameEntity* _entity) {
    return ECSManager::getInstance()->registerUniqueEntity(_dataTypeName, _entity);
}

inline void removeUniqueEntity(const std::string& _dataTypeName) {
    ECSManager::getInstance()->removeUniqueEntity(_dataTypeName);
}

void DestroyEntity(GameEntity* _entity);
