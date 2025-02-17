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
    GameEntity(const std::string& _dataType, uint32_t _id)
        : dataType_(_dataType), id_(_id) {}

    ~GameEntity() = default;

    operator uint32_t() const {
        return id_;
    }

private:
    uint32_t id_;
    std::string dataType_;

public:
    uint32_t getID() const {
        return id_;
    }
    const std::string& getDataType() const {
        return dataType_;
    }
    const std::string& getUniqueID() const {
        return dataType_ + std::to_string(id_);
    }
};
