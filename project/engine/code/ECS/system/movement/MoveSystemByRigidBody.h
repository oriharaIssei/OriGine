#pragma once
// parent
#include "system/ISystem.h"

/// engine

#include "util/globalVariables/SerializedField.h"

/// <summary>
/// Rigidbody によるTransformの移動を行うシステム
/// </summary>
class MoveSystemByRigidBody
    : public ISystem {
public:
    MoveSystemByRigidBody();
    ~MoveSystemByRigidBody();

    void Initialize() override;
    // void Update();
    void Finalize() override;

protected:
    void UpdateEntity(Entity* _entity) override;

protected:
    SerializedField<float> gravity_ = SerializedField<float>("Settings", "Physics", "Gravity");
};
