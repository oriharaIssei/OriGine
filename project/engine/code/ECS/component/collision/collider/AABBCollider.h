#pragma once
#include "Collider.h"

#include "bounds/AABB.h"

class AABBCollider
    : public Collider<math::bounds::AABB> {
    friend void to_json(nlohmann::json& _json, const AABBCollider& _a);
    friend void from_json(const nlohmann::json& _json, AABBCollider& _a);

public:
    AABBCollider()
        : Collider<math::bounds::AABB>() {}
    ~AABBCollider() {}

    void Edit(Scene* _scene, Entity* _entity, const std::string& _parentLabel) override;

    void CalculateWorldShape() override;

public: // accessor
    const Vec3f& GetLocalMin() const { return shape_.min_; }
    void SetLocalMin(const Vec3f& _min) { shape_.min_ = _min; }

    const Vec3f& GetLocalMax() const { return shape_.max_; }
    void SetLocalMax(const Vec3f& _max) { shape_.max_ = _max; }

    const Vec3f& GetWorldMin() const { return worldShape_.min_; }
    void SetWorldMin(const Vec3f& _min) { worldShape_.min_ = _min; }
    const Vec3f& GetWorldMax() const { return worldShape_.max_; }
    void SetWorldMax(const Vec3f& _max) { worldShape_.max_ = _max; }

    const Vec3f& GetWorldCenter() const { return (worldShape_.min_ + worldShape_.max_) * 0.5f; }
};
