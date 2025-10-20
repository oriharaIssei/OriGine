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
    const Vec3f& getLocalMin() const { return shape_.min_; }
    void setLocalMin(const Vec3f& _min) { shape_.min_ = _min; }

    const Vec3f& getLocalMax() const { return shape_.max_; }
    void setLocalMax(const Vec3f& _max) { shape_.max_ = _max; }

    const Vec3f& getWorldMin() const { return worldShape_.min_; }
    void setWorldMin(const Vec3f& _min) { worldShape_.min_ = _min; }
    const Vec3f& getWorldMax() const { return worldShape_.max_; }
    void setWorldMax(const Vec3f& _max) { worldShape_.max_ = _max; }

    const Vec3f& getWorldCenter() const { return (worldShape_.min_ + worldShape_.max_) * 0.5f; }
};
