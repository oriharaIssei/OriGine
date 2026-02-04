#pragma once

#include "component/collision/collider/base/Collider.h"
/// math
#include "bounds/Segment.h"

namespace OriGine {

/// <summary>
/// Segmentコライダー（線分）
/// </summary>
class SegmentCollider
    : public Collider<Bounds::Segment> {
    friend void to_json(nlohmann::json& _json, const SegmentCollider& _s);
    friend void from_json(const nlohmann::json& _json, SegmentCollider& _s);

public:
    SegmentCollider() : Collider<Bounds::Segment>() {}
    ~SegmentCollider() {}

    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;

    void CalculateWorldShape() override;

    Bounds::AABB ToWorldAABB() const override;

public: // accessor
    const Vec3f& GetLocalStart() const { return shape_.start; }
    void SetLocalStart(const Vec3f& _start) { shape_.start = _start; }
    const Vec3f& GetLocalEnd() const { return shape_.end; }
    void SetLocalEnd(const Vec3f& _end) { shape_.end = _end; }

    const Vec3f& GetWorldStart() const { return worldShape_.start; }
    const Vec3f& GetWorldEnd() const { return worldShape_.end; }
};

} // namespace OriGine
