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

public:
    ORIGINE_COMPONENT();

    DirectionalLight() : IComponent() {}
    ~DirectionalLight() {}

    void Initialize(Scene* /*_scene*/, const EntityHandle& /*_owner*/) {}

    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel);

    void Finalize() {}

public:
    bool isActive = true;

    Vec3f color         = {1.f, 1.f, 1.f};
    float intensity     = 0.f;
    Vec3f direction     = {0.f, 0.f, 1.f};
    float angularRadius = 0.0f;

public:
    struct ConstantBuffer {
        Vec3f color; // 12 bytes
        float intensity; // 4 bytes
        Vec3f direction; // 12 bytes
        float angularRadius; // 4 bytes (16バイトアライメント調整用)
        ConstantBuffer& operator=(const DirectionalLight& light) {
            color         = light.color;
            direction     = light.direction;
            intensity     = light.intensity;
            angularRadius = light.angularRadius;
            return *this;
        }
    };
};

// D-1: 保存対象フィールドに Opaque が無いため、表経由でシリアライズする。
// 手書きの to_json/from_json は D-4 で削除済み(ComponentArray 経由以外の呼び出しが
// 無いことを確認済み。docs/todo.html Phase 3 D-4 参照)。
template <>
inline constexpr bool kUsesDescriptorSerialization<DirectionalLight> = true;

} // namespace OriGine
