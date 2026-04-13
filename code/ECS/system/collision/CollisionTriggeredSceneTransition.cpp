#include "CollisionTriggeredSceneTransition.h"

/// ECS
// component
#include "component/collision/collider/AABBCollider.h"
#include "component/collision/collider/OBBCollider.h"
#include "component/collision/collider/SphereCollider.h"
#include "component/scene/SceneChanger.h"

using namespace OriGine;

OriGine::CollisionTriggeredSceneTransition::CollisionTriggeredSceneTransition() : ISystem(SystemCategory::Collision) {}

void OriGine::CollisionTriggeredSceneTransition::Initialize() {}
void OriGine::CollisionTriggeredSceneTransition::Finalize() {}

void OriGine::CollisionTriggeredSceneTransition::UpdateEntity(EntityHandle _handle) {
    auto& aabbColliders   = GetComponents<AABBCollider>(_handle);
    auto& sphereColliders = GetComponents<SphereCollider>(_handle);
    auto& obbColliders    = GetComponents<OBBCollider>(_handle);

    if (!aabbColliders.empty()) {
        for (auto& aabbCollider : aabbColliders) {
            if (!aabbCollider.IsActive()) {
                continue;
            }
            for (const auto& [otherHandle, collisionState] : aabbCollider.GetCollisionStateMap()) {
                if (collisionState == CollisionState::Enter) {
                    auto& sceneChangers = GetComponents<SceneChanger>(_handle);
                    for (auto& sceneChanger : sceneChangers) {
                        sceneChanger.ChangeScene();
                    }
                }
            }
        }
    }

    if (!sphereColliders.empty()) {
        for (auto& sphereCollider : sphereColliders) {
            if (!sphereCollider.IsActive()) {
                continue;
            }
            for (const auto& [otherHandle, collisionState] : sphereCollider.GetCollisionStateMap()) {
                if (collisionState == CollisionState::Enter) {
                    auto& sceneChangers = GetComponents<SceneChanger>(_handle);
                    for (auto& sceneChanger : sceneChangers) {
                        sceneChanger.ChangeScene();
                    }
                }
            }
        }
    }
    if (!obbColliders.empty()) {
        for (auto& obbCollider : obbColliders) {
            if (!obbCollider.IsActive()) {
                continue;
            }
            for (const auto& [otherHandle, collisionState] : obbCollider.GetCollisionStateMap()) {
                if (collisionState == CollisionState::Enter) {
                    auto& sceneChangers = GetComponents<SceneChanger>(_handle);
                    for (auto& sceneChanger : sceneChangers) {
                        sceneChanger.ChangeScene();
                    }
                }
            }
        }
    }
}
