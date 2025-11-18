#include "CollisionPushBackSystem.h"

/// ecs
#include "component/collision/CollisionPushBackInfo.h"
#include "component/physics/Rigidbody.h"
#include "component/transform/Transform.h"

#include "logger/Logger.h"

void CollisionPushBackSystem::Initialize() {}

void CollisionPushBackSystem::Finalize() {}

void CollisionPushBackSystem::UpdateEntity(Entity* _entity) {
    Transform* transform                    = GetComponent<Transform>(_entity);
    CollisionPushBackInfo* collPushbackInfo = GetComponent<CollisionPushBackInfo>(_entity);

    if (transform == nullptr || collPushbackInfo == nullptr) {
        LOG_ERROR("CollisionPushBackSystem: Entity {} has no Transform or CollisionPushBackInfo component.", _entity->GetID());
        return;
    }

    // PushBack処理
    for (auto& [entityID, info] : collPushbackInfo->GetCollisionInfoMap()) {
        Entity* otherEntity                      = GetEntity(entityID);
        CollisionPushBackInfo* otherCollPushbackInfo = GetComponent<CollisionPushBackInfo>(otherEntity);

        switch (otherCollPushbackInfo->GetPushBackType()) {
        case CollisionPushBackType::PushBack: {
            // ここでは単純に法線方向に押し戻す
            transform->translate += info.collVec;

            break;
        }
        case CollisionPushBackType::Reflect: {
            transform->translate += info.collVec;

            Rigidbody* rigidbody = GetComponent<Rigidbody>(_entity);

            // 衝突時に反射する
            // 反射ベクトルを計算
            Vec3f reflectDir = Reflect(rigidbody->GetVelocity(), info.collVec.normalize());
            // 反射後の速度を設定
            if (rigidbody) {
                rigidbody->SetVelocity(reflectDir);
                rigidbody->SetAcceleration(reflectDir.normalize() * rigidbody->GetAcceleration().length());
            }
            break;
        }
        default:
            break;
        }
    }
}
