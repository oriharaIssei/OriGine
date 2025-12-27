#pragma once

// parent
#include "component/IComponent.h"

/// engine
// component
#include "component/animation/AnimationData.h"

/// math
#include <Vector2.h>
#include <Vector4.h>

namespace OriGine {
// 前方宣言
class SpriteRenderer;

/// <summary>
/// Spriteをアニメーションさせるコンポーネント
/// </summary>
class SpriteAnimation
    : public IComponent {
    friend void to_json(nlohmann::json& j, const SpriteAnimation& r);
    friend void from_json(const nlohmann::json& j, SpriteAnimation& r);

public:
    SpriteAnimation();
    ~SpriteAnimation() override;

    void Initialize(Scene* _scene, EntityHandle _owner) override;
    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;
    void Finalize() override;

    void UpdateSpriteAnimation(float _deltaTime, SpriteRenderer* _spriteRenderer);

    /// <summary>
    /// Animation を開始する
    /// </summary>
    void PlayStart();
    /// <summary>
    /// Animation を停止する
    /// </summary>
    void Stop();

    /// <summary>
    /// Color Animation を再生する
    /// </summary>
    void PlayColorAnimation();
    /// <summary>
    /// Transform Animation を再生する
    /// </summary>
    void PlayTransformAnimation();
    /// <summary>
    /// UV Animation を再生する
    /// </summary>
    void PlayUVAnimation();

private:
    ComponentHandle spriteComponentHandle_ = ComponentHandle();

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
    AnimationCurve<Vec2f> uvscaleCurve_     = {};
    AnimationCurve<float> uvRotateCurve_    = {};
    AnimationCurve<Vec2f> uvTranslateCurve_ = {};

public:
    bool IsPlaying() const {
        return colorAnimationState_.isPlay_ || transformAnimationState_.isPlay_ || uvAnimationState_.isPlay_;
    }
    bool IsColorAnimationPlaying() const { return colorAnimationState_.isPlay_; }
    bool IsTransformAnimationPlaying() const { return transformAnimationState_.isPlay_; }
    bool IsUVAnimationPlaying() const { return uvAnimationState_.isPlay_; }

    bool IsEnded() const {
        return colorAnimationState_.isEnd_ && transformAnimationState_.isEnd_ && uvAnimationState_.isEnd_;
    }
    bool IsColorAnimationEnded() const { return colorAnimationState_.isEnd_; }
    bool IsTransformAnimationEnded() const { return transformAnimationState_.isEnd_; }
    bool IsUVAnimationEnded() const { return uvAnimationState_.isEnd_; }

    ComponentHandle GetSpriteComponentHandle() const { return spriteComponentHandle_; }
    void SetSpriteComponentHandle(ComponentHandle _handle) { spriteComponentHandle_ = _handle; }

    AnimationCurve<Vec4f>& GetColorCurve() { return colorCurve_; }
    AnimationCurve<Vec2f>& GetScaleCurve() { return scaleCurve_; }
    AnimationCurve<float>& GetRotateCurve() { return rotateCurve_; }
    AnimationCurve<Vec2f>& GetTranslateCurve() { return translateCurve_; }

    AnimationCurve<Vec2f>& GetUVScaleCurve() { return uvscaleCurve_; }
    AnimationCurve<float>& GetUVRotateCurve() { return uvRotateCurve_; }
    AnimationCurve<Vec2f>& GetUVTranslateCurve() { return uvTranslateCurve_; }
};

} // namespace OriGine
