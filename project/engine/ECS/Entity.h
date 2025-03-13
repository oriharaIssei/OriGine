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
    GameEntity(const std::string& _dataType, int32_t _id)
        : dataType_(_dataType), id_(_id) {}
    GameEntity() = default;

    ~GameEntity() = default;

    operator int32_t() const {
        return id_;
    }

private:
    int32_t id_           = -1;
    std::string dataType_ = "UNKNOWN";

    bool isAlive_ = true;

public:
    void setAlive(bool _isAlive) {
        isAlive_ = _isAlive;
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
    /// エンティティのユニークIDを取得する
    /// </summary>
    std::string getUniqueID() const {
        return dataType_ + std::to_string(id_);
    }
};
