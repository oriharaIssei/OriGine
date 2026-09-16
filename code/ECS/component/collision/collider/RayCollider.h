#pragma once

#include "component/collision/collider/base/Collider.h"
/// math
#include "bounds/Ray.h"

namespace OriGine {

/// <summary>
/// Rayコライダー（半直線）。
/// ローカル空間の原点(origin)と正規化済み方向(direction)で形状を表す。
/// </summary>
class RayCollider
    : public Collider<Bounds::Ray> {
    /// <summary>
    /// RayColliderの状態をJSONへ書き出す
    /// </summary>
    friend void to_json(nlohmann::json& _json, const RayCollider& _r);
    /// <summary>
    /// JSONからRayColliderの状態を復元する
    /// </summary>
    friend void from_json(const nlohmann::json& _json, RayCollider& _r);

public:
    RayCollider() : Collider<Bounds::Ray>() {}
    ~RayCollider() {}

    /// <summary>
    /// デバッグ用GUIでRay形状とTransformのパラメータを編集する
    /// </summary>
    /// <param name="_scene">対象シーン</param>
    /// <param name="_entity">対象エンティティ</param>
    /// <param name="_parentLabel">ImGuiのID衝突を避けるための親ラベル</param>
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel);

    // CalculateWorldShape()はここにはない。計算式はRayCollider.cppの
    // Bounds::CalculateWorldShape(const Ray&, Ray&, const Transform&)に移した(Phase 3 3B)。

    /// <summary>
    /// ワールド空間のAABBを取得する
    /// </summary>
    /// <returns>広域フェーズ（空間ハッシュ登録など）に使う、有限距離で打ち切ったワールド空間の外接AABB</returns>
    Bounds::AABB ToWorldAABB() const;

public: // accessor
    const Vec3f& GetLocalOrigin() const { return shape_.origin; }
    void SetLocalOrigin(const Vec3f& _origin) { shape_.origin = _origin; }
    const Vec3f& GetLocalDirection() const { return shape_.direction; }
    // 方向ベクトルは長さ1であることが前提のため、設定時に必ず正規化する
    void SetLocalDirection(const Vec3f& _direction) { shape_.direction = _direction.normalize(); }

    const Vec3f& GetWorldOrigin() const { return worldShape_.origin; }
    const Vec3f& GetWorldDirection() const { return worldShape_.direction; }
};

} // namespace OriGine
