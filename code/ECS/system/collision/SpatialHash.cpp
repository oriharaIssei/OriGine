#include "SpatialHash.h"

#include <algorithm>
#include <cmath>
#include <set>

namespace OriGine {

SpatialHash::SpatialHash(float _cellSize)
    : cellSize_(_cellSize), inverseCellSize_(1.0f / _cellSize) {}

void SpatialHash::SetCellSize(float _cellSize) {
    cellSize_        = _cellSize;
    inverseCellSize_ = 1.0f / _cellSize;
}

void SpatialHash::Clear() {
    cells_.clear();
    entityCells_.clear();
}

void SpatialHash::Insert(const EntityHandle& _entity, const Bounds::AABB& _aabb) {
    CellKey minCell, maxCell;
    GetCellRange(_aabb, minCell, maxCell);

    std::vector<CellKey>& cellList = entityCells_[_entity];
    cellList.clear();

    // AABBがカバーする全てのセルに登録
    // 1つのセルにだけ入れると、セルの境界をまたぐオブジェクトが隣のセルから見えなくなり
    // 衝突を取りこぼす。跨る全セルに重複して登録することでどのセルからでも発見できる
    // (その代償として、同じペアが複数のセルで見つかるためGetAllPairs側で重複排除が必要)
    for (int32_t z = minCell.z; z <= maxCell.z; ++z) {
        for (int32_t y = minCell.y; y <= maxCell.y; ++y) {
            for (int32_t x = minCell.x; x <= maxCell.x; ++x) {
                CellKey key{x, y, z};
                cells_[key].push_back(_entity);
                cellList.push_back(key);
            }
        }
    }
}

void SpatialHash::Query(const Bounds::AABB& _aabb, std::unordered_set<EntityHandle>& _outEntities) const {
    CellKey minCell, maxCell;
    GetCellRange(_aabb, minCell, maxCell);

    for (int32_t z = minCell.z; z <= maxCell.z; ++z) {
        for (int32_t y = minCell.y; y <= maxCell.y; ++y) {
            for (int32_t x = minCell.x; x <= maxCell.x; ++x) {
                CellKey key{x, y, z};
                auto it = cells_.find(key);
                if (it != cells_.end()) {
                    for (EntityHandle entity : it->second) {
                        _outEntities.insert(entity);
                    }
                }
            }
        }
    }
}

/// <summary>
/// 衝突している可能性のあるエンティティのペアを列挙する(ブロードフェーズ)。
/// </summary>
/// <remarks>
/// 全エンティティを総当たりするとO(n^2)になるため、同じセルに入っているもの同士だけを
/// 候補として返す。ここで返るのはあくまで「近くにある」ペアであり、
/// 実際に接触しているかは呼び出し側の詳細判定(ナローフェーズ)で確かめる必要がある。
/// </remarks>
void SpatialHash::GetAllPairs(std::vector<std::pair<EntityHandle, EntityHandle>>& _outPairs) const {
    _outPairs.clear();

    // 重複チェック用セット（EntityHandle同士のペア）
    // Insertで大きなオブジェクトは複数セルに登録されるため、同じペアが別々のセルで
    // 何度も見つかる。重複したまま返すと衝突応答が多重に適用されてしまう
    std::set<std::pair<EntityHandle, EntityHandle>> checkedPairs;

    for (const auto& [cellKey, entities] : cells_) {
        size_t count = entities.size();
        for (size_t i = 0; i < count; ++i) {
            for (size_t j = i + 1; j < count; ++j) {
                EntityHandle a = entities[i];
                EntityHandle b = entities[j];

                // 順序を正規化
                // (A,B)と(B,A)は同じ組み合わせだが、そのままでは別のキーとして扱われ
                // 重複排除が効かない。必ず小さい方を先にして1通りの表現に揃える
                if (b < a) {
                    std::swap(a, b);
                }
                auto pairKey = std::make_pair(a, b);

                // 未チェックのペアのみ追加
                if (checkedPairs.insert(pairKey).second) {
                    _outPairs.emplace_back(a, b);
                }
            }
        }
    }
}

/// <summary>
/// ワールド座標を、それが属するセルの整数インデックスに変換する。
/// </summary>
/// <remarks>
/// キャストではなくstd::floorを使うのが要点。intへのキャストは0方向への切り捨てになるため、
/// -0.5も0.5も同じセル0に落ちてしまい、原点をまたぐセルだけが2倍の幅を持つことになる。
/// floorなら負の側も正しく下方向に丸められ、全セルが等幅になる。
/// 除算ではなく逆数の乗算にしているのは、毎フレーム大量に呼ばれるため。
/// </remarks>
CellKey SpatialHash::PositionToCell(const Vec3f& _position) const {
    return CellKey{
        static_cast<int32_t>(std::floor(_position[X] * inverseCellSize_)),
        static_cast<int32_t>(std::floor(_position[Y] * inverseCellSize_)),
        static_cast<int32_t>(std::floor(_position[Z] * inverseCellSize_))};
}

void SpatialHash::GetCellRange(const Bounds::AABB& _aabb, CellKey& _min, CellKey& _max) const {
    Vec3f minPos = _aabb.Min();
    Vec3f maxPos = _aabb.Max();

    _min = PositionToCell(minPos);
    _max = PositionToCell(maxPos);
}

} // namespace OriGine
