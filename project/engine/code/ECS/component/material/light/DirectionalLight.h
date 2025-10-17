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

    void Initialize([[maybe_unused]] GameEntity* _entity) override {}

    void Edit(Scene* _scene, GameEntity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;

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
    bool isActive() const { return isActive_; }
    void setActive(bool _isActive) { isActive_ = _isActive; }

    Vec3f getColor() const { return color_; }
    void setColor(const Vec3f& _color) { color_ = _color; }
    float getIntensity() const { return intensity_; }
    void setIntensity(float _intensity) { intensity_ = _intensity; }
    Vec3f getDirection() const { return direction_; }
    void setDirection(const Vec3f& _direction) { direction_ = _direction; }
};
