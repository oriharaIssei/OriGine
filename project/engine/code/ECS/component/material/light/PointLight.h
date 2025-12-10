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
/// Point Light
/// </summary>
struct PointLight
    : public IComponent {
    friend void to_json(nlohmann::json& j, const PointLight& l);
    friend void from_json(const nlohmann::json& j, PointLight& l);

public:
    PointLight() {}
    ~PointLight() {}

    void Initialize([[maybe_unused]] Entity* _entity) override {}
    void Edit(Scene* _scene, Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;

    void Finalize() override {}

public:
    bool isActive_ = true;

    Vec3f color_     = {1.f, 1.f, 1.f};
    Vec3f pos_       = {0.f, 0.f, 0.f};
    float intensity_ = 0.f;
    float radius_    = 0.f;
    float decay_     = 0.f;

public:
    struct ConstantBuffer {
        Vec3f color; // 12 bytes
        float intensity; // 4 bytes to align to 16 bytes
        Vec3f pos; // 12 bytes
        float radius; // 4 bytes
        float decay; // 4 bytes
        float padding[3];
        ConstantBuffer& operator=(const PointLight& light) {
            color     = light.color_;
            pos       = light.pos_;
            intensity = light.intensity_;
            radius    = light.radius_;
            decay     = light.decay_;
            return *this;
        }
    };
};

} // namespace OriGine
