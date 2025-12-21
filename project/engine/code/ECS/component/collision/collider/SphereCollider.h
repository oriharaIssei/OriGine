#pragma once

#include "Collider.h"
/// math
#include "bounds/Sphere.h"

namespace OriGine {

class SphereCollider
    : public Collider<Bounds::Sphere> {
    friend void to_json(nlohmann::json& _json, const SphereCollider& _s);
    friend void from_json(const nlohmann::json& _json, SphereCollider& _s);

public:
    SphereCollider() : Collider<Bounds::Sphere>() {}
    ~SphereCollider() {}

    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;

    void CalculateWorldShape() override;

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
