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

/// math
#include <stdint.h>

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

    void RunInitialize();
    void ExecuteEntitiesDelete();

private:
    EntityComponentSystemManager() {}
    ~EntityComponentSystemManager() {}
    EntityComponentSystemManager(const EntityComponentSystemManager&)            = delete;
    EntityComponentSystemManager* operator=(const EntityComponentSystemManager&) = delete;

private:
    /// <summary>
    /// エンティティ配列
    /// </summary>
    std::vector<GameEntity> entityes_;
    std::vector<uint32_t> freeEntityIndex_;
    std::map<std::string, uint32_t> uniqueentityIDs_;
    uint32_t entityCapacity_ = 10000;

    std::queue<uint32_t> deleteEntityQueue_;

    /// <summary>
    /// コンポーネント配列
    /// </summary>
    std::map<std::string, std::unique_ptr<IComponentArray>> componentArrays_;

    /// <summary>
    /// システム配列
    /// </summary>
    std::array<std::map<std::string, std::unique_ptr<ISystem>>, int32_t(SystemType::Count)> systems_;

    std::array<std::vector<ISystem*>, int32_t(SystemType::Count)> workSystems_;

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

    int32_t getActiveEntityCount() const {
        return static_cast<int32_t>(entityes_.size() - freeEntityIndex_.size());
    }

    /// <summary>
    /// エンティティを作成する
    /// </summary>
    uint32_t registerEntity(const std::string& _entityDataType);

    /// <summary>
    /// エンティティを取得する
    /// </summary>
    GameEntity* getEntity(int32_t _entityIndex) {
        return &entityes_[_entityIndex];
    }

    GameEntity* getUniqueEntity(const std::string& _dataTypeName) {
        auto itr = uniqueentityIDs_.find(_dataTypeName);
        if (itr == uniqueentityIDs_.end()) {
            return nullptr;
        }
        return &entityes_[itr->second];
    }
    bool registerUniqueEntity(GameEntity* _entity) {
        if (uniqueentityIDs_.find(_entity->dataType_) != uniqueentityIDs_.end()) {
            return false;
        }

        _entity->isUnique_ = true;

        uniqueentityIDs_[_entity->dataType_] = _entity->getID();
        return true;
    }
    void removeUniqueEntity(const std::string& _dataTypeName) {
        auto itr = uniqueentityIDs_.find(_dataTypeName);
        if (itr == uniqueentityIDs_.end()) {
            return;
        }

        GameEntity& uniqueEntity = entityes_[uniqueentityIDs_[_dataTypeName]];
        uniqueEntity.isUnique_   = false;
        uniqueentityIDs_.erase(_dataTypeName);
    }

    const std::vector<GameEntity>& getEntities() const {
        return entityes_;
    }

    /// <summary>
    /// エンティティにコンポーネントを追加する
    /// </summary>
    template <IsComponent componentType>
    void addComponent(uint32_t _entityIndex, const componentType& _component, bool _doInitialize = true) {
        ComponentArray<componentType>* componentArray = getComponentArray<componentType>();
        componentArray->add(&entityes_[_entityIndex], _component, _doInitialize);
    }

    /// <summary>
    /// エンティティを削除する
    /// </summary>
    void destroyEntity(GameEntity* _entityIndex) {
        deleteEntityQueue_.push(_entityIndex->getID());
    }

    void clearEntities() {
        entityes_.clear();
        freeEntityIndex_.clear();
    }

    void clearUniqueEntities() {
        for (auto itr = uniqueentityIDs_.begin(); itr != uniqueentityIDs_.end();) {
            GameEntity& uniqueEntity = entityes_[itr->second];
            uniqueEntity.isUnique_   = false;
            itr                      = uniqueentityIDs_.erase(itr);
        }
    }

    /// <summary>
    /// 生きているエンティティを削除する
    /// </summary>
    void clearAliveEntities() {
        for (auto& entity : entityes_) {
            if (!entity.isAlive_) {
                continue;
            }
            destroyEntity(&entity);
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
    template <IsSystem SystemDataType>
    SystemDataType* getSystem(SystemType _systemType) const {
        std::string typeName = nameof<SystemDataType>();
        auto itr             = systems_[int32_t(_systemType)].find(typeName);
        if (itr != systems_[int32_t(_systemType)].end()) {
            return static_cast<SystemDataType*>(itr->second.get());
        }
        return nullptr;
    }

    ISystem* getSystem(const std::string& _name) const {
        for (const auto& systemMap : systems_) {
            auto itr = systemMap.find(_name);
            if (itr != systemMap.end()) {
                return itr->second.get();
            }
        }
        return nullptr;
    }
    ISystem* getSystem(SystemType _systemType, const std::string& _name) const {
        auto itr = systems_[int32_t(_systemType)].find(_name);
        if (itr != systems_[int32_t(_systemType)].end()) {
            return itr->second.get();
        }
        return nullptr;
    }

    const std::vector<ISystem*>& getWorkSystems(SystemType _systemType) const {
        return workSystems_[int32_t(_systemType)];
    }
    std::vector<ISystem*>& customWorkSystems(SystemType _systemType) {
        return workSystems_[int32_t(_systemType)];
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
            return;
        }
    }
    template <IsSystem SystemDataType>
    bool unregisterSystem() {
        std::string typeName = nameof<SystemDataType>();
        for (int32_t i = 0; i < int32_t(SystemType::Count); ++i) {
            auto itr = systems_[i].find(typeName);
            if (itr != systems_[i].end()) {
                // システムを削除
                workSystems_[i].erase(std::remove(workSystems_[i].begin(), workSystems_[i].end(), itr->second.get()), workSystems_[i].end());
                systems_[i].erase(itr);
                return true;
            }
        }
        // システムが見つからない場合は、falseを返す
        return false;
    }
    template <IsSystem SystemDataType>
    bool unregisterSystem(SystemType _type) {
        std::string typeName    = nameof<SystemDataType>();
        int32_t systemTypeIndex = int32_t(_type);
        auto itr                = systems_[systemTypeIndex].find(typeName);
        if (itr == systems_[systemTypeIndex].end()) {
            // システムが見つからない場合は、falseを返す
            return false;
        }
        // システムを削除
        workSystems_[systemTypeIndex].erase(std::remove(workSystems_[systemTypeIndex].begin(), workSystems_[systemTypeIndex].end(), itr->second.get()), workSystems_[systemTypeIndex].end());
        systems_[systemTypeIndex].erase(itr);
        return true;
    }

    template <IsSystem SystemDataType>
    bool ActivateSystem(bool _doInit = true) {
        std::string typeName = nameof<SystemDataType>();
        for (int32_t i = 0; i < int32_t(SystemType::Count); ++i) {
            auto itr = systems_[i].find(typeName);

            if (itr != systems_[i].end()) {
                auto workkingSystem = itr->second.get();
                workkingSystem->setIsActive(true);

                workSystems_[i].push_back(workkingSystem);
                if (_doInit) {
                    workSystems_[i].back()->Initialize();
                }
                std::sort(workSystems_[i].begin(), workSystems_[i].end(),
                    [](ISystem* a, ISystem* b) {
                        return a->getPriority() < b->getPriority();
                    });

                return true;
            }
        }

        // システムが見つからない場合は、falseを返す
        return false;
    }
    template <IsSystem SystemDataType>
    bool ActivateSystem(SystemType _type, bool _doInit = true) {
        std::string typeName    = nameof<SystemDataType>();
        int32_t systemTypeIndex = int32_t(_type);
        auto itr                = systems_[systemTypeIndex].find(typeName);

        if (itr == systems_[systemTypeIndex].end()) {
            // システムが見つからない場合は、falseを返す
            return false;
        }

        auto workkingSystem = itr->second.get();
        workkingSystem->setIsActive(true);

        workSystems_[systemTypeIndex].push_back(workkingSystem);
        if (_doInit) {
            workSystems_[systemTypeIndex].back()->Initialize();
        }
        std::sort(workSystems_[systemTypeIndex].begin(), workSystems_[systemTypeIndex].end(),
            [](ISystem* a, ISystem* b) {
                return a->getPriority() < b->getPriority();
            });

        return true;
    }
    bool ActivateSystem(const std::string& _name, bool _doInit = true);
    bool ActivateSystem(const std::string& _name, SystemType _type, bool _doInit = true);

    void AllActivateSystem(bool _doInit = true);

    template <IsSystem SystemDataType>
    bool StopSystem() {
        std::string typeName = nameof<SystemDataType>();
        for (int32_t i = 0; i < int32_t(SystemType::Count); ++i) {
            auto itr = systems_[i].find(typeName);
            if (itr != systems_[i].end()) {
                if (itr->second->isActive()) {
                    itr->second->setIsActive(false);

                    auto& systemsVector = workSystems_[i];
                    systemsVector.erase(std::remove(systemsVector.begin(), systemsVector.end(), itr->second.get()), systemsVector.end());
                    return true;
                } else {
                    return false;
                }
            }
        }
        // システムが見つからない場合は、falseを返す
        return false;
    }
    template <IsSystem SystemDataType>
    bool StopSystem(SystemType _type) {
        std::string typeName    = nameof<SystemDataType>();
        int32_t systemTypeIndex = int32_t(_type);
        auto itr                = systems_[systemTypeIndex].find(typeName);
        if (itr == systems_[systemTypeIndex].end()) {
            // システムが見つからない場合は、falseを返す
            return false;
        }
        if (itr->second->isActive()) {
            itr->second->setIsActive(false);

            auto& systemsVector = workSystems_[systemTypeIndex];
            systemsVector.erase(std::remove(
                                    systemsVector.begin(),
                                    systemsVector.end(),
                                    itr->second.get()),
                systemsVector.end());

            return true;
        } else {
            return false;
        }
    }
    bool StopSystem(const std::string& _name) {
        for (int32_t i = 0; i < int32_t(SystemType::Count); ++i) {
            auto itr = systems_[i].find(_name);
            if (itr != systems_[i].end()) {
                if (itr->second->isActive()) {
                    itr->second->setIsActive(false);
                    auto& systemsVector = workSystems_[i];
                    systemsVector.erase(std::remove(systemsVector.begin(), systemsVector.end(), itr->second.get()), systemsVector.end());
                    return true;
                } else {
                    return false;
                }
            }
        }
        // システムが見つからない場合は、falseを返す
        return false;
    }

    bool StopSystem(const std::string& _name, SystemType _type) {
        int32_t systemTypeIndex = int32_t(_type);
        auto itr                = systems_[systemTypeIndex].find(_name);
        if (itr == systems_[systemTypeIndex].end()) {
            // システムが見つからない場合は、falseを返す
            return false;
        }
        if (itr->second->isActive()) {
            itr->second->setIsActive(false);
            auto& systemsVector = workSystems_[systemTypeIndex];
            systemsVector.erase(std::remove(systemsVector.begin(), systemsVector.end(), itr->second.get()), systemsVector.end());

            return true;
        } else {
            return false;
        }
    }

    void SortPriorityOrderSystems() {
        for (int32_t systemTypeIndex = 0; systemTypeIndex < int32_t(SystemType::Count); ++systemTypeIndex) {
            // sort
            if (workSystems_[systemTypeIndex].size() < 2) {
                continue;
            }
            std::sort(
                workSystems_[systemTypeIndex].begin(),
                workSystems_[systemTypeIndex].end(),
                [](ISystem* a, ISystem* b) { return a->getPriority() < b->getPriority(); });
        }
    }
    void SortPriorityOrderSystems(int32_t _systemTypeIndex) {
        if (workSystems_[_systemTypeIndex].size() < 1) {
            return;
        }
        std::sort(
            workSystems_[_systemTypeIndex].begin(),
            workSystems_[int32_t(_systemTypeIndex)].end(),
            [](ISystem* a, ISystem* b) { return a->getPriority() < b->getPriority(); });
    }

    void InitializeSystems() {
        for (auto& systemMap : systems_) {
            for (auto& [systemID, system] : systemMap) {
                system->Initialize();
            }
        }
    }
    void InitializeWorkSystems() {
        for (auto& systemMap : workSystems_) {
            for (auto& system : systemMap) {
                system->Initialize();
            }
        }
    }

    void FinalizeSystems() {
        for (auto& systemMap : systems_) {
            for (auto& [systemID, system] : systemMap) {
                system->Finalize();
            }
        }
    }
    void FinalizeWorkSystems() {
        for (auto& systemMap : workSystems_) {
            for (auto& system : systemMap) {
                system->Finalize();
            }
        }
    }

    void clearAllSystems() {
        for (auto& prioritySystems : workSystems_) {
            prioritySystems.clear();
        }
        for (auto& systemMap : systems_) {
            systemMap.clear();
        }
    }
    void clearWorkSystems() {
        for (auto& prioritySystems : workSystems_) {
            prioritySystems.clear();
        }
    }
};

using ECSManager = EntityComponentSystemManager;

GameEntity* getEntity(int32_t _entityIndex);

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

    (ECSManager::getInstance()->template addComponent<ComponentArgs>(entityIndex, _args, false), ...);

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

inline bool registerUniqueEntity(GameEntity* _entity) {
    return ECSManager::getInstance()->registerUniqueEntity(_entity);
}

inline void removeUniqueEntity(const std::string& _dataTypeName) {
    ECSManager::getInstance()->removeUniqueEntity(_dataTypeName);
}

nlohmann::json EntityToJson(GameEntity* _entity);
GameEntity* EntityFromJson(const nlohmann::json& _jsonData);

void DestroyEntity(GameEntity* _entity);
