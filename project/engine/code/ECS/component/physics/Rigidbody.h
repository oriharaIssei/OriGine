#pragma once

/// parent
#include "component/IComponent.h"

/// math
#include "Vector3.h"

class Rigidbody
    : public IComponent {
    friend void to_json(nlohmann::json& j, const Rigidbody& r);
    friend void from_json(const nlohmann::json& j, Rigidbody& r);

public:
    Rigidbody();
    virtual ~Rigidbody() = default;

    void Initialize(GameEntity* _entity) override;

    void Edit(Scene* _scene,GameEntity* _entity,[[maybe_unused]] const std::string& _parentLabel);
    void Debug();

    virtual void Finalize();

private:
    Vec3f acceleration_ = Vec3f(0.0f, 0.0f, 0.0f);
    Vec3f velocity_     = Vec3f(0.0f, 0.0f, 0.0f);

    bool useGravity_    = false;
    float mass_         = 1.0f;
    float maxFallSpeed_ = 1000.0f; // 最大落下速度

public: // accsessor
    const Vec3f& getAcceleration() const { return acceleration_; }
    float getAcceleration(int32_t _index) const { return acceleration_[_index]; }
    void setAcceleration(const Vec3f& _acceleration) { acceleration_ = _acceleration; }
    void setAcceleration(int32_t _index, float _accel) {
        if (_index < 0 || _index >= 3) {
            throw std::out_of_range("Index must be between 0 and 2.");
        }
        acceleration_[_index] = _accel;
    }

    const Vec3f& getVelocity() const { return velocity_; }
    float getVelocity(int32_t _index) const { return velocity_[_index]; }
    void setVelocity(const Vec3f& _velocity) { velocity_ = _velocity; }
    void setVelocity(int32_t _index, float _velo) {
        if (_index < 0 || _index >= 3) {
            throw std::out_of_range("Index must be between 0 and 2.");
        }
        velocity_[_index] = _velo;
    }

    bool getUseGravity() const { return useGravity_; }
    void setUseGravity(const bool _isGravity) { useGravity_ = _isGravity; }

    float getMass() const { return mass_; }
    void setMass(const float _mass) { mass_ = _mass; }

    float maxFallSpeed() const { return maxFallSpeed_; }
    void setMaxFallSpeed(float _maxFallSpeed) {
        maxFallSpeed_ = _maxFallSpeed;
    }
};
