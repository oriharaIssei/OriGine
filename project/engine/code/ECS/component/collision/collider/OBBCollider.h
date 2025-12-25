#pragma once

#include "Collider.h"
/// math
#include "bounds/OBB.h"

namespace OriGine {

class OBBCollider
    : public Collider<Bounds::OBB> {
    friend void to_json(nlohmann::json& _json, const OBBCollider& _o);
    friend void from_json(const nlohmann::json& _json, OBBCollider& _o);

public:
    OBBCollider() : Collider<Bounds::OBB>() {}
    ~OBBCollider() {}

    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;
    void CalculateWorldShape() override;

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
