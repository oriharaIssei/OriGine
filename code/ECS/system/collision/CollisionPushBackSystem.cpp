#include "CollisionPushBackSystem.h"

/// ecs
#include "component/collision/CollisionPushBackInfo.h"
#include "component/physics/Rigidbody.h"
#include "component/transform/Transform.h"

#include "logger/Logger.h"

using namespace OriGine;

/// <summary>
/// 初期化
/// </summary>
void CollisionPushBackSystem::Initialize() {}

/// <summary>
/// 終了処理
/// </summary>
void CollisionPushBackSystem::Finalize() {}

/// <summary>
/// エンティティの押し戻し処理を行う
/// </summary>
/// <param name="_handle">対象のエンティティハンドル</param>
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

            Rigidbody* rigidbody      = GetComponent<Rigidbody>(_handle);
            Rigidbody* otherRigidbody = GetComponent<Rigidbody>(EntityHandle(entityID));

            if (rigidbody) {
                Vec3f velocity = rigidbody->GetVelocity();
                Vec3f normal   = info.collVec.normalize();

                float restitution = rigidbody->GetRestitution();
                if (otherRigidbody) {
                    restitution = std::max(restitution, otherRigidbody->GetRestitution());
                }

                velocity = Reflect(velocity, normal, restitution);

                rigidbody->SetVelocity(velocity);
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
