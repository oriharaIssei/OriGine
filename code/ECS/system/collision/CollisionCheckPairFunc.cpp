#include "CollisionCheckPairFunc.h"

/// ECS
#include "component/physics/Rigidbody.h"

/// math
#include "math/MathEnv.h"
#include "math/Vector3.h"

namespace OriGine {

/// <summary>
/// Sphere vs Sphere の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::Sphere& _shapeA, const Bounds::Sphere& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {

    Vec3f distance = (_shapeA.center_) - (_shapeB.center_);

    // 衝突を判定
    if (distance.lengthSq() >= (_shapeA.radius_ + _shapeB.radius_) * (_shapeA.radius_ + _shapeB.radius_)) {
        return false;
    }

    // 情報を収集するかしないか
    // infoが両方ともnullptrなら何もしない
    if (!_aInfo || !_bInfo) {
        return true;
    }

    bool aIsPushBack = _bInfo->GetPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack = _aInfo->GetPushBackType() != CollisionPushBackType::None;

    // どちらも押し戻しなら半分ずつ押し戻す
    float overlapRate = 1.f / (static_cast<float>(aIsPushBack) + static_cast<float>(bIsPushBack));

    Vec3f collNormal      = distance.normalize();
    float overlapDistance = (_shapeA.radius_ + _shapeB.radius_) - distance.length();

    // 衝突情報の登録
    CollisionPushBackInfo::Info aInfo;
    aInfo.pushBackType   = _bInfo->GetPushBackType();
    aInfo.collVec        = collNormal * overlapDistance * overlapRate;
    aInfo.collFaceNormal = collNormal;
    aInfo.collPoint      = _shapeA.center_ + aInfo.collVec.normalize() * _shapeA.radius_;
    _aInfo->AddCollisionInfo(_handleB, aInfo);

