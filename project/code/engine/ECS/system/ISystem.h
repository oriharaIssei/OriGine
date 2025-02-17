#pragma once

/// stl
#include <vector>

/// engine
// ECS
#include "ECS/Entity.h"

class ISystem {
public:
    ISystem()          = default;
    virtual ~ISystem() = default;

    void Update();

protected:
    virtual void UpdateEntity(GameEntity* _entity) = 0;

private:
    std::vector<GameEntity*> entities_;

public:
    void AddEntity(GameEntity* _entity) {
        entities_.push_back(_entity);
    }
    void RemoveEntity(GameEntity* _entity) {
        entities_.erase(std::remove(entities_.begin(), entities_.end(), _entity), entities_.end());
    }
    void clear() {
        entities_.clear();
    }
};
