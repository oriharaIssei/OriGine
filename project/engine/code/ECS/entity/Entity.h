#pragma once

/// stl
#include <string>

/// engine
// ECS
class EntityRepository;

/// math
#include <stdint.h>

/// <summary>
/// 実体を表すクラス (実際にはIDでしか無い)
/// </summary>
class Entity {
    friend class EntityRepository;

public:
    Entity(const std::string& _dataType, int32_t _id, bool _isUnique = false)
        : dataType_(_dataType), id_(_id), IsAlive_(true), isUnique_(_isUnique) {}
    Entity() = default;

    ~Entity() = default;

    operator int32_t() const {
        return id_;
    }

private:
    std::string dataType_ = "UNKNOWN";

    int32_t id_      = -1;
    bool IsAlive_    = false;
    bool isUnique_   = false;
    bool ShouldSave_ = true;

public:
    bool IsAlive() const {
        return IsAlive_;
    }
    bool IsUnique() const {
        return isUnique_;
    }

    bool ShouldSave() const {
        return ShouldSave_;
    }
    void SetShouldSave(bool _ShouldSave) {
        ShouldSave_ = _ShouldSave;
    }

    void DeleteEntity() {
        IsAlive_ = false;
    }

    /// <summary>
    /// エンティティのIDを取得する
    /// </summary>
    int32_t GetID() const {
        return id_;
    }
    /// <summary>
    /// エンティティのデータタイプを取得する
    /// </summary>
    const std::string& GetDataType() const {
        return dataType_;
    }

    /// <summary>
    /// エンティティのデータタイプを設定する
    /// </summary>
    void SetDataType(const std::string& _dataType) {
        dataType_ = _dataType;
    }

    /// <summary>
    /// エンティティのユニークIDを取得する
    /// </summary>
    std::string GetUniqueID() const {
        return dataType_ + std::to_string(id_);
    }
};
