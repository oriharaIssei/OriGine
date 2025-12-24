#include "MoveSystemByRigidBody.h"

/// Engine
#define ENGINE_INCLUDE
#include "engine/EngineInclude.h"
#include "scene/Scene.h"
/// ECS
// component
#include "component/physics/Rigidbody.h"
#include "component/transform/Transform.h"

#include "logger/Logger.h"

using namespace OriGine;

MoveSystemByRigidBody::MoveSystemByRigidBody() : ISystem(SystemCategory::Movement) {}
MoveSystemByRigidBody::~MoveSystemByRigidBody() {}

void MoveSystemByRigidBody::Initialize() {}

void MoveSystemByRigidBody::Finalize() {}

void MoveSystemByRigidBody::UpdateEntity(EntityHandle _handle) {
    const float deltaTime = Engine::GetInstance()->GetDeltaTime();
    Transform* transform  = GetComponent<Transform>(_handle);

    Rigidbody* rigidbody = GetComponent<Rigidbody>(_handle);

    bool resourceCheck = (transform != nullptr) && (rigidbody != nullptr);
    if (!resourceCheck) {
        Entity* entity = GetScene()->GetEntity(_handle);
        if (!transform) {
            LOG_ERROR("{} doesn't have Transform", entity->GetUniqueID());
        }
        if (!rigidbody) {
            LOG_ERROR("{} doesn't have Rigidbody", entity->GetUniqueID());
        }
        return;
    }

    /// --------------------------------------- 速度の更新 --------------------------------------- ///
    Vec3f acceleration = rigidbody->GetAcceleration();
    Vec3f velocity     = rigidbody->GetVelocity();

    // 重力加速度
    if (rigidbody->GetUseGravity()) {
        acceleration[Y] -= (std::min)(gravity_ * rigidbody->GetMass(), rigidbody->maxFallSpeed());
    }
    rigidbody->SetAcceleration(acceleration);

    // 速度の更新
    velocity += acceleration * deltaTime;

    // xz 平面の速度制限
    Vec2f xz           = Vec2f(velocity[X], velocity[Z]);
    float limitXZSpeed = rigidbody->GetMaxXZSpeed();
    if (xz.lengthSq() >= limitXZSpeed * limitXZSpeed) {
        xz          = xz.normalize() * limitXZSpeed;
        velocity[X] = xz[X];
        velocity[Z] = xz[Y];
    }
    float maxFallSpeed = rigidbody->maxFallSpeed();
    if (velocity[Y] * velocity[Y] >= maxFallSpeed * maxFallSpeed) {
        velocity[Y] = (velocity[Y] > 0) ? maxFallSpeed : -maxFallSpeed;
    }

    rigidbody->SetVelocity(velocity);

    /// --------------------------------------- 位置の更新 --------------------------------------- ///
    Vec3f realVelo = velocity * deltaTime;
    transform->translate += realVelo;

    rigidbody->SetRealVelocity(realVelo);

    // worldMatの更新
    transform->UpdateMatrix();
}