    // 衝突情報の登録
    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType   = _aInfo->GetPushBackType();
    bInfo.collVec        = -collNormal * overlapDistance * overlapRate;
    bInfo.collFaceNormal = -collNormal;
    bInfo.collPoint      = _shapeB.center_ + bInfo.collVec.normalize() * _shapeB.radius_;
    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

/// <summary>
/// AABB vs Sphere の衝突判定の実装
/// </summary>
/// <remarks>
/// 球がRigidbodyを持つ場合は連続衝突判定(スウィープ)、持たない場合は
/// その瞬間の位置だけを見る離散判定を行う。
///
/// スウィープが必要なのは、高速に動く球が1フレームで薄い壁を飛び越えてしまい、
/// 前後どちらの位置でも重なっていない＝すり抜けが起きるため。
/// 前フレーム位置から現在位置までの移動を線分とみなし、その途中で当たったかを調べる。
///
/// このとき、AABBを球の半径だけ全方向へ膨らませておくと、
/// 「球とAABBの交差」を「球の中心(点)の移動線分と膨らませたAABBの交差」に置き換えられる
/// (ミンコフスキー和の考え方)。これにより判定がレイ vs AABB に単純化される。
/// </remarks>
template <>
bool CheckCollisionPair(Scene* _scene, const EntityHandle& _aabbEntity, const EntityHandle& _sphereEntity, const Bounds::AABB& _aabb, const Bounds::Sphere& _sphere, CollisionPushBackInfo* _aabbInfo, CollisionPushBackInfo* _sphereInfo) {
    Vec3f sphereCenter = _sphere.center_;
    Vec3f closest      = {0.f, 0.f, 0.f};
    Vec3f distance     = {0.f, 0.f, 0.f};
    Vec3f normal(0.f, 0.f, 0.f);
    Vec3f sphereCollVec = {0.f, 0.f, 0.f};

    Vec3f aabbMin = _aabb.Min();
    Vec3f aabbMax = _aabb.Max();

    bool isCollided = false;

    Rigidbody* sphereRigidbody = _scene->GetComponent<Rigidbody>(_sphereEntity);
    bool useSwept              = false;
    if (sphereRigidbody) {
        Vec3f velo = sphereRigidbody->GetRealVelocity();
        useSwept   = true;

        if (useSwept) {
            // 交差を判定
            // 速度を引くことで前フレームの位置を復元する(prePos→sphereCenterが今フレームの移動線分)
            Vec3f prePos = sphereCenter - velo;

            // AABBを球の半径分だけ膨らませ、球の判定を「中心点の移動」の判定に置き換える
            aabbMin -= Vec3f(_sphere.radius_, _sphere.radius_, _sphere.radius_);
            aabbMax += Vec3f(_sphere.radius_, _sphere.radius_, _sphere.radius_);

            // スラブ法。AABBを「3組の平行な板(スラブ)の共通部分」とみなし、
            // 軸ごとに線分が板に入る時刻t1と出る時刻t2を求める。
            // 全軸の「入る時刻の最大値」(t) が「出る時刻の最小値」(tMax) を超えたら、
            // 3枚の板を同時に貫いている区間が存在しない＝交差していない。
            // tは0〜1の比率で、そのまま移動のどの時点で当たったかを表す
            int32_t axis = 0;
            int32_t sign = 0;
            float t      = -FLT_MAX;
            float tMax   = FLT_MAX;
            for (int32_t i = 0; i < 3; ++i) {
                if (std::abs(velo[i]) < kEpsilon) {
                    // 並行
                    // この軸方向に動いていない＝板に対して平行に進んでいるため、
                    // 出入りの時刻が計算できない(下の除算がゼロ除算になる)。
                    // 平行な場合は「最初から板の内側にいるか」だけで判断でき、
                    // 外にいるなら永久に交差しないので非衝突として打ち切る
                    if (prePos[i] < aabbMin[i] || prePos[i] > aabbMax[i]) {
                        useSwept = false;
                        break;
                    }
                } else {
                    // ood = one over direction. 3回使う除算を1回にまとめる
                    float ood = 1.0f / velo[i];
                    float t1  = (aabbMin[i] - prePos[i]) * ood;
                    float t2  = (aabbMax[i] - prePos[i]) * ood;
                    // 必ず、t1 が小さいようにする
                    // 負の方向へ進んでいる場合はmin側とmax側で入口と出口が逆転するため、
                    // 入替えてt1を必ず「入る時刻」、t2を「出る時刻」に揃える
                    if (t1 > t2) {
                        std::swap(t1, t2);
                    }
                    if (t1 > t) { // 各軸のAABB への侵入時間が最も遅いものを採用
                        t    = t1;
                        axis = i;
                        sign = velo[i] > 0 ? -1 : 1;
                    }
                    if (t2 < tMax) {
                        tMax = t2;
                    }
                    if (t > tMax) {
                        useSwept = false;
                        break;
                    }
                }
            }

            // tが0〜1に収まる場合だけ、この1フレームの移動区間内で衝突したことになる。
            // 負なら前フレームより過去、1超なら次フレーム以降の交差なので今回は当たっていない
            isCollided = t >= 0 && t <= 1.f && useSwept;
            if (isCollided) {
                // 各軸のAABB への侵入時間が最も遅いものを採用
                Vec3f collPoint = prePos + velo * t;

                normal[axis] = static_cast<float>(sign);

                sphereCollVec[axis] += collPoint[axis] - _sphere.center_[axis]; // current から衝突点までのベクトルを加味
            } else {
                // スウィープでは当たらなかった場合、下の離散判定にフォールバックする
                useSwept = false;
                // 膨らませたままだと離散判定が実際より大きな箱で行われてしまうため、
                // AABB の min,max を通常仕様に戻す
                aabbMin = _aabb.Min();
                aabbMax = _aabb.Max();
                if (t == 0) {
                    sphereCenter = prePos;
                }
            }
        }
    }

    if (!useSwept) {
        // AABBの最近接点を求める
        closest = {
            std::clamp(sphereCenter[X], aabbMin[X], aabbMax[X]),
            std::clamp(sphereCenter[Y], aabbMin[Y], aabbMax[Y]),
            std::clamp(sphereCenter[Z], aabbMin[Z], aabbMax[Z])};

        distance = closest - sphereCenter;

        // 衝突を判定
        if (distance.lengthSq() <= _sphere.radius_ * _sphere.radius_) {
            isCollided = true;

            // 衝突法線の計算
            Vec3f diff = sphereCenter - closest;
            // 埋まっている場合、
            if (diff.lengthSq() <= 0.f) {
                closest = {
                    std::clamp(sphereCenter[X], aabbMin[X], aabbMax[X]),
                    std::clamp(sphereCenter[Y], aabbMin[Y], aabbMax[Y]),
                    std::clamp(sphereCenter[Z], aabbMin[Z], aabbMax[Z])};
            }
            // 最近接点から球の中心へのベクトルのうち、最も成分が大きい軸が
            // 「どの面に当たったか」を表す。AABBは軸に平行なので、法線は必ずいずれかの軸方向になる
            float absX = std::abs(diff[X]);
            float absY = std::abs(diff[Y]);
            float absZ = std::abs(diff[Z]);

            int32_t axis = 0;
            if (absX >= absY && absX >= absZ) {
                axis      = X;
                normal[X] = (diff[X] > 0) ? 1.0f : -1.0f;
            } else if (absY >= absX && absY >= absZ) {
                axis      = Y;
                normal[Y] = (diff[Y] > 0) ? 1.0f : -1.0f;
            } else {
                axis      = Z;
                normal[Z] = (diff[Z] > 0) ? 1.0f : -1.0f;
            }

            sphereCollVec = normal * (_sphere.radius_ - distance.length());
            sphereCollVec[axis] += sphereCenter[axis] - _sphere.center_[axis]; // current から衝突点までのベクトルを加味
        }
    }

    if (!isCollided) {
        return false;
    }

    // 情報を収集するかしないか
    if (!_aabbInfo || !_sphereInfo) {
        return isCollided;
    }
    bool aabbIsPushBack   = _sphereInfo && _sphereInfo->GetPushBackType() != CollisionPushBackType::None;
    bool sphereIsPushBack = _aabbInfo && _aabbInfo->GetPushBackType() != CollisionPushBackType::None;

    // どちらも押し戻しなら半分ずつ押し戻す
    float overlapRate = 1.f / (float(aabbIsPushBack) + float(sphereIsPushBack));

    // 衝突時の処理
    CollisionPushBackInfo::Info aabbInfo;
    aabbInfo.pushBackType   = _sphereInfo->GetPushBackType();
    aabbInfo.collFaceNormal = distance.normalize();
    aabbInfo.collPoint      = _sphere.center_ + closest.normalize() * _sphere.radius_;
    aabbInfo.collVec        = (distance.normalize() * (_sphere.radius_ - distance.length())) * overlapRate;

    _aabbInfo->AddCollisionInfo(_sphereEntity, aabbInfo);

    CollisionPushBackInfo::Info sphereInfo;
    sphereInfo.pushBackType   = _aabbInfo->GetPushBackType();
    sphereInfo.collPoint      = closest;
    sphereInfo.collFaceNormal = normal;
    sphereInfo.collVec        = sphereCollVec;

    _sphereInfo->AddCollisionInfo(_aabbEntity, sphereInfo);

    return isCollided;
}

/// <summary>
/// Sphere vs AABB の衝突判定の実装 (引数を入れ替えて AABB vs Sphere を呼び出す)
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::Sphere& _shapeA, const Bounds::AABB& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::AABB, Bounds::Sphere>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

/// <summary>
/// Sphere vs OBB の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::Sphere& _shapeA, const Bounds::OBB& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    auto& obb    = _shapeB;
    auto& sphere = _shapeA;

