#include "EntityRepository.h"

#include "entity/Entity.h"

/// externals
#include "logger/Logger.h"
#include <uuid/uuid.h>
#include <uuidGenerator/UuidGenerator.h>

using namespace OriGine;

EntityRepository::EntityRepository() {}
EntityRepository::~EntityRepository() {}

void EntityRepository::Initialize() {
    entities_.resize(size_);
    entityActiveBits_.resize(size_);
}

void EntityRepository::Finalize() {
    Clear();
}

int32_t EntityRepository::AllocateIndex() {
    if (entityActiveBits_.GetTrueCount() >= size_) {
        size_ *= 2;
        entities_.resize(size_);
        entityActiveBits_.resize(size_);
    }
    return static_cast<int32_t>(entityActiveBits_.allocateBit());
}

int32_t OriGine::EntityRepository::FindIndex(const uuids::uuid& _uuid) const {
    auto it = uuidToIndex_.find(_uuid);
    if (it == uuidToIndex_.end()) {
        LOG_ERROR("Entity not found \n uuid : {}", uuids::to_string(_uuid));
        return -1;
    }
    return it->second;
}

EntityHandle EntityRepository::CreateEntity(const std::string& _type, bool _unique) {
    int32_t index = AllocateIndex();

    Entity& e   = entities_[index];
    e.id_       = index;
    e.dataType_ = _type;
    e.isAlive_  = true;
    e.isUnique_ = false;
    e.handle_   = EntityHandle(UuidGenerator::RandomGenerate());

    entityActiveBits_.Set(index, true);
    uuidToIndex_[e.handle_.uuid] = index;

    if (_unique) {
        uniqueEntities_[_type] = e.handle_.uuid;
        e.isUnique_            = true;
    }

    return e.handle_;
}

bool OriGine::EntityRepository::RgisterUniqueEntity(const std::string& _dataType, EntityHandle _handle) {
    auto it = uuidToIndex_.find(_handle.uuid);
    if (it == uuidToIndex_.end()) {
        LOG_ERROR("Entity not found. \n uuid : {}", uuids::to_string(_handle.uuid));
        return false;
    }
    int32_t index = it->second;
    Entity& e     = entities_[index];
    if (e.isUnique_) {
        LOG_ERROR("Entity is already unique. \n uuid : {}", uuids::to_string(_handle.uuid));
        return false;
    }
    uniqueEntities_[_dataType] = _handle.uuid;
    e.isUnique_                = true;
    return true;
}

bool EntityRepository::RemoveEntity(EntityHandle _handle) {
    auto it = uuidToIndex_.find(_handle.uuid);
    if (it == uuidToIndex_.end()) {
        return false;
    }

    int32_t index = it->second;
    Entity& e     = entities_[index];

    if (e.isUnique_) {
        uniqueEntities_.erase(e.dataType_);
    }

    uuidToIndex_.erase(it);
    entityActiveBits_.Set(index, false);

    e = Entity();
    return true;
}

void OriGine::EntityRepository::Clear() {
    entities_.clear();
    entityActiveBits_.resize(0);
    uuidToIndex_.clear();
    uniqueEntities_.clear();
}

Entity* EntityRepository::GetEntity(EntityHandle _handle) {
    auto itr = uuidToIndex_.find(_handle.uuid);
    if (itr == uuidToIndex_.end()) {
        LOG_ERROR("Entity not fount. \n uuid : {}", uuids::to_string(_handle.uuid));
        return nullptr;
    }
    return &entities_[itr->second];
}

bool EntityRepository::IsAlive(EntityHandle _handle) const {
    return uuidToIndex_.contains(_handle.uuid);
}

EntityHandle EntityRepository::GetUniqueEntity(const std::string& _type) {
    auto itr = uniqueEntities_.find(_type);
    if (itr == uniqueEntities_.end()) {
        LOG_ERROR("Unique entity not fount. \n type : {}", _type);
        return {};
    }
    return EntityHandle{itr->second};
}
