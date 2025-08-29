#pragma once

/// stl
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

/// engine
// ECS
class EntityRepository;

#include "logger/Logger.h"

// util
#include "util/BitArray.h"
#include "util/nameof.h"

/// math
#include <stdint.h>

/// <summary>
/// 実体を表すクラス (実際にはIDでしか無い)
/// </summary>
class GameEntity {
    friend class EntityRepository;

public:
    GameEntity(const std::string& _dataType, int32_t _id, bool _isUnique = false)
        : dataType_(_dataType), id_(_id), isAlive_(true), isUnique_(_isUnique) {}
    GameEntity() = default;

    ~GameEntity() = default;

    operator int32_t() const {
        return id_;
    }

private:
    std::string dataType_ = "UNKNOWN";

    int32_t id_      = -1;
    bool isAlive_    = false;
    bool isUnique_   = false;
    bool shouldSave_ = true;

public:
    bool isAlive() const {
        return isAlive_;
    }
    bool isUnique() const {
        return isUnique_;
    }

    bool shouldSave() const {
        return shouldSave_;
    }
    void setShouldSave(bool _shouldSave) {
        shouldSave_ = _shouldSave;
    }

    void deleteEntity() {
        isAlive_ = false;
    }

    /// <summary>
    /// エンティティのIDを取得する
    /// </summary>
    int32_t getID() const {
        return id_;
    }
    /// <summary>
    /// エンティティのデータタイプを取得する
    /// </summary>
    const std::string& getDataType() const {
        return dataType_;
    }

    /// <summary>
    /// エンティティのデータタイプを設定する
    /// </summary>
    void setDataType(const std::string& _dataType) {
        dataType_ = _dataType;
    }

    /// <summary>
    /// エンティティのユニークIDを取得する
    /// </summary>
    std::string getUniqueID() const {
        return dataType_ + std::to_string(id_);
    }
};

/// <summary>
/// Entity Repository(登録, 削除, 取得などを行う)
/// </summary>
class EntityRepository final {
public:
    EntityRepository() {}
    ~EntityRepository() = default;

    void Initialize();
    void Finalize();

    void resize(uint32_t _newSize) {
        size_ = _newSize;
        entities_.resize(size_);
        entityActiveBits_.resize(size_);
    }

    /// <summary>
    /// Entity の取得
    /// </summary>
    GameEntity* getEntity(int32_t _entityIndex) {
        if (_entityIndex < 0 || static_cast<uint32_t>(_entityIndex) >= size_) {
            LOG_ERROR("EntityRepository: Invalid entity index.");
            return nullptr;
        }
        return &entities_[_entityIndex];
    }
    /// <summary>
    /// UniqueEntity の取得
    /// </summary>
    GameEntity* getUniqueEntity(const std::string& _dataTypeName) {
        auto itr = uniqueEntityIDs_.find(_dataTypeName);

        if (itr == uniqueEntityIDs_.end()) {
            LOG_ERROR("EntityRepository: Unique entity not found with data type: {}", _dataTypeName);
            return nullptr;
        }

        return &entities_[itr->second];
    }

    /// <summary>
    /// EntityIndex の 確保
    /// </summary>
    int32_t allocateEntity() {
        if (entityActiveBits_.getTrueCount() >= size_) {
            LOG_INFO("EntityRepository: Allocating more entities than current size. Resizing...");
            resize(size_ * 2);
        }

        int32_t entityIndex = static_cast<int32_t>(entityActiveBits_.allocateBit());
        return entityIndex;
    }

    /// <summary>
    /// UniqueEntity に 登録する
    /// </summary>
    bool registerUniqueEntity(GameEntity* _entity) {
        if (uniqueEntityIDs_.find(_entity->dataType_) != uniqueEntityIDs_.end()) {
            LOG_ERROR("EntityRepository: Unique entity already registered with data type: {}", _entity->dataType_);
            return false;
        }
        _entity->isUnique_                   = true;
        uniqueEntityIDs_[_entity->dataType_] = _entity->getID();
        return true;
    }
    /// <summary>
    /// UniqueEntity を削除する
    /// </summary>
    bool unregisterUniqueEntity(const std::string& _dataTypeName) {
        auto itr = uniqueEntityIDs_.find(_dataTypeName);
        if (itr == uniqueEntityIDs_.end()) {
            LOG_ERROR("EntityRepository: Unique entity not found with data type: {}", _dataTypeName);
            return false;
        }
        GameEntity& uniqueEntity = entities_[itr->second];
        uniqueEntity.isUnique_   = false;
        uniqueEntityIDs_.erase(itr);
        return true;
    }

    /// <summary>
    /// Entity を登録する
    /// </summary>
    int32_t CreateEntity(const std::string& _dataType, bool _isUnique = false) {
        int32_t entityIndex = allocateEntity();

        GameEntity& entity = entities_[entityIndex];
        entity.id_         = entityIndex;
        entity.dataType_   = _dataType;
        entity.isAlive_    = true;
        entity.isUnique_   = false;

        if (_isUnique) {
            registerUniqueEntity(&entity);
        }

        return entityIndex;
    }
    /// <summary>
    /// Entity を指定したIndexに登録する
    /// </summary>
    int32_t CreateEntity(int32_t _id, const std::string& _dataType, bool _isUnique = false) {
        if (_id < 0 || static_cast<uint32_t>(_id) >= size_) {
            LOG_ERROR("EntityRepository: Invalid entity index allocated.");
            return -1;
        }

        GameEntity& entity = entities_[_id];
        if (entity.isAlive_) {
            LOG_ERROR("EntityRepository: Entity with ID {} already exists.", _id);
            return -1;
        }

        entity.id_                  = _id;
        entity.dataType_            = _dataType;
        entity.isAlive_             = true;
        entity.isUnique_            = false;
        uniqueEntityIDs_[_dataType] = _id;

        if (_isUnique) {
            registerUniqueEntity(&entity);
        }

        return _id;
    }

    /// <summary>
    /// Entity を削除する
    /// </summary>
    bool removeEntity(int32_t _entityIndex) {
        if (_entityIndex < 0 || static_cast<uint32_t>(_entityIndex) >= size_) {
            LOG_ERROR("EntityRepository: Invalid entity index to unregister.");
            return false;
        }

        GameEntity& entity = entities_[_entityIndex];

        if (!entity.isAlive_) {
            LOG_ERROR("EntityRepository: Entity already dead.");
            return false;
        }

        if (entity.isUnique_) {
            unregisterUniqueEntity(entity.dataType_);
        }

        entity = GameEntity();

        entityActiveBits_.set(_entityIndex, false);

        return true;
    }

private:
    uint32_t size_ = 10000;
    std::vector<GameEntity> entities_;
    std::unordered_map<std::string, int32_t> uniqueEntityIDs_;
    BitArray<uint64_t> entityActiveBits_;

public:
    uint32_t getSize() const {
        return size_;
    }
    uint32_t getActiveEntityCount() const {
        return static_cast<uint32_t>(entityActiveBits_.getTrueCount());
    }
    uint32_t getInactiveEntityCount() const {
        return static_cast<uint32_t>(entityActiveBits_.getFalseCount());
    }
    void clear() {
        size_ = 0;
        entities_.clear();
        entityActiveBits_.resize(0);
    }

    const std::vector<GameEntity>& getEntities() const {
        return entities_;
    }
    std::vector<GameEntity>& getEntitiesRef() {
        return entities_;
    }
};
