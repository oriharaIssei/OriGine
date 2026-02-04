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

void SpatialHash::Insert(EntityHandle _entity, const Bounds::AABB& _aabb) {
    CellKey minCell, maxCell;
    GetCellRange(_aabb, minCell, maxCell);

    std::vector<CellKey>& cellList = entityCells_[_entity];
    cellList.clear();

    // AABBがカバーする全てのセルに登録
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

void SpatialHash::GetAllPairs(std::vector<std::pair<EntityHandle, EntityHandle>>& _outPairs) const {
    _outPairs.clear();

    // 重複チェック用セット（EntityHandle同士のペア）
    std::set<std::pair<EntityHandle, EntityHandle>> checkedPairs;

    for (const auto& [cellKey, entities] : cells_) {
        size_t count = entities.size();
        for (size_t i = 0; i < count; ++i) {
            for (size_t j = i + 1; j < count; ++j) {
                EntityHandle a = entities[i];
                EntityHandle b = entities[j];

                // 順序を正規化
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
