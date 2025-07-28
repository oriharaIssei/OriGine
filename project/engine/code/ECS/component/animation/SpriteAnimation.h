#pragma once

// parent
#include "component/IComponent.h"

/// engine
// component
#include "component/animation/AnimationData.h"
class SpriteRenderer;

/// math
#include <Vector2.h>
#include <Vector4.h>

class SpriteAnimation
    : public IComponent {
    friend void to_json(nlohmann::json& j, const SpriteAnimation& r);
    friend void from_json(const nlohmann::json& j, SpriteAnimation& r);

public:
    SpriteAnimation();
    ~SpriteAnimation() override;

    void Initialize(GameEntity* _hostEntity) override;
    void Edit(Scene* _scene,GameEntity* _entity,const std::string& _parentLabel) override;
    void Finalize() override;

    void UpdateSpriteAnimation(float _deltaTime, SpriteRenderer* _spriteRenderer);

    void PlayStart();
    void Stop();

private:
    float duration_    = 0.0f; // (秒)
    float currentTime_ = 0.0f; // (秒)

    AnimationState colorAnimationState_       = {};
    InterpolationType colorInterpolationType_ = InterpolationType::LINEAR;
    AnimationCurve<Vec4f> colorCurve_         = {};

    AnimationState transformAnimationState_       = {};
    InterpolationType transformInterpolationType_ = InterpolationType::LINEAR;
    AnimationCurve<Vec2f> scaleCurve_             = {};
    AnimationCurve<float> rotateCurve_            = {};
    AnimationCurve<Vec2f> translateCurve_         = {};

    AnimationState uvAnimationState_        = {};
    InterpolationType uvInterpolationType_  = InterpolationType::LINEAR;
    AnimationCurve<Vec2f> uvScaleCurve_     = {};
    AnimationCurve<float> uvRotateCurve_    = {};
    AnimationCurve<Vec2f> uvTranslateCurve_ = {};
};
