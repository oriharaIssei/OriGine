#pragma once
#include "Collider.h"

#include "bounds/OBB.h"

class OBBCollider
    : public Collider<math::bounds::OBB> {
    friend void to_json(nlohmann::json& _json, const OBBCollider& _o);
    friend void from_json(const nlohmann::json& _json, OBBCollider& _o);

public:
    OBBCollider() : Collider<math::bounds::OBB>() {}
    ~OBBCollider() {}

    void Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) override;
    void CalculateWorldShape() override;

public: // accessor
    const Vec3f& getLocalCenter() const { return shape_.center_; }
    void setLocalCenter(const Vec3f& _center) { shape_.center_ = _center; }
    void setLocalCenter(float _val, int _axis) { shape_.center_[_axis] = _val; }
    const Vec3f& getLocalHalfSize() const { return shape_.halfSize_; }
    void setLocalHalfSize(const Vec3f& _halfSize) { shape_.halfSize_ = _halfSize; }
    void setLocalHalfSize(float _val, int _axis) { shape_.halfSize_[_axis] = _val; }
    const Vec3f& getWorldCenter() const { return worldShape_.center_; }
    const Vec3f& getWorldHalfSize() const { return worldShape_.halfSize_; }

    const Orientation& getWorldOrientations() const { return worldShape_.orientations_; }
    const Orientation& getLocalOrientations() const { return shape_.orientations_; }
    void setRotate(const Quaternion& _rotate) { shape_.orientations_.setRotation(_rotate); }
};
