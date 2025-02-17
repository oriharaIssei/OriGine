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
#include "util/TypeName.h"

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
    std::vector<std::unique_ptr<GameEntity>> entities_;
    std::vector<uint32_t> freeEntityIndex_;

    uint32_t entityCapacity_ = 100;

    /// <summary>
    /// コンポーネント配列
    /// </summary>
    std::map<std::string, std::unique_ptr<IComponentArray>> componentArrays_;

    /// <summary>
    /// システム配列
    /// </summary>
    std::vector<std::unique_ptr<ISystem>> systems_;

public:
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
    uint32_t createEntity(const std::string& _entityDataType) {
        if (freeEntityIndex_.empty()) {
            return -1;
        }
        uint32_t index = freeEntityIndex_.back();
        freeEntityIndex_.pop_back();
        return index;
    }

    /// <summary>
    /// エンティティを削除する
    /// </summary>
    void destroyEntity(uint32_t _entityIndex) {
        //! TODO : エンティティの削除処理 (コンポーネントの削除処理も必要)
        entities_[_entityIndex].reset();
        freeEntityIndex_.push_back(_entityIndex);
    }

    template <IsComponent componentType>
    ComponentArray<componentType>* getComponentArray() {
        std::string typeName = getTypeName<componentType>();
        if (componentArrays_.find(typeName) == componentArrays_.end()) {
            return nullptr;
        }
        return static_cast<ComponentArray<componentType>*>(componentArrays_[typeName]);
    }
};
