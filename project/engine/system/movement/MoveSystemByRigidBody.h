#pragma once
// parent
#include "system/ISystem.h"

/// engine
// lib
#include "lib/globalVariables/SerializedField.h"

class MoveSystemByRigidBody
    : public ISystem {
public:
    MoveSystemByRigidBody();
    ~MoveSystemByRigidBody();

    void Init() override;
    // void Update();
    void Finalize() override;

protected:
    void UpdateEntity(GameEntity* _entity) override;

protected:
    SerializedField<float> gravity_ = SerializedField<float>("InGame", "Physics", "Gravity");
};
