#pragma once

/// stl
#include <string>

/// ECS
// entity
#include "entity/EntityHandle.h"

/// externals
#include <uuid/uuid.h>

/// math
#include <stdint.h>

namespace OriGine {

/// engine
// ECS
class EntityRepository;

constexpr int32_t kInvalidEntityID = -1;

/// <summary>
/// 実体を表すクラス (実際にはIDでしか無い)
/// </summary>
class Entity {
    friend class EntityRepository;

public:
    Entity(const ::std::string& _dataType, int32_t _id, EntityHandle _handle, bool _isUnique = false)
        : dataType_(_dataType), id_(_id), handle_(_handle), isAlive_(true), isUnique_(_isUnique) {}
    Entity() = default;

    ~Entity() = default;

private:
    ::std::string dataType_ = "UNKNOWN";

    int32_t id_       = kInvalidEntityID;
    EntityHandle handle_ = EntityHandle();
    bool isAlive_     = false;
    bool isUnique_    = false;
    bool shouldSave_  = true;

public:
    EntityHandle GetHandle() const {
        return handle_;
    }

    bool IsAlive() const {
        return isAlive_;
    }
    bool IsUnique() const {
        return isUnique_;
    }
    bool ShouldSave() const {
        return shouldSave_;
    }
    void SetShouldSave(bool _ShouldSave) {
        shouldSave_ = _ShouldSave;
    }

    /// <summary>
    /// エンティティのデータタイプを取得する
    /// </summary>
    const ::std::string& GetDataType() const {
        return dataType_;
    }

    /// <summary>
    /// エンティティのデータタイプを設定する
    /// </summary>
    void SetDataType(const ::std::string& _dataType) {
        dataType_ = _dataType;
    }

    /// <summary>
    /// エンティティのユニークIDを取得する
    /// </summary>
    ::std::string GetUniqueID() const {
        return dataType_ + ::std::to_string(id_);
    }
};

} // namespace OriGine
