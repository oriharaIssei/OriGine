#pragma once

#include "component/collision/collider/base/Collider.h"
/// math
#include "bounds/Capsule.h"

namespace OriGine {

/// <summary>
/// Capsuleコライダー
/// </summary>
class CapsuleCollider
    : public Collider<Bounds::Capsule> {
    friend void to_json(nlohmann::json& _json, const CapsuleCollider& _c);
    friend void from_json(const nlohmann::json& _json, CapsuleCollider& _c);

public:
    CapsuleCollider() : Collider<Bounds::Capsule>() {}
    ~CapsuleCollider() {}

    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;

    void CalculateWorldShape() override;

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
