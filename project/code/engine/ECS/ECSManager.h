#pragma once

/// stl
#include <memory>
#include <string>
// container
#include <array>
#include <map>

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

    void Init();
    void Run();
    void Finalize();

    void ComponentArraysInit();

private:
    EntityComponentSystemManager() {}
    ~EntityComponentSystemManager() {}
    EntityComponentSystemManager(const EntityComponentSystemManager&)            = delete;
    EntityComponentSystemManager* operator=(const EntityComponentSystemManager&) = delete;

private:
    std::vector<GameEntity> entities_;
    std::vector<uint32_t> freeEntityIndex_;

    uint32_t entityCapacity_ = 100;

    /// <summary>
    /// コンポーネント配列
    /// </summary>
    std::map<std::string, std::unique_ptr<IComponentArray>> componentArrays_;

    /// <summary>
    /// システム配列
    /// </summary>
    std::array<std::map<std::string, std::unique_ptr<ISystem>>, int32_t(SystemType::Count)> systems_;

public: // ============== accessor ==============//
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
            //  リサイズ前のサイズを保存
            size_t oldSize = entities_.size();
            // entities_ を倍のサイズにリサイズ
            entities_.resize(oldSize * 2);
            // 新しく追加されたインデックスを freeEntityIndex_ に登録
            for (uint32_t i = static_cast<uint32_t>(oldSize); i < entities_.size(); i++) {
                freeEntityIndex_.push_back(i);
            }
            entityCapacity_ = uint32_t(entities_.size());

            for (auto& [componentID, componentArray] : componentArrays_) {
                componentArray->Init(entityCapacity_);
            }
        }
        uint32_t index = freeEntityIndex_.back();
        freeEntityIndex_.pop_back();

        entities_[index] = GameEntity(_entityDataType, index);

        return index;
    }

    /// <summary>
    /// エンティティを取得する
    /// </summary>
    GameEntity* getEntity(uint32_t _entityIndex) {
        return &entities_[_entityIndex];
    }

    const std::vector<GameEntity>& getEntities() const {
        return entities_;
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
    void destroyEntity(uint32_t _entityIndex) {
        freeEntityIndex_.push_back(_entityIndex);
        entities_[_entityIndex] = GameEntity("UNKNOWN", -1);
    }

    void clearEntity() {
        entities_.clear();
        freeEntityIndex_.clear();
    }

    // --------------------------------------------------------------------------------------
    //  Component
    // --------------------------------------------------------------------------------------
    template <IsComponent componentType>
    void registerComponent() {
        std::string typeName = nameof<componentType>();
        if (componentArrays_.find(typeName) == componentArrays_.end()) {
            componentArrays_[typeName] = std::make_unique<ComponentArray<componentType>>();
            componentArrays_[typeName]->Init(entityCapacity_);
        }
    }

    template <IsComponent componentType>
    ComponentArray<componentType>* getComponentArray() {
        std::string typeName = nameof<componentType>();
        if (componentArrays_.find(typeName) == componentArrays_.end()) {
            componentArrays_[typeName] = std::make_unique<ComponentArray<componentType>>();
            componentArrays_[typeName]->Init(entityCapacity_);
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

    template <IsSystem SystemDataType, typename... Args>
    void registerSystem(Args... _args) {
        // 登録する インスタンスを作成
        std::unique_ptr<SystemDataType> system = std::make_unique<SystemDataType>(_args...);
        system->Init();

        // システムのデータタイプを取得
        std::string typeName = nameof<SystemDataType>();

        // システムのタイプを取得
        SystemType systemType = system->getSystemType();

        // システムを登録
        if (systems_[int32_t(systemType)].find(typeName) == systems_[int32_t(systemType)].end()) {
            systems_[int32_t(systemType)][typeName] = std::move(system);
            return;
        }

        // すでに登録されている場合はエラーを出力
        assert(false && "System already registered");
    }

    void clearSystem() {
        for (auto& systemMap : systems_) {
            systemMap.clear();
        }
    }
};

using ECSManager = EntityComponentSystemManager;

template <IsComponent ComponentType>
ComponentType* getComponent(GameEntity* _entity, int32_t _index = 0) {
    return ECSManager::getInstance()->getComponent<ComponentType>(_entity, _index);
}

template <IsComponent... ComponentArgs>
GameEntity* CreateEntity(const std::string& _dataType, ComponentArgs... _args) {
    uint32_t entityIndex = ECSManager::getInstance()->registerEntity(_dataType);
    (ECSManager::getInstance()->template addComponent<ComponentArgs>(entityIndex, _args), ...);
    return ECSManager::getInstance()->getEntity(entityIndex);
}
