#pragma once

/// stl
#include <string>

/// engine
// ecs
#include "component/IComponent.h"

/// math
#include "math/Vector3.h"
#include "math/Vector4.h"

namespace OriGine {

/// <summary>
/// DirectionalLight
/// </summary>
struct DirectionalLight
    : public IComponent {

    // to_json, from_json を friend として宣言
    friend void to_json(nlohmann::json& j, const DirectionalLight& l);
    friend void from_json(const nlohmann::json& j, DirectionalLight& l);

public:
    DirectionalLight() : IComponent() {}
    ~DirectionalLight() {}

    void Initialize(Scene* /*_scene*/, EntityHandle /*_owner*/) override {}

    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;

    void Finalize() override {}

public:
    bool isActive_ = true;

    Vec3f color_     = {1.f, 1.f, 1.f};
    float intensity_ = 0.f;
    Vec3f direction_ = {0.f, 0.f, 1.f};

public:
    struct ConstantBuffer {
        Vec3f color; // 12 bytes
        float intensity; // 4 bytes
        Vec3f direction; // 12 bytes
        ConstantBuffer& operator=(const DirectionalLight& light) {
            color     = light.color_;
            direction = light.direction_;
            intensity = light.intensity_;
            return *this;
        }
    };

};

} // namespace OriGine
