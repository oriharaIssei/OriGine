#pragma once

/// parent
#include "component/IComponent.h"

/// math
#include "Vector3.h"

namespace OriGine {

/// <summary>
/// Rigidbody コンポーネント
/// </summary>
class Rigidbody
    : public IComponent {
    friend void to_json(nlohmann::json& j, const Rigidbody& r);
    friend void from_json(const nlohmann::json& j, Rigidbody& r);

public:
    Rigidbody();
    virtual ~Rigidbody() = default;

    void Initialize(Scene* _scene, EntityHandle _entity) override;

    void Edit(Scene* _scene, EntityHandle _handle, [[maybe_unused]] const std::string& _parentLabel);
    void Debug();

    virtual void Finalize();

private:
    Vec3f acceleration_ = Vec3f(0.0f, 0.0f, 0.0f);
    Vec3f velocity_     = Vec3f(0.0f, 0.0f, 0.0f);
    Vec3f realVelocity_ = Vec3f(0.0f, 0.0f, 0.0f); // DeltaTime をかけた実際の速度
    float maxXZSpeed_   = 100.0f; // XZ 平面の最大速度

    bool useGravity_    = false;
    float mass_         = 1.0f;
    float maxFallSpeed_ = 1000.0f; // 最大落下速度

public: // accsessor
    const Vec3f& GetAcceleration() const { return acceleration_; }
    float GetAcceleration(int32_t _index) const { return acceleration_[_index]; }
    void SetAcceleration(const Vec3f& _acceleration) { acceleration_ = _acceleration; }
    void SetAcceleration(int32_t _index, float _accel) {
        if (_index < 0 || _index >= 3) {
            throw std::out_of_range("Index must be between 0 and 2.");
        }
        acceleration_[_index] = _accel;
    }

    const Vec3f& GetVelocity() const { return velocity_; }
    float GetVelocity(int32_t _index) const { return velocity_[_index]; }
    void SetVelocity(const Vec3f& _velocity) { velocity_ = _velocity; }
    void SetVelocity(int32_t _index, float _velo) {
        if (_index < 0 || _index >= 3) {
            throw std::out_of_range("Index must be between 0 and 2.");
        }
        velocity_[_index] = _velo;
    }

    const Vec3f& GetRealVelocity() const { return realVelocity_; }
    float GetRealVelocity(int32_t _index) const { return realVelocity_[_index]; }
    void SetRealVelocity(const Vec3f& _velocity) { realVelocity_ = _velocity; }
    void SetRealVelocity(int32_t _index, float _velo) {
        if (_index < 0 || _index >= 3) {
            throw std::out_of_range("Index must be between 0 and 2.");
        }
        realVelocity_[_index] = _velo;
    }

    float GetMaxXZSpeed() const { return maxXZSpeed_; }
    void SetMaxXZSpeed(float _maxXZSpeed) { maxXZSpeed_ = _maxXZSpeed; }

    bool GetUseGravity() const { return useGravity_; }
    void SetUseGravity(const bool _isGravity) { useGravity_ = _isGravity; }

    float GetMass() const { return mass_; }
    void SetMass(const float _mass) { mass_ = _mass; }

    float maxFallSpeed() const { return maxFallSpeed_; }
    void SetMaxFallSpeed(float _maxFallSpeed) {
        maxFallSpeed_ = _maxFallSpeed;
    }
};

} // namespace OriGine
