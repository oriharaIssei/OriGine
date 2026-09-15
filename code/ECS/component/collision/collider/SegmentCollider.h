#pragma once

#include "component/collision/collider/base/Collider.h"
/// math
#include "bounds/Segment.h"

namespace OriGine {

/// <summary>
/// Segmentコライダー（線分）。
/// ローカル空間の始点(start)と終点(end)の2点で形状を表す。太さは持たない。
/// </summary>
class SegmentCollider
    : public Collider<Bounds::Segment> {
    /// <summary>
    /// SegmentColliderの状態をJSONへ書き出す
    /// </summary>
    friend void to_json(nlohmann::json& _json, const SegmentCollider& _s);
    /// <summary>
    /// JSONからSegmentColliderの状態を復元する
    /// </summary>
    friend void from_json(const nlohmann::json& _json, SegmentCollider& _s);

public:
    SegmentCollider() : Collider<Bounds::Segment>() {}
    ~SegmentCollider() {}

    /// <summary>
    /// デバッグ用GUIでSegment形状とTransformのパラメータを編集する
    /// </summary>
    /// <param name="_scene">対象シーン</param>
    /// <param name="_entity">対象エンティティ</param>
    /// <param name="_parentLabel">ImGuiのID衝突を避けるための親ラベル</param>
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel);

    /// <summary>
    /// ローカル形状(shape_)とTransformの現在値から、ワールド空間のSegment(worldShape_)を再計算する
    /// </summary>
    void CalculateWorldShape();

    /// <summary>
    /// ワールド空間のAABBを取得する
    /// </summary>
    /// <returns>広域フェーズ（空間ハッシュ登録など）に使うワールド空間の外接AABB</returns>
    Bounds::AABB ToWorldAABB() const;

public: // accessor
    const Vec3f& GetLocalStart() const { return shape_.start; }
    void SetLocalStart(const Vec3f& _start) { shape_.start = _start; }
    const Vec3f& GetLocalEnd() const { return shape_.end; }
    void SetLocalEnd(const Vec3f& _end) { shape_.end = _end; }

    const Vec3f& GetWorldStart() const { return worldShape_.start; }
    const Vec3f& GetWorldEnd() const { return worldShape_.end; }
};

} // namespace OriGine
