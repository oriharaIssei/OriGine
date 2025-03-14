#include "MoveSystemByRigidBody.h"

/// Engine
#define ENGINE_INCLUDE
/// ECS
#define ENGINE_ECS
#include "EngineInclude.h"

MoveSystemByRigidBody::MoveSystemByRigidBody() : ISystem(SystemType::Movement) {}

MoveSystemByRigidBody::~MoveSystemByRigidBody() {}

void MoveSystemByRigidBody::Initialize() {}

void MoveSystemByRigidBody::Finalize() {}

void MoveSystemByRigidBody::UpdateEntity(GameEntity* _entity) {
    const float deltaTime = Engine::getInstance()->getDeltaTime();
    Transform* transform  = getComponent<Transform>(_entity);

    Rigidbody* rigidbody = getComponent<Rigidbody>(_entity);

    /// --------------------------------------- 速度の更新 --------------------------------------- ///
    Vec3f acceleration = rigidbody->getAcceleration();
    Vec3f velocity     = rigidbody->getVelocity();

    // 重力加速度
    if (rigidbody->getUseGravity()) {
        acceleration[Y] -= gravity_;
    }

    // 速度の更新
    velocity += acceleration * deltaTime;

    rigidbody->setAcceleration(acceleration);
    rigidbody->setVelocity(velocity);

    /// --------------------------------------- 位置の更新 --------------------------------------- ///
    transform->translate += velocity * deltaTime;
    // worldMatの更新
    transform->Update();
}
