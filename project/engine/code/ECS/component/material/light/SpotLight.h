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

/// <summary>
/// Spot Light
/// </summary>
class SpotLight
    : public IComponent {
    friend void to_json(nlohmann::json& j, const SpotLight& l);
    friend void from_json(const nlohmann::json& j, SpotLight& l);

public:
    SpotLight() {}
    ~SpotLight() {}

    void Initialize([[maybe_unused]] Entity* _entity) override {}

    void Edit(Scene* _scene,Entity* _entity,[[maybe_unused]] const std::string& _parentLabel) override;

    void Finalize() override {}

private:
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

public: // access
    bool isActive() const { return isActive_; }
    void setActive(bool _isActive) { isActive_ = _isActive; }

    Vec3f getColor() const { return color_; }
    void setColor(const Vec3f& _color) { color_ = _color; }
    Vec3f getPos() const { return pos_; }
    void setPos(const Vec3f& _pos) { pos_ = _pos; }
    float getIntensity() const { return intensity_; }
    void setIntensity(float _intensity) { intensity_ = _intensity; }
    Vec3f getDirection() const { return direction_; }
    void setDirection(const Vec3f& _direction) { direction_ = _direction; }
    float getDistance() const { return distance_; }
    void setDistance(float _distance) { distance_ = _distance; }
    float getDecay() const { return decay_; }
    void setDecay(float _decay) { decay_ = _decay; }
    float getCosAngle() const { return cosAngle_; }
    void setCosAngle(float _cosAngle) { cosAngle_ = _cosAngle; }
    float getCosFalloffStart() const { return cosFalloffStart_; }
    void setCosFalloffStart(float _cosFalloffStart) { cosFalloffStart_ = _cosFalloffStart; }
};
