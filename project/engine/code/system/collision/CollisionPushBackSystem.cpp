#include "CollisionPushBackSystem.h"

/// ecs
#include "component/collider/CollisionPushBackInfo.h"
#include "component/physics/Rigidbody.h"
#include "component/transform/Transform.h"
#include "ECSManager.h"

/// lib
#include "logger/Logger.h"

void CollisionPushBackSystem::Initialize() {
}

void CollisionPushBackSystem::Finalize() {
}

void CollisionPushBackSystem::UpdateEntity(GameEntity* _entity) {
    Transform* transform                    = getComponent<Transform>(_entity);
    CollisionPushBackInfo* collPushbackInfo = getComponent<CollisionPushBackInfo>(_entity);

    if (transform == nullptr || collPushbackInfo == nullptr) {
        LOG_ERROR("CollisionPushBackSystem: Entity {} has no Transform or CollisionPushBackInfo component.", _entity->getID());
        return;
    }
    if (collPushbackInfo->getPushBackType() == CollisionPushBackType::None) {
        return;
    }

    // PushBack処理
    for (auto& [entityID, info] : collPushbackInfo->getCollisionStateMap()) {
        switch (collPushbackInfo->getPushBackType()) {
        case CollisionPushBackType::PushBack: {
            // ここでは単純に法線方向に押し戻す
            transform->translate += info.collVec;
            break;
        }
        case CollisionPushBackType::Reflect: {
            transform->translate += info.collVec;

            Rigidbody* rigidbody = getComponent<Rigidbody>(_entity);

            // 衝突時に反射する
            // 反射ベクトルを計算
            Vec3f reflectDir = Reflect(rigidbody->getVelocity(), info.collVec.normalize());
            // 反射後の速度を設定
            if (rigidbody) {
                rigidbody->setVelocity(reflectDir);
                rigidbody->setAcceleration(reflectDir.normalize() * rigidbody->getAcceleration().length());
            }
            break;
        }
        default:
            break;
        }
    }
}
