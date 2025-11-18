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
/// Point Light
/// </summary>
class PointLight
    : public IComponent {
    friend void to_json(nlohmann::json& j, const PointLight& l);
    friend void from_json(const nlohmann::json& j, PointLight& l);

public:
    PointLight() {}
    ~PointLight() {}

    void Initialize([[maybe_unused]] Entity* _entity) override {}
    void Edit(Scene* _scene,Entity* _entity,[[maybe_unused]] const std::string& _parentLabel) override;
    
    void Finalize() override {}

private:
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

public: // access
    bool IsActive() const { return isActive_; }
    void SetActive(bool _isActive) { isActive_ = _isActive; }

    Vec3f GetColor() const { return color_; }
    void SetColor(const Vec3f& _color) { color_ = _color; }
    Vec3f GetPos() const { return pos_; }
    void SetPos(const Vec3f& _pos) { pos_ = _pos; }
    float GetIntensity() const { return intensity_; }
    void SetIntensity(float _intensity) { intensity_ = _intensity; }
    float GetRadius() const { return radius_; }
    void SetRadius(float _radius) { radius_ = _radius; }
    float GetDecay() const { return decay_; }
    void SetDecay(float _decay) { decay_ = _decay; }
};
