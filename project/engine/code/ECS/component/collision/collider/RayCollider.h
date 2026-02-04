#pragma once

#include "component/collision/collider/base/Collider.h"
/// math
#include "bounds/Ray.h"

namespace OriGine {

/// <summary>
/// Rayコライダー（半直線）
/// </summary>
class RayCollider
    : public Collider<Bounds::Ray> {
    friend void to_json(nlohmann::json& _json, const RayCollider& _r);
    friend void from_json(const nlohmann::json& _json, RayCollider& _r);

public:
    RayCollider() : Collider<Bounds::Ray>() {}
    ~RayCollider() {}

    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;

    void CalculateWorldShape() override;

    Bounds::AABB ToWorldAABB() const override;

public: // accessor
    const Vec3f& GetLocalOrigin() const { return shape_.origin; }
    void SetLocalOrigin(const Vec3f& _origin) { shape_.origin = _origin; }
    const Vec3f& GetLocalDirection() const { return shape_.direction; }
    void SetLocalDirection(const Vec3f& _direction) { shape_.direction = _direction.normalize(); }

    const Vec3f& GetWorldOrigin() const { return worldShape_.origin; }
    const Vec3f& GetWorldDirection() const { return worldShape_.direction; }
};

} // namespace OriGine
