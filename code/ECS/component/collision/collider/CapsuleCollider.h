#pragma once

#include "component/collision/collider/base/Collider.h"
/// math
#include "bounds/Capsule.h"

namespace OriGine {

/// <summary>
/// Capsuleコライダー。
/// ローカル空間の線分(start-end)と半径(radius)で形状を表し、線分の周囲radius分を膨らませた形になる。
/// </summary>
class ORIGINE_API CapsuleCollider
    : public Collider<Bounds::Capsule> {
    /// <summary>
    /// CapsuleColliderの状態をJSONへ書き出す
    /// </summary>
    friend ORIGINE_API void to_json(nlohmann::json& _json, const CapsuleCollider& _c);
    /// <summary>
    /// JSONからCapsuleColliderの状態を復元する
    /// </summary>
    friend ORIGINE_API void from_json(const nlohmann::json& _json, CapsuleCollider& _c);

public:
    CapsuleCollider() : Collider<Bounds::Capsule>() {}
    ~CapsuleCollider() {}

    /// <summary>
    /// デバッグ用GUIでCapsule形状とTransformのパラメータを編集する
    /// </summary>
    /// <param name="_scene">対象シーン</param>
    /// <param name="_entity">対象エンティティ</param>
    /// <param name="_parentLabel">ImGuiのID衝突を避けるための親ラベル</param>
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel);

    // CalculateWorldShape()はここにはない。計算式はCapsuleCollider.cppの
    // Bounds::CalculateWorldShape(const Capsule&, Capsule&, const Transform&)に移した(Phase 3 3B)。

    /// <summary>
    /// ワールド空間のAABBを取得する
    /// </summary>
    /// <returns>広域フェーズ（空間ハッシュ登録など）に使うワールド空間の外接AABB</returns>
    Bounds::AABB ToWorldAABB() const;

public: // accessor
    const Vec3f& GetLocalStart() const { return shape_.segment.start; }
    void SetLocalStart(const Vec3f& _start) { shape_.segment.start = _start; }
    const Vec3f& GetLocalEnd() const { return shape_.segment.end; }
    void SetLocalEnd(const Vec3f& _end) { shape_.segment.end = _end; }
    float GetLocalRadius() const { return shape_.radius; }
    void SetLocalRadius(float _radius) { shape_.radius = _radius; }

    const Vec3f& GetWorldStart() const { return worldShape_.segment.start; }
    const Vec3f& GetWorldEnd() const { return worldShape_.segment.end; }
    float GetWorldRadius() const { return worldShape_.radius; }
};

} // namespace OriGine
