#pragma once
#include "system/ISystem.h"

class CollisionPushBackSystem
    : public ISystem {
public:
    CollisionPushBackSystem()
        : ISystem(SystemType::Collision) {}
    ~CollisionPushBackSystem() {}

    void Initialize() override;
    // void Update() override;
    void Finalize() override;

protected:
    void UpdateEntity(GameEntity* _entity) override;
};
