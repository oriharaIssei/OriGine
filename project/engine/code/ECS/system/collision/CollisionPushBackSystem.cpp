#include "CollisionPushBackSystem.h"

/// ecs
#include "component/collision/CollisionPushBackInfo.h"
#include "component/physics/Rigidbody.h"
#include "component/transform/Transform.h"

#include "logger/Logger.h"

using namespace OriGine;

void CollisionPushBackSystem::Initialize() {}
void CollisionPushBackSystem::Finalize() {}

void CollisionPushBackSystem::UpdateEntity(EntityHandle _handle) {
    CollisionPushBackInfo* collPushbackInfo = GetComponent<CollisionPushBackInfo>(_handle);

    if (collPushbackInfo == nullptr) {
        LOG_ERROR("EntityHandle {} has no CollisionPushBackInfo component.", uuids::to_string(_handle.uuid));
        return;
    }

    Vec3f pushBackSum = Vec3f(0.f, 0.f, 0.f);

    // PushBack処理
    for (auto& [entityID, info] : collPushbackInfo->GetCollisionInfoMap()) {

        switch (info.pushBackType) {
        case CollisionPushBackType::PushBack: {
            // ここでは単純に法線方向に押し戻す
            pushBackSum += info.collVec;

            break;
        }
        case CollisionPushBackType::Reflect: {
            pushBackSum += info.collVec;

            Rigidbody* rigidbody = GetComponent<Rigidbody>(_handle);

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

    Transform* transform = GetComponent<Transform>(_handle);
    transform->translate += pushBackSum;
}
