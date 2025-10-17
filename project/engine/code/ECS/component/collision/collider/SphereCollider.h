#pragma once
#include "Collider.h"

#include "bounds/Sphere.h"

class SphereCollider
    : public Collider<math::bounds::Sphere> {
    friend void to_json(nlohmann::json& _json, const SphereCollider& _s);
    friend void from_json(const nlohmann::json& _json, SphereCollider& _s);

public:
    SphereCollider() : Collider<math::bounds::Sphere>() {}
    ~SphereCollider() {}

    void Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) override;

    void CalculateWorldShape() override;

public: // accessor
    const Vec3f& getLocalCenter() const { return shape_.center_; }
    void setLocalCenter(const Vec3f& _center) { shape_.center_ = _center; }
    const float& getLocalRadius() const { return shape_.radius_; }
    void setLocalRadius(const float& _radius) { shape_.radius_ = _radius; }

    const Vec3f& getWorldCenter() const { return worldShape_.center_; }
    void setWorldCenter(const Vec3f& _center) { worldShape_.center_ = _center; }
    const float& getWorldRadius() const { return worldShape_.radius_; }
    void setWorldRadius(const float& _radius) { worldShape_.radius_ = _radius; }
};
