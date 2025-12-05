#include "CollisionPushBackSystem.h"

/// ecs
#include "component/collision/CollisionPushBackInfo.h"
#include "component/physics/Rigidbody.h"
#include "component/transform/Transform.h"

#include "logger/Logger.h"

void CollisionPushBackSystem::Initialize() {}

void CollisionPushBackSystem::Finalize() {}

void CollisionPushBackSystem::UpdateEntity(Entity* _entity) {
    CollisionPushBackInfo* collPushbackInfo = GetComponent<CollisionPushBackInfo>(_entity);

    if (collPushbackInfo == nullptr) {
        LOG_ERROR("Entity {} has no CollisionPushBackInfo component.", _entity->GetID());
        return;
    }

    Vec3f pushBackSum = Vec3f(0.f, 0.f, 0.f); 

    // PushBack処理
    for (auto& [entityID, info] : collPushbackInfo->GetCollisionInfoMap()) {
       
        switch (info.pushBackType){
        case CollisionPushBackType::PushBack: {
            // ここでは単純に法線方向に押し戻す
            pushBackSum += info.collVec;

            break;
        }
        case CollisionPushBackType::Reflect: {
            pushBackSum += info.collVec;

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

    Transform* transform = GetComponent<Transform>(_entity);
    transform->translate += pushBackSum;
}
