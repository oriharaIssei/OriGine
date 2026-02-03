#pragma once

#include "component/collision/collider/base/Collider.h"

#include "bounds/AABB.h"

namespace OriGine {

/// <summary>
/// AABBコライダー
/// </summary>
class AABBCollider
    : public Collider<Bounds::AABB> {
    friend void to_json(nlohmann::json& _json, const AABBCollider& _a);
    friend void from_json(const nlohmann::json& _json, AABBCollider& _a);

public:
    AABBCollider()
        : Collider<Bounds::AABB>() {}
    ~AABBCollider() {}

    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;

    void CalculateWorldShape() override;

public: // accessor
};

} // namespace OriGine
