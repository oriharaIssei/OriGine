#pragma once

/// stl
#include <memory>
#include <string>
// container
#include <map>

/// engine
// ECS
#include "ECS/ComponentArray.h"
#include "ECS/system/ISystem.h"
#include "Entity.h"
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
    std::map<std::string, std::unique_ptr<ISystem>> systems_;

    /// <summary>
    /// エンティティのデータタイプ
    /// <string, byDataTypeIndex>
    /// </summary>
    std::map<std::string, uint32_t> entityDataTypes_;
    /// <summary>
    /// データタイプごとに所持しているコンポーネントの種類と数を保持
    /// </summary>
    std::vector<std::vector<std::pair<std::string, int32_t>>> componentTypeListByDataType_;
    /// <summary>
    /// データタイプごとに所属しているシステムのタイプを保持
    /// </summary>
    std::vector<std::vector<std::string>> systemTypeListByDataType_;

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
                componentArray->resize(entityCapacity_);
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

    /// <summary>
    /// エンティティにコンポーネントを追加する
    /// </summary>
    template <IsComponent componentType>
    void addComponent(uint32_t _entityIndex, componentType&& _component) {
        ComponentArray<componentType>* componentArray = getComponentArray<componentType>();
        componentArray->addComponent(_entityIndex, std::forward<componentType>(_component));
    }

    /// <summary>
    /// エンティティを削除する
    /// </summary>
    void destroyEntity(uint32_t _entityIndex) {
        //! TODO : エンティティの削除処理 (コンポーネントの削除処理も必要)
        freeEntityIndex_.push_back(_entityIndex);
    }

    // --------------------------------------------------------------------------------------
    //  Component
    // --------------------------------------------------------------------------------------
    template <IsComponent componentType>
    ComponentArray<componentType>* getComponentArray() {
        std::string typeName = nameof<componentType>();
        if (componentArrays_.find(typeName) == componentArrays_.end()) {
            return nullptr;
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
        return componentArray->getComponent(_entity, _index);
    }

    IComponentArray* getComponentArray(const std::string& _typeName) const {
        auto itr = componentArrays_.find(_typeName);
        if (itr == componentArrays_.end()) {
            return nullptr;
        }
        return itr->second.get();
    }
    // --------------------------------------------------------------------------------------
    //  System
    // --------------------------------------------------------------------------------------
    template <IsSystem SystemType>
    SystemType* getSystem() const {
        std::string typeName = nameof<SystemType>();

        auto itr = systems_.find(typeName);
        if (itr == systems_.end()) {
            return nullptr;
        }
        return static_cast<SystemType*>(itr->second.get());
    }
    ISystem* getSystem(const std::string& _typeName) const {
        auto itr = systems_.find(_typeName);
        if (itr == systems_.end()) {
            return nullptr;
        }
        return itr->second.get();
    }

    // --------------------------------------------------------------------------------------
    // ForDataType
    // --------------------------------------------------------------------------------------
    void registerNewEntityDataType(const std::string& dataType) {
        uint32_t dataTypeIndex     = uint32_t(entityDataTypes_.size());
        entityDataTypes_[dataType] = dataTypeIndex;
        componentTypeListByDataType_.resize(dataTypeIndex);
        systemTypeListByDataType_.resize(dataTypeIndex);
    }
    void removeEntityDataType(const std::string& dataType) {
        uint32_t dataTypeIndex = entityDataTypes_[dataType];
        entityDataTypes_.erase(dataType);
        componentTypeListByDataType_.erase(componentTypeListByDataType_.begin() + dataTypeIndex);
        systemTypeListByDataType_.erase(systemTypeListByDataType_.begin() + dataTypeIndex);
    }

    /// -------------------------- ComponentArrayByDataType -------------------------- ///
    template <IsComponent componentType>
    void registerComponentTypeByDataType(const std::string& _dataType) {
        std::string typeName                                            = nameof<componentType>();
        std::vector<std::pair<std::string, int32_t>>& componentTypeList = componentTypeListByDataType_[entityDataTypes_[_dataType]];

        // すでに登録されているか確認
        for (auto& [registeredComponentTypeName, componentSize] : componentTypeList) {
            // 登録されている場合 登録されている数情報を増やす
            if (registeredComponentTypeName == typeName) {
                componentSize++;
                return;
            }
        }
        componentTypeList.push_back({typeName, 1});
    }
    void removeComponentTypeByDataType(const std::string& _dataType, const std::string& _componentType) {
        std::vector<std::pair<std::string, int32_t>>& componentTypeList = componentTypeListByDataType_[entityDataTypes_[_dataType]];

        // すでに登録されているか確認
        for (auto itr = componentTypeList.begin(); itr != componentTypeList.end(); ++itr) {
            // 登録されている場合
            if (itr->first == _componentType) {
                // 登録されている数情報を減らす
                itr->second--;
                if (itr->second == 0) {
                    // 0になった場合は削除
                    componentTypeList.erase(itr);
                }
                return;
            }
        }
    }
    const std::vector<std::pair<std::string, int32_t>>& getComponentTypeByDataType(const std::string& dataType) const {
        static const std::vector<std::pair<std::string, int32_t>> empty;
        uint32_t dataTypeIndex = entityDataTypes_.at(dataType);
        return (dataTypeIndex < componentTypeListByDataType_.size()) ? componentTypeListByDataType_[dataTypeIndex] : empty;
    }

    /// -------------------------- SystemByDataType -------------------------- ///
    template <IsSystem systemType>
    void registerSystemByDataType(const std::string& dataType, systemType* system) {
        systemTypeListByDataType_[entityDataTypes_[dataType]].push_back(nameof<systemType>());
    }
    void removeSystem(const std::string& dataType, const std::string& systemType) {
        std::vector<std::string>& systemList = systemTypeListByDataType_[entityDataTypes_[dataType]];
        systemList.erase(std::remove(systemList.begin(), systemList.end(), systemType), systemList.end());
    }

    const std::vector<std::string>& getSystemsByDataType(const std::string& dataType) const {
        static const std::vector<std::string> empty;
        uint32_t dataTypeIndex = entityDataTypes_.at(dataType);
        return (dataTypeIndex < systemTypeListByDataType_.size()) ? systemTypeListByDataType_[dataTypeIndex] : empty;
    }
};

using ECSManager = EntityComponentSystemManager;

template <IsComponent ComponentType>
ComponentType* getComponent(GameEntity* _entity, int32_t _index = 0) {
    return ECSManager::getInstance()->getComponent<ComponentType>(_entity, _index);
}