    const Vec3f& obbCenter    = obb.center_;
    const Vec3f& sphereCenter = sphere.center_;
    const Vec3f& obbHalfSize  = obb.halfSize_;

    // --- Sphere中心を OBBローカル座標 へ ---
    Vec3f localCenter = sphereCenter - obbCenter;
    auto invRot       = obb.orientations_.rot.Conjugation();
    localCenter       = (MakeMatrix4x4::RotateQuaternion(invRot) * MakeMatrix4x4::Translate(localCenter))[3];

    // --- localAABB との判定 ---
    Vec3f aabbMin = -obbHalfSize;
    Vec3f aabbMax = obbHalfSize;

    // 最近接点
    Vec3f closest = {
        std::clamp(localCenter[X], aabbMin[X], aabbMax[X]),
        std::clamp(localCenter[Y], aabbMin[Y], aabbMax[Y]),
        std::clamp(localCenter[Z], aabbMin[Z], aabbMax[Z])};

    Vec3f diff   = localCenter - closest;
    float distSq = diff.lengthSq();

    if (distSq > sphere.radius_ * sphere.radius_) {
        return false; // 衝突なし
    }

    // --- 衝突ベクトルをローカルで計算 ---
    float dist = std::sqrt(distSq);
    Vec3f normal;
    if (dist > kEpsilon) {
        normal = diff / dist;
    } else {
        normal = axisX; // 球がAABB内部に完全に埋まってるときのfallback
    }

