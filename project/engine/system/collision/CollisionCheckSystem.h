#pragma once

/// parent
#include "system/ISystem.h"

/// ECS
// component
struct Transform;

class CollisionCheckSystem
    : public ISystem {
public:
    CollisionCheckSystem();
    ~CollisionCheckSystem();

    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    void UpdateEntity(GameEntity* _entity) override;

protected:
    std::vector<GameEntity*> entities_;
    std::vector<GameEntity*>::iterator entityItr_;
};

template <typename ShapeA, typename ShapeB>
bool CheckCollisionPair(const ShapeA& _shapeA, const ShapeB& _shapeB) {
    return false;
}
