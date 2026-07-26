#pragma once

#include "component/collision/collider/base/Collider.h"
/// math
#include "bounds/Sphere.h"

namespace OriGine {

/// <summary>
/// Sphereコライダー。
/// ローカル空間の中心(center_)と半径(radius_)で形状を表す。
/// </summary>
class SphereCollider
    : public Collider<Bounds::Sphere> {
    /// <summary>
    /// SphereColliderの状態をJSONへ書き出す
    /// </summary>
    friend void to_json(nlohmann::json& _json, const SphereCollider& _s);
    /// <summary>
    /// JSONからSphereColliderの状態を復元する
    /// </summary>
    friend void from_json(const nlohmann::json& _json, SphereCollider& _s);

public:
    SphereCollider() : Collider<Bounds::Sphere>() {}
    ~SphereCollider() {}

    /// <summary>
    /// デバッグ用GUIでSphere形状とTransformのパラメータを編集する
    /// </summary>
    /// <param name="_scene">対象シーン</param>
    /// <param name="_entity">対象エンティティ</param>
    /// <param name="_parentLabel">ImGuiのID衝突を避けるための親ラベル</param>
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel) override;

    /// <summary>
    /// ローカル形状(shape_)とTransformの現在値から、ワールド空間のSphere(worldShape_)を再計算する
    /// </summary>
    void CalculateWorldShape() override;

    /// <summary>
    /// ワールド空間のAABBを取得する
    /// </summary>
    /// <returns>広域フェーズ（空間ハッシュ登録など）に使うワールド空間の外接AABB</returns>
    Bounds::AABB ToWorldAABB() const override;

public: // accessor
    const Vec3f& GetLocalCenter() const { return shape_.center_; }
    void SetLocalCenter(const Vec3f& _center) { shape_.center_ = _center; }
    const float& GetLocalRadius() const { return shape_.radius_; }
    void SetLocalRadius(const float& _radius) { shape_.radius_ = _radius; }

    const Vec3f& GetWorldCenter() const { return worldShape_.center_; }
    void SetWorldCenter(const Vec3f& _center) { worldShape_.center_ = _center; }
    const float& GetWorldRadius() const { return worldShape_.radius_; }
    void SetWorldRadius(const float& _radius) { worldShape_.radius_ = _radius; }
};

} // namespace OriGine