    float penetration  = sphere.radius_ - dist;
    Vec3f localCollVec = normal * penetration;

    Vec3f localCollPoint = closest; // 最近接点を衝突点とする

    // --- ワールドに戻す ---
    auto rotMat          = MakeMatrix4x4::RotateQuaternion(obb.orientations_.rot);
    Vec3f worldCollVec   = localCollVec * rotMat;
    Vec3f worldCollPoint = obbCenter + (localCollPoint * rotMat);

    // --- PushBack 情報を登録 ---
    if (!_aInfo || !_bInfo) {
        return true;
    }
    bool aIsPushBack = _aInfo->GetPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack = _bInfo->GetPushBackType() != CollisionPushBackType::None;

    // どちらも押し戻しなら半分ずつ押し戻す
    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    Vec3f direction = worldCollVec.normalize();
    float length    = worldCollVec.length();

    CollisionPushBackInfo::Info aInfo;
    aInfo.pushBackType = _bInfo->GetPushBackType();
    aInfo.collVec      = direction * (length * overlapRate);
    aInfo.collPoint    = worldCollPoint;
    _aInfo->AddCollisionInfo(_handleB, aInfo);

    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType = _aInfo->GetPushBackType();
    bInfo.collVec      = direction * -(length * overlapRate);
    bInfo.collPoint    = worldCollPoint;
    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

/// <summary>
/// OBB vs Sphere の衝突判定の実装 (引数を入れ替えて Sphere vs OBB を呼び出す)
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::OBB& _shapeA, const Bounds::Sphere& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    return CheckCollisionPair<Bounds::Sphere, Bounds::OBB>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
};

