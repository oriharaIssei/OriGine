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

/// <summary>
/// デストラクタ
/// </summary>
MoveSystemByRigidBody::~MoveSystemByRigidBody() {}

/// <summary>
/// 初期化処理
/// </summary>
void MoveSystemByRigidBody::Initialize() {}

/// <summary>
/// 終了処理
/// </summary>
void MoveSystemByRigidBody::Finalize() {}

/// <summary>
/// 各エンティティのRigidbodyに基づいた物理移動を計算し、Transformを更新する
/// </summary>
/// <param name="_handle">対象のエンティティハンドル</param>
void MoveSystemByRigidBody::UpdateEntity(EntityHandle _handle) {
    float deltaTime = 0.f;

    Transform* transform = GetComponent<Transform>(_handle);

    Rigidbody* rigidbody = GetComponent<Rigidbody>(_handle);
    bool resourceCheck   = (transform != nullptr) && (rigidbody != nullptr);

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
    if (!rigidbody->IsActive()) {
        return;
    }

    if (rigidbody->IsUsingLocalDeltaTime()) {
        deltaTime = Engine::GetInstance()->GetDeltaTimer()->GetScaledDeltaTime(rigidbody->GetLocalDeltaTimeName());
    } else {
        deltaTime = Engine::GetInstance()->GetDeltaTime();
    }

    /// --------------------------------------- 速度の更新 --------------------------------------- ///
    Vec3f acceleration = rigidbody->GetAcceleration();
    Vec3f velocity     = rigidbody->GetVelocity();
    float maxFallSpeed = rigidbody->MaxFallSpeed();

    // 重力加速度
    if (rigidbody->GetUseGravity()) {
        acceleration[Y] -= (std::min)(gravity_ * rigidbody->GetMass(), maxFallSpeed);
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
    if (velocity[Y] * velocity[Y] >= maxFallSpeed * maxFallSpeed) {
        velocity[Y] = (velocity[Y] > 0) ? maxFallSpeed : -maxFallSpeed;
    }

    rigidbody->SetVelocity(velocity);

    /// --------------------------------------- 位置の更新 --------------------------------------- ///
    rigidbody->SetPrePos(transform->translate);
    Vec3f realVelo = velocity * deltaTime;
    transform->translate += realVelo;

    rigidbody->SetRealVelocity(realVelo);

    // worldMatの更新
    transform->UpdateMatrix();
}
