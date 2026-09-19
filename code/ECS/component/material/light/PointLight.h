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
    friend void to_json(nlohmann::json& _j, const PointLight& _comp);
    friend void from_json(const nlohmann::json& _j, PointLight& _comp);

public:
    ORIGINE_COMPONENT();

    PointLight() {}
    ~PointLight() {}

    void Initialize(Scene* /*_scene*/, const EntityHandle& /*_owner*/) {}
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel);

    void Finalize() {}

public:
    bool isActive = true;

    Vec3f color         = {1.f, 1.f, 1.f};
    Vec3f pos           = {0.f, 0.f, 0.f};
    float intensity     = 0.f;
    float radius        = 0.f;
    float decay         = 0.f;
    float angularRadius = 0.f;

    /// Transform連動用インデックス（-1 = 連動しない）
    int32_t targetTransformIndex = -1;

public:
    struct ConstantBuffer {
        Vec3f color; // 12 bytes
        float intensity; // 4 bytes to align to 16 bytes
        Vec3f pos; // 12 bytes
        float radius; // 4 bytes
        float decay; // 4 bytes
        float angularRadius;
        float padding[2];
        ConstantBuffer& operator=(const PointLight& _comp) {
            color         = _comp.color;
            pos           = _comp.pos;
            intensity     = _comp.intensity;
            radius        = _comp.radius;
            decay         = _comp.decay;
            angularRadius = _comp.angularRadius;
            return *this;
        }
    };
};

// D-1: 保存対象フィールドに Opaque が無いため、表経由でシリアライズする
// (PointLight.cpp の to_json/from_json は D-4 で表経由に置き換わるまでのフォールバックとして残す)。
template <>
inline constexpr bool kUsesDescriptorSerialization<PointLight> = true;

} // namespace OriGine
