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

};
