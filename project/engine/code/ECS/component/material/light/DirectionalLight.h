#pragma once

#include <d3d12.h>
#include <wrl.h>

/// stl
#include <string>

/// engine
// ecs
#include "component/IComponent.h"

/// math
#include "math/Vector3.h"
#include "math/Vector4.h"

/// <summary>
/// DirectionalLight
/// </summary>
class DirectionalLight
    : public IComponent {

    // to_json, from_json を friend として宣言
    friend void to_json(nlohmann::json& j, const DirectionalLight& l);
    friend void from_json(const nlohmann::json& j, DirectionalLight& l);

public:
    DirectionalLight() : IComponent() {}
    ~DirectionalLight() {}

    void Initialize([[maybe_unused]] Entity* _entity) override {}

    void Edit(Scene* _scene, Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;

    void Finalize() override {}

private:
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

public: // access
    bool IsActive() const { return isActive_; }
    void SetActive(bool _isActive) { isActive_ = _isActive; }

    Vec3f GetColor() const { return color_; }
    void SetColor(const Vec3f& _color) { color_ = _color; }
    float GetIntensity() const { return intensity_; }
    void SetIntensity(float _intensity) { intensity_ = _intensity; }
    Vec3f GetDirection() const { return direction_; }
    void SetDirection(const Vec3f& _direction) { direction_ = _direction; }
};
