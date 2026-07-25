#include "CollisionCheckPairFunc.h"
#include "CollisionCheckUtility.h"

/// math
#include "math/MathEnv.h"
#include "math/Vector3.h"

namespace OriGine {

#pragma region Capsule Collisions

/// <summary>
/// Capsule vs Capsule の衝突判定の実装
/// </summary>
/// <remarks>
/// カプセルは「線分から半径r以内の空間」と定義できるため、
/// 2つのカプセルの衝突判定は「2本の線分の最接近距離が半径の和以下か」に帰着する。
/// この考え方(軸となる形状の最接近点を求め、半径の和と比べる)は
/// 以下のCapsule vs Sphere / AABB / OBB でも共通で、相手の形状に応じて
/// 最接近点の求め方だけが変わる。
///
/// 押し戻し情報(CollisionPushBackInfo)の組み立ても本ファイル内で共通の形をとる。
/// ・normal は A から B へ向かう単位ベクトル。B 側には反転して渡す
/// ・penetration は「半径の和 - 実距離」で、めり込んでいる深さ
/// ・overlapRate は押し戻しを両者で分担するための係数
/// ・A に記録する pushBackType は相手(B)側の設定を使う。
///   「自分がどう押されるか」は相手が押し返す種類で決まるため
/// </remarks>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::Capsule& _shapeA, const Bounds::Capsule& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    Vec3f closest1, closest2;
    ClosestPointsBetweenSegments(
        _shapeA.segment.start, _shapeA.segment.end,
        _shapeB.segment.start, _shapeB.segment.end,
        closest1, closest2);

    Vec3f diff      = closest2 - closest1;
    float distSq    = diff.lengthSq();
    float radiusSum = _shapeA.radius + _shapeB.radius;

