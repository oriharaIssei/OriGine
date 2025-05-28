#pragma once

/// stl
#include <stdint.h>
#include <string>

/// engine
// ECS
class EntityComponentSystemManager;

/// <summary>
/// 実体を表すクラス (実際にはIDでしか無い)
/// </summary>
class GameEntity {
    friend class EntityComponentSystemManager;

public:
    GameEntity(const std::string& _dataType, int32_t _id, bool _isUnique = false)
        : dataType_(_dataType), id_(_id), isAlive_(true), isUnique_(_isUnique) {}
    GameEntity() = default;

    ~GameEntity() = default;

    operator int32_t() const {
        return id_;
    }

private:
    int32_t id_           = -1;
    std::string dataType_ = "UNKNOWN";

    bool isAlive_  = false;
    bool isUnique_ = false;

public:
    bool isAlive() const {
        return isAlive_;
    }
    bool isUnique() const {
        return isUnique_;
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
