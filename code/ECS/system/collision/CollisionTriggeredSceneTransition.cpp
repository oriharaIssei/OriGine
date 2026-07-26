#include "CollisionTriggeredSceneTransition.h"

/// ECS
// component
#include "component/collision/collider/AABBCollider.h"
#include "component/collision/collider/OBBCollider.h"
#include "component/collision/collider/SphereCollider.h"
#include "component/scene/SceneChanger.h"

using namespace OriGine;

/// <summary>
/// コンストラクタ
/// </summary>
OriGine::CollisionTriggeredSceneTransition::CollisionTriggeredSceneTransition() : ISystem(SystemCategory::Collision) {}

/// <summary>
/// 初期化処理（このシステムは状態を持たないため何もしない）
/// </summary>
void OriGine::CollisionTriggeredSceneTransition::Initialize() {}
/// <summary>
/// 終了処理（このシステムは状態を持たないため何もしない）
/// </summary>
void OriGine::CollisionTriggeredSceneTransition::Finalize() {}

/// <summary>
/// エンティティが持つコライダーの衝突状態を確認し、いずれかが衝突し始めた瞬間(Enter)であれば
/// 同じエンティティが持つSceneChangerへシーン遷移を発火させる
/// </summary>
/// <param name="_handle">対象のエンティティハンドル</param>
void OriGine::CollisionTriggeredSceneTransition::UpdateEntity(const EntityHandle& _handle) {
    auto& aabbColliders   = GetComponents<AABBCollider>(_handle);
    auto& sphereColliders = GetComponents<SphereCollider>(_handle);
    auto& obbColliders    = GetComponents<OBBCollider>(_handle);

    // Stayではなく毎フレーム見るCollisionState::Enterだけを条件にすることで、
    // 触れ続けている間に何度もシーン遷移を発火させてしまうのを防いでいる
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
