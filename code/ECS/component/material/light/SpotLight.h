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
public:
    ORIGINE_COMPONENT();

    SpotLight() {}
    ~SpotLight() {}

    void Initialize(Scene* /*_scene*/, const EntityHandle& /*_owner*/) {}

    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel);

    void Finalize() {}

public:
    bool isActive = true;

    Vec3f color           = {1.f, 1.f, 1.f};
    Vec3f pos             = {0.f, 0.f, 0.f};
    float intensity       = 0.f;
    Vec3f direction       = {0.f, 0.f, 1.f};
    float distance        = 0.f;
    float decay           = 0.f;
    float cosAngle        = 0.f;
    float cosFalloffStart = 0.f;
    float angularRadius;

    /// Transform連動用インデックス（-1 = 連動しない）
    int32_t targetTransformIndex = -1;

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
        float angularRadius;
        float padding; // 8 bytes (to align to 16 bytes)
        ConstantBuffer& operator=(const SpotLight& _comp) {
            color           = _comp.color;
            pos             = _comp.pos;
            intensity       = _comp.intensity;
            direction       = _comp.direction;
            distance        = _comp.distance;
            decay           = _comp.decay;
            cosAngle        = _comp.cosAngle;
            cosFalloffStart = _comp.cosFalloffStart;
            angularRadius   = _comp.angularRadius;
            return *this;
        }
    };
};

// D-1: 保存対象フィールドに Opaque が無いため、表経由でシリアライズする。
// 手書きの to_json/from_json は D-4 で削除済み(ComponentArray 経由以外の呼び出しが
// 無いことを確認済み。docs/todo.html Phase 3 D-4 参照)。
template <>
inline constexpr bool kUsesDescriptorSerialization<SpotLight> = true;

} // namespace OriGine
