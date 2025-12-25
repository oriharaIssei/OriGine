#pragma once

#include <d3d12.h>
#include <wrl.h>

/// stl
#include <string>

/// engine
#include "component/IComponent.h"

/// math
#include "Vector3.h"
#include "Vector4.h"

namespace OriGine {

/// <summary>
/// Spot Light
/// </summary>
struct SpotLight
    : public IComponent {
    friend void to_json(nlohmann::json& j, const SpotLight& l);
    friend void from_json(const nlohmann::json& j, SpotLight& l);

public:
    SpotLight() {}
    ~SpotLight() {}

    void Initialize(Scene* /*_scene*/, EntityHandle /*_owner*/) override {}

    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;

    void Finalize() override {}

public:
    bool isActive_ = true;

    Vec3f color_           = {1.f, 1.f, 1.f};
    Vec3f pos_             = {0.f, 0.f, 0.f};
    float intensity_       = 0.f;
    Vec3f direction_       = {0.f, 0.f, 1.f};
    float distance_        = 0.f;
    float decay_           = 0.f;
    float cosAngle_        = 0.f;
    float cosFalloffStart_ = 0.f;

public:
    struct ConstantBuffer {
        Vec3f color; // 12 bytes
        float intensity; // 4 bytes
        Vec3f pos; // 12 bytes
        float distance; // 4 bytes
        Vec3f direction; // 12 bytes
        float decay; // 4 bytes
        float cosAngle; // 4 bytes
        float cosFalloffStart; // 4 bytes
        float padding[2]; // 8 bytes (to align to 16 bytes)
        ConstantBuffer& operator=(const SpotLight& light) {
            color           = light.color_;
            pos             = light.pos_;
            intensity       = light.intensity_;
            direction       = light.direction_;
            distance        = light.distance_;
            decay           = light.decay_;
            cosAngle        = light.cosAngle_;
            cosFalloffStart = light.cosFalloffStart_;
            return *this;
        }
    };
};

} // namespace OriGine