    // 距離と半径の和を、どちらも2乗したまま比較する。
    // 判定に必要なのは大小関係だけなので、毎フレーム大量に呼ばれるここでは
    // 平方根を省いて済ませる(sqrtは下の押し戻し量の計算まで遅延させる)
    if (distSq > radiusSum * radiusSum) {
        return false;
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    Vec3f normal = diff.normalize();

    float dist        = std::sqrt(distSq);
    // めり込み量 = 触れ合うはずの距離(半径の和) - 実際の距離
    float penetration = radiusSum - dist;

    // 押し戻しの分担率。両者とも押し戻す設定なら 1/2 ずつ動いて合計でめり込みが解消し、
    // 片方だけが押し戻される(相手が壁など不動)なら 1/1 でその片方が全量を負担する。
    // NOTE: 双方ともNoneの場合は分母が0になりinfになるが、
    //       その場合pushBackTypeもNoneとなり押し戻し処理自体が行われないため実害はない
    bool aIsPushBack  = _bInfo->GetPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack  = _aInfo->GetPushBackType() != CollisionPushBackType::None;
    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    // 衝突点は「Aの表面上」に取る。Aの軸上の最接近点から、法線方向へ半径分進めた位置
    Vec3f collPoint = closest1 + normal * _shapeA.radius;

    CollisionPushBackInfo::Info aInfo;
    aInfo.pushBackType   = _bInfo->GetPushBackType();
    aInfo.collPoint      = collPoint;
    aInfo.collFaceNormal = normal;
    aInfo.collVec        = normal * penetration * overlapRate;
    _aInfo->AddCollisionInfo(_handleB, aInfo);

    // B側は同じ衝突を反対から見たものなので、法線と押し戻しベクトルの符号だけ反転させる
    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType   = _aInfo->GetPushBackType();
    bInfo.collPoint      = collPoint;
    bInfo.collFaceNormal = -normal;
    bInfo.collVec        = -normal * penetration * overlapRate;
    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

/// <summary>
/// Capsule vs Sphere の衝突判定の実装
/// </summary>
/// <remarks>
/// 球は「1点から半径r以内」なので、カプセルの線分上で球の中心に最も近い点を求め、
/// そこから球の中心までの距離を半径の和と比べればよい。
/// 押し戻し情報の組み立て方はCapsule vs Capsuleと同じ(そちらの説明を参照)。
/// </remarks>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::Capsule& _shapeA, const Bounds::Sphere& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    Vec3f closest   = ClosestPointOnSegment(_shapeB.center_, _shapeA.segment.start, _shapeA.segment.end);
    Vec3f diff      = _shapeB.center_ - closest;
    float distSq    = diff.lengthSq();
    float radiusSum = _shapeA.radius + _shapeB.radius_;

    if (distSq > radiusSum * radiusSum) {
        return false;
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    Vec3f normal      = diff.normalize();
    float dist        = std::sqrt(distSq);
    float penetration = radiusSum - dist;

    bool aIsPushBack  = _bInfo->GetPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack  = _aInfo->GetPushBackType() != CollisionPushBackType::None;
    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    Vec3f collPoint = closest + normal * _shapeA.radius;

    CollisionPushBackInfo::Info aInfo;
    aInfo.pushBackType   = _bInfo->GetPushBackType();
    aInfo.collPoint      = collPoint;
    aInfo.collFaceNormal = normal;
    aInfo.collVec        = normal * penetration * overlapRate;
    _aInfo->AddCollisionInfo(_handleB, aInfo);

    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType   = _aInfo->GetPushBackType();
    bInfo.collPoint      = collPoint;
    bInfo.collFaceNormal = -normal;
    bInfo.collVec        = -normal * penetration * overlapRate;
    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

/// <summary>
/// Sphere vs Capsule の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::Sphere& _shapeA, const Bounds::Capsule& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    // 判定は形状の順序によらず同じなので、Capsule vs Sphere 版に丸投げする。
    // その際ハンドル・形状・押し戻し情報の全てをA/B入れ替えて渡すこと。
    // 一部だけ入れ替えると、法線の向きや押し戻し先が逆のエンティティに記録されてしまう
    return CheckCollisionPair<Bounds::Capsule, Bounds::Sphere>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

/// <summary>
/// Capsule vs AABB の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::Capsule& _shapeA, const Bounds::AABB& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    // 反復最近接点法：線分上の点とAABB上の点を交互に更新して収束させる
    // 線分とAABBの最接近点には、線分同士のような単純な閉じた解が無い(相手の面・辺・頂点の
    // どこが最も近いかで式が変わる)。そこで「相手の現在の最接近点に最も近い点」を
    // 互いに求め直す操作を繰り返し、動かなくなった点を近似解として使う。
    // 初期値に線分の中心を選ぶのは、どちらの端に寄っている場合でも偏りが少ないため
    Vec3f closestOnSeg  = _shapeA.segment.Center();
    Vec3f closestOnAABB = ClosestPointOnAABB(closestOnSeg, _shapeB);

    // 各反復で点は必ず相手に近づくため単調に収束する。実測では数回で十分収束するが、
    // 収束しない形状でも1フレームの処理時間が伸びないよう上限を設けている
    constexpr int kMaxIterations = 8;
    for (int i = 0; i < kMaxIterations; ++i) {
        Vec3f prevOnSeg  = closestOnSeg;
        closestOnSeg     = ClosestPointOnSegment(closestOnAABB, _shapeA.segment.start, _shapeA.segment.end);
        closestOnAABB    = ClosestPointOnAABB(closestOnSeg, _shapeB);

        // 収束判定
        if (Vec3f(closestOnSeg - prevOnSeg).lengthSq() < kEpsilon * kEpsilon) {
            break;
        }
    }

    float minDistSq = Vec3f(closestOnAABB - closestOnSeg).lengthSq();

    if (minDistSq > _shapeA.radius * _shapeA.radius) {
        return false;
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    float dist        = std::sqrt(minDistSq);
    Vec3f diff        = closestOnAABB - closestOnSeg;
    Vec3f normal      = diff.normalize();
    float penetration = _shapeA.radius - dist;

    bool aIsPushBack  = _bInfo->GetPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack  = _aInfo->GetPushBackType() != CollisionPushBackType::None;
    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    Vec3f collPoint = closestOnSeg + normal * _shapeA.radius;

    CollisionPushBackInfo::Info aInfo;
    aInfo.pushBackType   = _bInfo->GetPushBackType();
    aInfo.collPoint      = collPoint;
    aInfo.collFaceNormal = normal;
    aInfo.collVec        = normal * penetration * overlapRate;
    _aInfo->AddCollisionInfo(_handleB, aInfo);

    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType   = _aInfo->GetPushBackType();
    bInfo.collPoint      = collPoint;
    bInfo.collFaceNormal = -normal;
    bInfo.collVec        = -normal * penetration * overlapRate;
    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

/// <summary>
/// AABB vs Capsule の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::AABB& _shapeA, const Bounds::Capsule& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Capsule, Bounds::AABB>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

/// <summary>
/// Capsule vs OBB の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::Capsule& _shapeA, const Bounds::OBB& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    // 反復最近接点法：線分上の点とOBB上の点を交互に更新して収束させる
    Vec3f closestOnSeg = _shapeA.segment.Center();
    Vec3f closestOnOBB = ClosestPointOnOBB(closestOnSeg, _shapeB);

    constexpr int kMaxIterations = 8;
    for (int i = 0; i < kMaxIterations; ++i) {
        Vec3f prevOnSeg = closestOnSeg;
        closestOnSeg    = ClosestPointOnSegment(closestOnOBB, _shapeA.segment.start, _shapeA.segment.end);
        closestOnOBB    = ClosestPointOnOBB(closestOnSeg, _shapeB);

        if (Vec3f(closestOnSeg - prevOnSeg).lengthSq() < kEpsilon * kEpsilon) {
            break;
        }
    }

    float minDistSq = Vec3f(closestOnOBB - closestOnSeg).lengthSq();

    if (minDistSq > _shapeA.radius * _shapeA.radius) {
        return false;
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    float dist        = std::sqrt(minDistSq);
    Vec3f diff        = closestOnOBB - closestOnSeg;
    Vec3f normal = diff.normalize();
    float penetration = _shapeA.radius - dist;

    bool aIsPushBack  = _bInfo->GetPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack  = _aInfo->GetPushBackType() != CollisionPushBackType::None;
    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    Vec3f collPoint = closestOnSeg + normal * _shapeA.radius;

    CollisionPushBackInfo::Info aInfo;
    aInfo.pushBackType   = _bInfo->GetPushBackType();
    aInfo.collPoint      = collPoint;
    aInfo.collFaceNormal = normal;
    aInfo.collVec        = normal * penetration * overlapRate;
    _aInfo->AddCollisionInfo(_handleB, aInfo);

    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType   = _aInfo->GetPushBackType();
    bInfo.collPoint      = collPoint;
    bInfo.collFaceNormal = -normal;
    bInfo.collVec        = -normal * penetration * overlapRate;
    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

/// <summary>
/// OBB vs Capsule の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::OBB& _shapeA, const Bounds::Capsule& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Capsule, Bounds::OBB>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

/// <summary>
/// Capsule vs Segment の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::Capsule& _shapeA, const Bounds::Segment& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    Vec3f closest1, closest2;
    ClosestPointsBetweenSegments(
        _shapeA.segment.start, _shapeA.segment.end,
        _shapeB.start, _shapeB.end,
        closest1, closest2);

    Vec3f diff   = closest2 - closest1;
    float distSq = diff.lengthSq();

    if (distSq > _shapeA.radius * _shapeA.radius) {
        return false;
    }

    if (!_aInfo || !_bInfo) {
        return true;
    }

    float dist        = std::sqrt(distSq);
    Vec3f normal = diff.normalize();
    float penetration = _shapeA.radius - dist;

    bool aIsPushBack  = _bInfo->GetPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack  = _aInfo->GetPushBackType() != CollisionPushBackType::None;
    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    Vec3f collPoint = closest1 + normal * _shapeA.radius;

    CollisionPushBackInfo::Info aInfo;
    aInfo.pushBackType   = _bInfo->GetPushBackType();
    aInfo.collPoint      = collPoint;
    aInfo.collFaceNormal = normal;
    aInfo.collVec        = normal * penetration * overlapRate;
    _aInfo->AddCollisionInfo(_handleB, aInfo);

    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType   = _aInfo->GetPushBackType();
    bInfo.collPoint      = collPoint;
    bInfo.collFaceNormal = -normal;
    bInfo.collVec        = -normal * penetration * overlapRate;
    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

/// <summary>
/// Segment vs Capsule の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::Segment& _shapeA, const Bounds::Capsule& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Capsule, Bounds::Segment>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

#pragma endregion

} // namespace OriGine
