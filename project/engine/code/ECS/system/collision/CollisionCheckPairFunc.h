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
/// Sphere vs Sphere
/// </summary>
/// <typeparam name="ShapeA"></typeparam>
/// <typeparam name="ShapeB"></typeparam>
/// <param name="_entityA"></param>
/// <param name="_entityB"></param>
/// <param name="_shapeA"></param>
/// <param name="_shapeB"></param>
/// <param name="_aInfo"></param>
/// <param name="_bInfo"></param>
/// <returns></returns>
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
/// Sphere vs AABB
/// </summary>
/// <typeparam name="ShapeA"></typeparam>
/// <typeparam name="ShapeB"></typeparam>
/// <param name="_entityA"></param>
/// <param name="_entityB"></param>
/// <param name="_shapeA"></param>
/// <param name="_shapeB"></param>
/// <param name="_aInfo"></param>
/// <param name="_bInfo"></param>
/// <returns></returns>
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
///  Sphere vs AABB
/// </summary>
/// <typeparam name="ShapeA"></typeparam>
/// <typeparam name="ShapeB"></typeparam>
/// <param name="_entityA"></param>
/// <param name="_entityB"></param>
/// <param name="_shapeA"></param>
/// <param name="_shapeB"></param>
/// <param name="_aInfo"></param>
/// <param name="_bInfo"></param>
/// <returns></returns>
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
/// Sphere vs OBB
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
/// Sphere vs OBB
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
/// AABB vs AABB
/// </summary>
/// <typeparam name="ShapeA"></typeparam>
/// <typeparam name="ShapeB"></typeparam>
/// <param name="_entityA"></param>
/// <param name="_entityB"></param>
/// <param name="_shapeA"></param>
/// <param name="_shapeB"></param>
/// <param name="_aInfo"></param>
/// <param name="_bInfo"></param>
/// <returns></returns>
template <>
bool CheckCollisionPair(
    Scene* _scene,
    EntityHandle _handleA,
    EntityHandle _handleB,
    const Bounds::AABB& _shapeA,
    const Bounds::AABB& _shapeB,
    [[maybe_unused]] CollisionPushBackInfo* _aInfo,
    [[maybe_unused]] CollisionPushBackInfo* _bInfo);

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
/// AABB vs OBB
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
/// OBB vs AABB
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
