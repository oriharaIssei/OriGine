#pragma once

#include "component/IComponent.h"

/// stl
#include <array>

/// math
#include "Vector3.h"

namespace OriGine {

/// <summary>
/// オブジェクトの伸縮を表すコンポーネント (Transform&Rigidbodyを使用する)
/// </summary>
class SquashStretchComponent
    : public IComponent {
    friend void to_json(nlohmann::json& _j, const SquashStretchComponent& _component);
    friend void from_json(const nlohmann::json& _j, SquashStretchComponent& _component);

public:
    SquashStretchComponent()           = default;
    ~SquashStretchComponent() override = default;

    void Initialize(Scene* _scene, EntityHandle _owner) override;
    void Finalize() override;
    void Edit(Scene* _scene, EntityHandle _owner, const std::string& _parentLabel) override;

private:
    Vec3f baseScale_     = Vec3f(1.f, 1.f, 1.f);
    Vec3f scaleVelocity_ = {};

    Vec3f preVelocity_ = {};

    std::array<bool, 3> isVelocityStretchActiveByAxis_ = {false, false, false};
    std::array<bool, 3> isAccelSquashActiveByAxis_     = {false, false, false};
    float velocityStretchThresholdEnter_               = 1.f;
    float velocityStretchThresholdExit_                = 1.f;

    float accelSquashThresholdEnter_ = 1.f;
    float accelSquashThresholdExit_  = 1.f;

    float velocityStretchInfluence_ = 1.f;
    float accelSquashInfluence_     = 1.f;

    Vec3f stiffness_ = {};
    Vec3f damping_   = {};

    float maxStretch_ = 1.f;
    float maxSquash_  = 1.f;

public:
    const Vec3f& GetBaseScale() const {
        return baseScale_;
    }
    void SetBaseScale(const Vec3f& _baseScale) {
        baseScale_ = _baseScale;
    }

    const Vec3f& GetPreVelocity() const {
        return preVelocity_;
    }
    void SetPreVelocity(const Vec3f& _preVelocity) {
        preVelocity_ = _preVelocity;
    }
    const Vec3f& GetScaleVelocity() const {
        return scaleVelocity_;
    }
    void SetScaleVelocity(const Vec3f& _scaleVelocity) {
        scaleVelocity_ = _scaleVelocity;
    }

    std::array<bool, 3> IsVelocityStretchActiveByAxis() const {
        return isVelocityStretchActiveByAxis_;
    }
    void SetVelocityStretchActive(AxisIndex _axis, bool _isVelocityStretchActive) {
        isVelocityStretchActiveByAxis_[_axis] = _isVelocityStretchActive;
    }
    std::array<bool, 3> IsAccelSquashActiveByAxis() const {
        return isAccelSquashActiveByAxis_;
    }
    void SetAccelSquashActive(AxisIndex _axis, bool _isAccelSquashActive) {
        isAccelSquashActiveByAxis_[_axis] = _isAccelSquashActive;
    }

    float GetVelocityStretchThresholdEnter() const {
        return velocityStretchThresholdEnter_;
    }
    float GetVelocityStretchThresholdExit() const {
        return velocityStretchThresholdEnter_;
    }
    float GetAccelSquashThresholdEnter() const {
        return accelSquashThresholdEnter_;
    }
    float GetAccelSquashThresholdExit() const {
        return accelSquashThresholdExit_;
    }

    float GetVelocityStretchInfluence() const {
        return velocityStretchInfluence_;
    }
    void SetVelocityStretchInfluence(float _velocityStretchInfluence) {
        velocityStretchInfluence_ = _velocityStretchInfluence;
    }
    float GetAccelSquashInfluence() const {
        return accelSquashInfluence_;
    }
    void SetAccelSquashInfluence(float _accelSquashInfluence) {
        accelSquashInfluence_ = _accelSquashInfluence;
    }
    const Vec3f& GetStiffness() const {
        return stiffness_;
    }
    void SetStiffness(const Vec3f& _stiffness) {
        stiffness_ = _stiffness;
    }
    const Vec3f& GetDamping() const {
        return damping_;
    }
    void SetDamping(const Vec3f& _damping) {
        damping_ = _damping;
    }
    float GetMaxStretch() const {
        return maxStretch_;
    }
    void SetMaxStretch(float _maxStretch) {
        maxStretch_ = _maxStretch;
    }
    float GetMaxSquash() const {
        return maxSquash_;
    }
    void SetMaxSquash(float _maxSquash) {
        maxSquash_ = _maxSquash;
    }
};
}
