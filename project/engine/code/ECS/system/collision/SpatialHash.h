#pragma once

/// stl
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/// math
#include "bounds/AABB.h"
#include "Vector3.h"

/// ECS
#include "entity/EntityHandle.h"

namespace OriGine {

/// <summary>
/// セルキー（3D座標をハッシュ化）
/// </summary>
struct CellKey {
    int32_t x, y, z;

    bool operator==(const CellKey& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

/// <summary>
/// CellKeyのハッシュ関数
/// </summary>
struct CellKeyHash {
    size_t operator()(const CellKey& key) const {
        // 素数を使った単純なハッシュ
        return static_cast<size_t>(key.x * 73856093) ^ static_cast<size_t>(key.y * 19349663) ^ static_cast<size_t>(key.z * 83492791);
    }
};

/// <summary>
/// 空間ハッシュによる広域フェーズ衝突検出
/// </summary>
class SpatialHash {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="_cellSize">セルのサイズ（オブジェクトの平均サイズの2倍程度を推奨）</param>
    explicit SpatialHash(float _cellSize = 10.0f);
    ~SpatialHash() = default;

    /// <summary>
    /// セルサイズを設定
    /// </summary>
    void SetCellSize(float _cellSize);

    /// <summary>
    /// セルサイズを取得
    /// </summary>
    float GetCellSize() const { return cellSize_; }

    /// <summary>
    /// 全セルをクリア
    /// </summary>
    void Clear();

    /// <summary>
    /// オブジェクトを登録
    /// </summary>
    /// <param name="_entity">エンティティハンドル</param>
    /// <param name="_aabb">オブジェクトのAABB</param>
    void Insert(EntityHandle _entity, const Bounds::AABB& _aabb);

    /// <summary>
    /// 指定AABBと衝突する可能性のあるエンティティを取得
    /// </summary>
    /// <param name="_aabb">検索範囲のAABB</param>
    /// <param name="_outEntities">結果を格納するセット</param>
    void Query(const Bounds::AABB& _aabb, std::unordered_set<EntityHandle>& _outEntities) const;

    /// <summary>
    /// 全ての衝突候補ペアを取得
    /// </summary>
    /// <param name="_outPairs">結果を格納するベクター（pair<EntityA, EntityB>）</param>
    void GetAllPairs(std::vector<std::pair<EntityHandle, EntityHandle>>& _outPairs) const;

    /// <summary>
    /// 登録されているエンティティ数を取得
    /// </summary>
    size_t GetEntityCount() const { return entityCells_.size(); }

    /// <summary>
    /// 使用中のセル数を取得
    /// </summary>
    size_t GetCellCount() const { return cells_.size(); }

private:
    /// <summary>
    /// 座標からセルキーを計算
    /// </summary>
    CellKey PositionToCell(const Vec3f& _position) const;

    /// <summary>
    /// AABBがカバーするセル範囲を取得
    /// </summary>
    void GetCellRange(const Bounds::AABB& _aabb, CellKey& _min, CellKey& _max) const;

private:
    float cellSize_;
    float inverseCellSize_; // 除算を避けるため逆数を保持

    // セルごとのエンティティリスト
    std::unordered_map<CellKey, std::vector<EntityHandle>, CellKeyHash> cells_;

    // エンティティが属するセルのリスト（複数セルに跨る場合あり）
    std::unordered_map<EntityHandle, std::vector<CellKey>> entityCells_;
};

} // namespace OriGine
