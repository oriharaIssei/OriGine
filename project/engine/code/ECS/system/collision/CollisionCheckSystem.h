#pragma once

/// parent
#include "system/ISystem.h"

/// stl
#include <vector>

/// collision
#include "SpatialHash.h"

namespace OriGine {

/// <summary>
/// 衝突判定システム
/// </summary>
class CollisionCheckSystem
    : public ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    CollisionCheckSystem();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~CollisionCheckSystem();

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 全体の衝突判定更新
    /// </summary>
    void Update() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

protected:
    /// <summary>
    /// エンティティの包含AABBを計算
    /// </summary>
    Bounds::AABB ComputeEntityAABB(EntityHandle _entity);

    /// <summary>
    /// エンティティペア間の衝突判定を行う
    /// </summary>
    void CheckEntityPair(EntityHandle _aEntity, EntityHandle _bEntity);

protected:
    /// <summary>
    /// 空間ハッシュ
    /// </summary>
    SpatialHash spatialHash_;

    /// <summary>
    /// 衝突候補ペア
    /// </summary>
    std::vector<std::pair<EntityHandle, EntityHandle>> collisionPairs_;

    /// <summary>
    /// エンティティのペアを走査するためのイテレータ
    /// </summary>
    ::std::vector<EntityHandle>::iterator entityItr_;
};

} // namespace OriGine
