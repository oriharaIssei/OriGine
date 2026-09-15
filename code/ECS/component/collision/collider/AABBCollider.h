#pragma once

#include "component/collision/collider/base/Collider.h"

#include "bounds/AABB.h"

namespace OriGine {

/// <summary>
/// AABBコライダー（軸並行境界ボックス）。
/// ローカル空間での中心(center)と各軸の半長(halfSize)を保持し、
/// CalculateWorldShapeでTransformのワールド行列・スケールを適用したものをワールド形状として持つ。
/// </summary>
class AABBCollider
    : public Collider<Bounds::AABB> {
    /// <summary>
    /// AABBColliderの状態をJSONへ書き出す
    /// </summary>
    friend void to_json(nlohmann::json& _json, const AABBCollider& _a);
    /// <summary>
    /// JSONからAABBColliderの状態を復元する
    /// </summary>
    friend void from_json(const nlohmann::json& _json, AABBCollider& _a);

public:
    AABBCollider()
        : Collider<Bounds::AABB>() {}
    ~AABBCollider() {}

    /// <summary>
    /// デバッグ用GUIでAABB形状とTransformのパラメータを編集する
    /// </summary>
    /// <param name="_scene">対象シーン</param>
    /// <param name="_entity">対象エンティティ</param>
    /// <param name="_parentLabel">ImGuiのID衝突を避けるための親ラベル</param>
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel);

    /// <summary>
    /// ローカル形状(shape_)とTransformの現在値から、ワールド空間のAABB(worldShape_)を再計算する
    /// </summary>
    void CalculateWorldShape();

    /// <summary>
    /// ワールド空間のAABBを取得する
    /// </summary>
    /// <returns>広域フェーズ（空間ハッシュ登録など）に使うワールド空間の外接AABB</returns>
    Bounds::AABB ToWorldAABB() const;

public: // accessor
};

} // namespace OriGine
