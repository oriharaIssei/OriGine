#include "EntityRepository.h"

#include "entity/Entity.h"

/// externals
#include "logger/Logger.h"

using namespace OriGine;

EntityRepository::EntityRepository() {}
EntityRepository::~EntityRepository() = default;

void EntityRepository::Initialize() {
    entities_.resize(size_);
    entityActiveBits_.resize(size_);
    if (!uniqueEntityIDs_.empty()) {
        uniqueEntityIDs_.clear();
    }
}

void EntityRepository::Finalize() {
    entities_.clear();
    entityActiveBits_.resize(0);
    uniqueEntityIDs_.clear();
}

void EntityRepository::resize(uint32_t _newSize) {
    size_ = _newSize;
    entities_.resize(size_);
    entityActiveBits_.resize(size_);
}

/// <summary>
/// Entity の取得
/// </summary>
Entity* EntityRepository::GetEntity(int32_t _entityIndex) {
    if (_entityIndex < 0 || static_cast<uint32_t>(_entityIndex) >= size_) {
        LOG_ERROR("EntityRepository: Invalid entity index.");
        return nullptr;
    }
    return &entities_[_entityIndex];
}
/// <summary>
/// UniqueEntity の取得
/// </summary>
Entity* EntityRepository::GetUniqueEntity(const std::string& _dataTypeName) {
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
int32_t EntityRepository::AllocateEntity() {
    if (entityActiveBits_.GetTrueCount() >= size_) {
        LOG_INFO("EntityRepository: Allocating more entities than current size. Resizing...");
        resize(size_ * 2);
    }

    int32_t entityIndex = static_cast<int32_t>(entityActiveBits_.allocateBit());
    return entityIndex;
}

/// <summary>
/// UniqueEntity に 登録する
/// </summary>
bool EntityRepository::RegisterUniqueEntity(Entity* _entity) {
    if (uniqueEntityIDs_.find(_entity->dataType_) != uniqueEntityIDs_.end()) {
        LOG_ERROR("EntityRepository: Unique entity already registered with data type: {}", _entity->dataType_);
        return false;
    }
    _entity->isUnique_                   = true;
    uniqueEntityIDs_[_entity->dataType_] = _entity->GetID();
    return true;
}
/// <summary>
/// UniqueEntity を削除する
/// </summary>
bool EntityRepository::UnregisterUniqueEntity(const std::string& _dataTypeName) {
    auto itr = uniqueEntityIDs_.find(_dataTypeName);
    if (itr == uniqueEntityIDs_.end()) {
        LOG_ERROR("EntityRepository: Unique entity not found with data type: {}", _dataTypeName);
        return false;
    }
    Entity& uniqueEntity   = entities_[itr->second];
    uniqueEntity.isUnique_ = false;
    uniqueEntityIDs_.erase(itr);
    return true;
}

/// <summary>
/// Entity を登録する
/// </summary>
int32_t EntityRepository::CreateEntity(const std::string& _dataType, bool _isUnique) {
    int32_t entityIndex = AllocateEntity();

    Entity& entity   = entities_[entityIndex];
    entity.id_       = entityIndex;
    entity.dataType_ = _dataType;
    entity.isAlive_  = true;
    entity.isUnique_ = false;

    if (_isUnique) {
        RegisterUniqueEntity(&entity);
    }

    return entityIndex;
}
/// <summary>
/// Entity を指定したIndexに登録する
/// </summary>
int32_t EntityRepository::CreateEntity(int32_t _id, const std::string& _dataType, bool _isUnique) {
    if (_id < 0 || static_cast<uint32_t>(_id) >= size_) {
        LOG_ERROR("EntityRepository: Invalid entity index allocated.");
        return kInvalidEntityID;
    }

    Entity& entity = entities_[_id];
    if (entity.isAlive_) {
        LOG_ERROR("EntityRepository: Entity with ID {} already exists.", _id);
        return kInvalidEntityID;
    }

    entity.id_                  = _id;
    entity.dataType_            = _dataType;
    entity.isAlive_             = true;
    entity.isUnique_            = false;
    uniqueEntityIDs_[_dataType] = _id;

    if (_isUnique) {
        RegisterUniqueEntity(&entity);
    }

    return _id;
}

/// <summary>
/// Entity を削除する
/// </summary>
bool EntityRepository::RemoveEntity(int32_t _entityIndex) {
    if (_entityIndex < 0 || static_cast<uint32_t>(_entityIndex) >= size_) {
        LOG_ERROR("EntityRepository: Invalid entity index to unregister.");
        return false;
    }

    Entity& entity = entities_[_entityIndex];

    if (!entity.isAlive_) {
        LOG_ERROR("EntityRepository: Entity already dead.");
        return false;
    }

    if (entity.isUnique_) {
        UnregisterUniqueEntity(entity.dataType_);
    }

    entity = Entity();

    entityActiveBits_.Set(_entityIndex, false);

    return true;
}

uint32_t EntityRepository::GetSize() const {
    return size_;
}
uint32_t EntityRepository::GetActiveEntityCount() const {
    return static_cast<uint32_t>(entityActiveBits_.GetTrueCount());
}
uint32_t EntityRepository::GetInactiveEntityCount() const {
    return static_cast<uint32_t>(entityActiveBits_.GetFalseCount());
}
void EntityRepository::Clear() {
    size_ = 0;
    entities_.clear();
    entityActiveBits_.resize(0);
}

const std::vector<Entity>& EntityRepository::GetEntities() const {
    return entities_;
}
std::vector<Entity>& EntityRepository::GetEntitiesRef() {
    return entities_;
}