/// <summary>
/// AABB vs AABB の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::AABB& _shapeA, const Bounds::AABB& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {

    Vec3f aabbAMin = _shapeA.Min();
    Vec3f aabbAMax = _shapeA.Max();
    Vec3f aabbBMin = _shapeB.Min();
    Vec3f aabbBMax = _shapeB.Max();

    // 分離軸判定の最も単純な形。AABBは軸に平行なので、X/Y/Zのどれか1軸でも
    // 区間が重なっていなければ、その時点で衝突していないと確定できる。
    // 大半の組み合わせはここで弾かれるため、重い計算の前に早期リターンする
    if (aabbAMax[X] < aabbBMin[X] || aabbAMin[X] > aabbBMax[X]) {
        return false;
    }
    if (aabbAMax[Y] < aabbBMin[Y] || aabbAMin[Y] > aabbBMax[Y]) {
        return false;
    }
    if (aabbAMax[Z] < aabbBMin[Z] || aabbAMin[Z] > aabbBMax[Z]) {
        return false;
    }

    if (!_aInfo || !_bInfo) {
        return true; // 衝突情報がない場合は何もしない
    }

    // 各軸ごとの重なり幅を計算
    float overlapMinX = (std::max)(aabbAMin[X], aabbBMin[X]);
    float overlapMaxX = (std::min)(aabbAMax[X], aabbBMax[X]);

    float overlapMinY = (std::max)(aabbAMin[Y], aabbBMin[Y]);
    float overlapMaxY = (std::min)(aabbAMax[Y], aabbBMax[Y]);

    float overlapMinZ = (std::max)(aabbAMin[Z], aabbBMin[Z]);
    float overlapMaxZ = (std::min)(aabbAMax[Z], aabbBMax[Z]);

    // 各軸ごとの重なり区間
    float overlapX = overlapMaxX - overlapMinX;
    float overlapY = overlapMaxY - overlapMinY;
    float overlapZ = overlapMaxZ - overlapMinZ;

    // 最小の重なり軸を探す
    // 重なりが最も浅い軸へ押し戻すのが、最も移動量が少なくて済む解決方法
    // (最小移動ベクトル / MTV)。深い軸へ押し出すと、箱を突き抜けて反対側へ
    // 飛び出したように見えてしまう
    float minOverlap = overlapX;
    int axis         = X;
    if (overlapY < minOverlap) {
        minOverlap = overlapY;
        axis       = Y;
    }
    if (overlapZ < minOverlap) {
        minOverlap = overlapZ;
        axis       = Z;
    }

    bool aIsPushBack = _aInfo->GetPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack = _bInfo->GetPushBackType() != CollisionPushBackType::None;

    // どちらも押し戻しなら半分ずつ押し戻す
    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    // 各AABBの中心&halfSizeを計算
    Vec3f aHalfSize = (aabbAMax - aabbAMin) * 0.5f;
    Vec3f bHalfSize = (aabbBMax - aabbBMin) * 0.5f;
    Vec3f aCenter   = aabbAMin + aHalfSize;
    Vec3f bCenter   = aabbBMin + bHalfSize;

    // 押し出し方向を決定
    // 中心の位置関係で符号を決める。Aの中心がBより手前にあるならAは手前(負方向)へ退く。
    // 中心を基準にしないと、重なりの浅い側ではなく深い側へ押してしまい貫通する
    float dir = (aCenter[axis] < bCenter[axis]) ? -1.0f : 1.0f;

    // collVecを作成
    Vec3f collVec(0, 0, 0);
    collVec[axis] = (minOverlap + kEpsilon) * dir;

    // 重なり部分の中心＝衝突点
    // NOTE: 補間係数に0.5fではなくoverlapRateを使っているため、実際に中心になるのは
    //       双方が押し戻される(overlapRate==0.5)場合だけ。片側のみの場合は1.0となり、
    //       重なり区間の中心ではなくmax側の角が衝突点になる
    Vec3f collPoint = {
        overlapMinX + (overlapMaxX - overlapMinX) * overlapRate,
        overlapMinY + (overlapMaxY - overlapMinY) * overlapRate,
        overlapMinZ + (overlapMaxZ - overlapMinZ) * overlapRate};

    // 衝突時の処理
    CollisionPushBackInfo::Info ainfo;
    ainfo.pushBackType  = _bInfo->GetPushBackType();
    ainfo.collVec       = Vec3f(0, 0, 0);
    ainfo.collVec[axis] = (minOverlap * overlapRate) * dir;

    ainfo.collPoint = collPoint;

    _aInfo->AddCollisionInfo(_handleB, ainfo);

    // 衝突時の処理
    CollisionPushBackInfo::Info bInfo;
    bInfo.pushBackType  = _aInfo->GetPushBackType();
    bInfo.collVec       = Vec3f(0, 0, 0);
    bInfo.collVec[axis] = (minOverlap * overlapRate) * -dir;

    bInfo.collPoint = collPoint;

    _bInfo->AddCollisionInfo(_handleA, bInfo);

    return true;
}

