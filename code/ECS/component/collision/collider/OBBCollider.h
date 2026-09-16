#pragma once

#include "component/collision/collider/base/Collider.h"
/// math
#include "bounds/OBB.h"

namespace OriGine {

/// <summary>
/// OBBコライダー（有向境界ボックス）。
/// ローカル空間の中心(center_)・各軸の半長(halfSize_)・回転(orientations_)で形状を表す。
/// </summary>
class OBBCollider
    : public Collider<Bounds::OBB> {
    /// <summary>
    /// OBBColliderの状態をJSONへ書き出す
    /// </summary>
    friend void to_json(nlohmann::json& _json, const OBBCollider& _o);
    /// <summary>
    /// JSONからOBBColliderの状態を復元する
    /// </summary>
    friend void from_json(const nlohmann::json& _json, OBBCollider& _o);

public:
    OBBCollider() : Collider<Bounds::OBB>() {}
    ~OBBCollider() {}

    /// <summary>
    /// デバッグ用GUIでOBB形状とTransformのパラメータを編集する
    /// </summary>
    /// <param name="_scene">対象シーン</param>
    /// <param name="_entity">対象エンティティ</param>
    /// <param name="_parentLabel">ImGuiのID衝突を避けるための親ラベル</param>
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel);

    // CalculateWorldShape()はここにはない。計算式はOBBCollider.cppの
    // Bounds::CalculateWorldShape(const OBB&, OBB&, const Transform&)に移した(Phase 3 3B)。

    /// <summary>
    /// ワールド空間のAABBを取得する
    /// </summary>
    /// <returns>広域フェーズ（空間ハッシュ登録など）に使う、OBBを軸並行境界に投影したAABB</returns>
    Bounds::AABB ToWorldAABB() const;

public: // accessor
    const Vec3f& GetLocalCenter() const { return shape_.center_; }
    void SetLocalCenter(const Vec3f& _center) { shape_.center_ = _center; }
    void SetLocalCenter(float _val, int _axis) { shape_.center_[_axis] = _val; }
    const Vec3f& GetLocalHalfSize() const { return shape_.halfSize_; }
    void SetLocalHalfSize(const Vec3f& _halfSize) { shape_.halfSize_ = _halfSize; }
    void SetLocalHalfSize(float _val, int _axis) { shape_.halfSize_[_axis] = _val; }
    const Vec3f& GetWorldCenter() const { return worldShape_.center_; }
    const Vec3f& GetWorldHalfSize() const { return worldShape_.halfSize_; }

    const Orientation& GetWorldOrientations() const { return worldShape_.orientations_; }
    const Orientation& GetLocalOrientations() const { return shape_.orientations_; }
    void SetRotate(const Quaternion& _rotate) { shape_.orientations_.SetRotation(_rotate); }
};

} // namespace OriGine
