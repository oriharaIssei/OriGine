#pragma once
// parent
#include "system/ISystem.h"

/// util
#include "util/globalVariables/SerializedField.h"

namespace OriGine {

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

} // namespace OriGine