/// <summary>
/// OBB vs OBB の衝突判定の実装(分離軸判定 / SAT)
/// </summary>
/// <remarks>
/// 分離軸定理: 「2つの凸形状が衝突していない ⇔ 両者の射影が重ならない軸が1本でも存在する」。
/// よって候補となる軸すべてに両OBBの頂点を射影し、1本でも隙間が空いていれば非衝突、
/// 全ての軸で重なっていれば衝突と判定できる。
///
/// 候補軸は次の15本。これで漏れなく判定できることが定理として示されている。
///   ・各OBBの面の法線 3本 × 2 = 6本 (面同士がぶつかる場合を捉える)
///   ・両OBBの辺の組み合わせの外積 3×3 = 9本 (辺同士が斜めにぶつかる場合を捉える)
/// 外積の軸は、対応する辺が平行だとゼロベクトルになり軸として使えないため除外する。
///
/// 衝突している場合、重なりが最も小さかった軸が最小移動ベクトル(MTV)の向きになり、
/// その重なり量がめり込みの深さになる。
/// </remarks>
template <>
bool CheckCollisionPair(Scene* /*_scene*/, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::OBB& _shapeA, const Bounds::OBB& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    // === 頂点計算 ===
    auto computeVerts = [](const Bounds::OBB& obb) {
        std::array<Vec3f, 8> verts;
        Vec3f min     = -obb.halfSize_;
        Vec3f max     = obb.halfSize_;
        Matrix4x4 mat = MakeMatrix4x4::Affine({1.f, 1.f, 1.f}, obb.orientations_.rot, obb.center_);
        verts[0]      = TransformVector({min[X], min[Y], min[Z]}, mat);
        verts[1]      = TransformVector({max[X], min[Y], min[Z]}, mat);
        verts[2]      = TransformVector({min[X], max[Y], min[Z]}, mat);
        verts[3]      = TransformVector({max[X], max[Y], min[Z]}, mat);
        verts[4]      = TransformVector({min[X], min[Y], max[Z]}, mat);
        verts[5]      = TransformVector({max[X], min[Y], max[Z]}, mat);
        verts[6]      = TransformVector({min[X], max[Y], max[Z]}, mat);
        verts[7]      = TransformVector({max[X], max[Y], max[Z]}, mat);
        return verts;
    };
    auto vertsA = computeVerts(_shapeA);
    auto vertsB = computeVerts(_shapeB);

    // === 軸候補 ===
    std::vector<Vec3f> axes;
    for (int i = 0; i < 3; i++) {
        axes.push_back(_shapeA.orientations_.axis[i]);
    }
    for (int i = 0; i < 3; i++) {
        axes.push_back(_shapeB.orientations_.axis[i]);
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            // 2つの軸が平行なとき外積はゼロベクトルになる。
            // 軸として意味を成さないうえ、normalizeでゼロ除算するため除外する
            Vec3f crossAxis = _shapeA.orientations_.axis[i].cross(_shapeB.orientations_.axis[j]);
            if (crossAxis.lengthSq() > 1e-6f)
                axes.push_back(crossAxis.normalize());
        }
    }

    // === SAT 判定 & 最小overlap ===
    float minOverlap = FLT_MAX;
    Vec3f minAxis(0, 0, 0);

    // 8頂点を軸へ内積で射影し、その軸上で box が占める区間[min,max]を求める。
    // 凸形状なので、頂点の射影の最小・最大が形状全体の区間と一致する
    auto project = [](const std::array<Vec3f, 8>& verts, const Vec3f& axis, float& outMin, float& outMax) {
        float p = verts[0].dot(axis);
        outMin = outMax = p;
        for (int i = 1; i < 8; i++) {
            float v = verts[i].dot(axis);
            if (v < outMin) {
                outMin = v;
            }
            if (v > outMax) {
                outMax = v;
            }
        }
    };

    for (auto& axisRaw : axes) {
        Vec3f axis = axisRaw.normalize();
        float minA, maxA, minB, maxB;
        project(vertsA, axis, minA, maxA);
        project(vertsB, axis, minB, maxB);
        // 2区間の重なり幅。負なら区間の間に隙間がある＝この軸が分離軸
        float overlap = (std::min)(maxA, maxB) - (std::max)(minA, minB);
        // 分離軸が1本でも見つかれば非衝突が確定するので、残りの軸は調べない
        if (overlap <= 0) {
            return false; // 分離
        }
        if (overlap < minOverlap) {
            minOverlap = overlap;
            minAxis    = axis;
        }
    }

    // === サポート点で法線方向を決定 ===
    auto supportPoint = [](const std::array<Vec3f, 8>& verts, const Vec3f& dir) {
        float maxProj = -FLT_MAX;
        Vec3f best;
        for (auto& v : verts) {
            float p = v.dot(dir);
            if (p > maxProj) {
                maxProj = p;
                best    = v;
            }
        }
        return best;
    };

    // サポート点＝その方向に最も突き出している頂点。互いに相手へ向かって最も
    // 食い込んでいる頂点同士なので、接触箇所の代表点として使える
    Vec3f pA = supportPoint(vertsA, minAxis);
    Vec3f pB = supportPoint(vertsB, -minAxis);

    // 法線の向きを補正
    // 分離軸は外積などから作るため向きが一定せず、AからB・BからAのどちらを向くか分からない。
    // 実際の位置関係(AのサポートからBのサポートへのベクトル)と符号を突き合わせ、
    // 必ずAからBを向くように揃える。ここを誤ると押し戻しが逆向きになり互いを引き寄せてしまう
    Vec3f collNormal = minAxis;
    if (Vec3f(pB - pA).dot(collNormal) < 0) {
        collNormal = -collNormal;
    }

    // penetration
    float penetration = minOverlap;

    // 衝突点 = サポート点の平均
    Vec3f collPoint = (pA + pB) * 0.5f;

    // collVec = 法線 * penetration
    Vec3f collVec = collNormal * penetration;

    // === PushBack 情報 ===
    if (!_aInfo || !_bInfo) {
        return true;
    }
    bool aIsPushBack = _aInfo->GetPushBackType() != CollisionPushBackType::None;
    bool bIsPushBack = _bInfo->GetPushBackType() != CollisionPushBackType::None;

    float overlapRate = 1.f / (float(aIsPushBack) + float(bIsPushBack));

    CollisionPushBackInfo::Info ainfo;
    ainfo.pushBackType = _bInfo->GetPushBackType();
    ainfo.collVec      = -collVec * overlapRate;
    ainfo.collPoint    = collPoint;
    _aInfo->AddCollisionInfo(_handleB, ainfo);

    CollisionPushBackInfo::Info binfo;
    binfo.pushBackType = _aInfo->GetPushBackType();
    binfo.collVec      = collVec * overlapRate;
    binfo.collPoint    = collPoint;
    _bInfo->AddCollisionInfo(_handleA, binfo);

    return true;
}

