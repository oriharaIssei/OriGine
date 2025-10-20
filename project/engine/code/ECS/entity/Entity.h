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
        : dataType_(_dataType), id_(_id), isAlive_(true), isUnique_(_isUnique) {}
    Entity() = default;

    ~Entity() = default;

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
