#pragma once

/// engine
#include "scene/Scene.h"

/// ecs
// entity
#include "entity/Entity.h"
// component
#include "component/collision/collider/SphereCollider.h"
#include "component/collision/CollisionPushBackInfo.h"

/// math
#include "math/bounds/AABB.h"
#include "math/bounds/OBB.h"
#include "math/bounds/Sphere.h"
#include "math/mathEnv.h"

namespace OriGine {

/// <summary>
/// 2つの形状の衝突判定を行うベーステンプレート
/// </summary>
/// <typeparam name="ShapeA">一つ目の形状の型</typeparam>
/// <typeparam name="ShapeB">二つ目の形状の型</typeparam>
/// <param name="_scene">所属シーン</param>
/// <param name="_handleA">一つ目のエンティティハンドル</param>
/// <param name="_handleB">二つ目のエンティティハンドル</param>
/// <param name="_shapeA">一つ目の形状データ</param>
/// <param name="_shapeB">二つ目の形状データ</param>
/// <param name="_aInfo">一つ目のエンティティの押し戻し情報</param>
/// <param name="_bInfo">二つ目のエンティティの押し戻し情報</param>
/// <returns>衝突していればtrue</returns>
template <typename ShapeA, typename ShapeB>
bool CheckCollisionPair(
    [[maybe_unused]] Scene* _scene,
    [[maybe_unused]] EntityHandle _handleA,
    [[maybe_unused]] EntityHandle _handleB,
    [[maybe_unused]] const ShapeA& _shapeA,
    [[maybe_unused]] const ShapeB& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo) {
    return false;
}

/// <summary>
/// Sphere vs Sphere の衝突判定
/// </summary>
template <>
bool CheckCollisionPair(
    Scene* _scene,
    EntityHandle _handleA,
    EntityHandle _handleB,
    const Bounds::Sphere& _shapeA,
    const Bounds::Sphere& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo);

/// <summary>
/// AABB vs Sphere の衝突判定
/// </summary>
template <>
bool CheckCollisionPair(
    Scene* _scene,
    EntityHandle _aabbEntity,
    EntityHandle _sphereEntity,
    const Bounds::AABB& _aabb,
    const Bounds::Sphere& _sphere,
    [[maybe_unused]] CollisionPushBackInfo* _aabbInfo,
    [[maybe_unused]] CollisionPushBackInfo* _sphereInfo);

/// <summary>
/// Sphere vs AABB の衝突判定
/// </summary>
template <>
bool CheckCollisionPair(
    Scene* _scene,
    EntityHandle _handleA,
    EntityHandle _handleB,
    // 順番が A, B と B, A で同じ処理を行うため、関数を呼び出しを入れ替える
    const Bounds::Sphere& _shapeA,
    const Bounds::AABB& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo);

/// <summary>
/// Sphere vs OBB の衝突判定
/// </summary>
template <>
bool CheckCollisionPair(
    Scene* _scene,
    EntityHandle _handleA,
    EntityHandle _handleB,
    const Bounds::Sphere& _shapeA,
    const Bounds::OBB& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo);

/// <summary>
/// OBB vs Sphere の衝突判定
/// </summary>
template <>
bool CheckCollisionPair(
    Scene* _scene,
    EntityHandle _handleA,
    EntityHandle _handleB,
    // 順番が A, B と B, A で同じ処理を行うため、関数を呼び出しを入れ替える
    const Bounds::OBB& _shapeA,
    const Bounds::Sphere& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo);

/// <summary>
/// AABB vs AABB の衝突判定
/// </summary>
template <>
bool CheckCollisionPair(
    Scene* _scene,
    EntityHandle _handleA,
    EntityHandle _handleB,
    const Bounds::AABB& _shapeA,
    const Bounds::AABB& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo);

/// <summary>
/// OBB vs OBB の衝突判定
/// </summary>
template <>
bool CheckCollisionPair(
    Scene* _scene,
    EntityHandle _handleA,
    EntityHandle _handleB,
    const Bounds::OBB& _shapeA,
    const Bounds::OBB& _shapeB,
    CollisionPushBackInfo* _aInfo,
    CollisionPushBackInfo* _bInfo);

/// <summary>
/// AABB vs OBB の衝突判定
/// </summary>
template <>
bool CheckCollisionPair(
    Scene* _scene,
    EntityHandle _handleA,
    EntityHandle _handleB,
    const Bounds::AABB& _shapeA,
    const Bounds::OBB& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo);

/// <summary>
/// OBB vs AABB の衝突判定
/// </summary>
template <>
bool CheckCollisionPair(
    Scene* _scene,
    EntityHandle _handleA,
    EntityHandle _handleB,
    const Bounds::OBB& _shapeA,
    const Bounds::AABB& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo);

#pragma endregion

} // namespace OriGine