/// <summary>
/// AABB vs OBB の衝突判定の実装
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::AABB& _shapeA, const Bounds::OBB& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {

    // --- AABB を OBB と同じ形式に変換 ---

    Bounds::OBB aAsOBB;
    aAsOBB.center_       = _shapeA.center;
    aAsOBB.halfSize_     = _shapeA.halfSize;
    aAsOBB.orientations_ = Orientation::Identity();

    // --- OBB vs OBB 判定を使う ---
    return CheckCollisionPair<Bounds::OBB, Bounds::OBB>(_scene, _handleA, _handleB, aAsOBB, _shapeB, _aInfo, _bInfo);
}

/// <summary>
/// OBB vs AABB の衝突判定の実装 (引数を入れ替えて AABB vs OBB を呼び出す)
/// </summary>
template <>
bool CheckCollisionPair(Scene* _scene, const EntityHandle& _handleA, const EntityHandle& _handleB, const Bounds::OBB& _shapeA, const Bounds::AABB& _shapeB, CollisionPushBackInfo* _aInfo, CollisionPushBackInfo* _bInfo) {
    // 順序を入れ替えて再利用
    return CheckCollisionPair<Bounds::AABB, Bounds::OBB>(_scene, _handleB, _handleA, _shapeB, _shapeA, _bInfo, _aInfo);
}

} // namespace OriGine
