#pragma once
/// parent
#include "component/IComponent.h"

/// stl
#include <memory>

/// engine
// component
#include "component/animation/AnimationData.h"
struct Material;
struct Transform;

class PrimitiveNodeAnimation
    : public IComponent {
    friend void to_json(nlohmann::json& _json, const PrimitiveNodeAnimation& _primitiveNodeAnimation);
    friend void from_json(const nlohmann::json& _json, PrimitiveNodeAnimation& _primitiveNodeAnimation);

public:
    PrimitiveNodeAnimation()           = default;
    ~PrimitiveNodeAnimation() override = default;

    void Initialize(GameEntity* _entity) override;

    bool Edit() override;

    void Finalize() override;

    void Update(float _deltaTime, Transform* _transform, Material* _material);

    void PlayStart();
    void Stop();

protected:
    void UpdateTransformAnimation(Transform* _transform);
    void UpdateMaterialAnimation(Material* _material);

private:
    float duration_    = 0.0f; // (秒)
    float currentTime_ = 0.0f; // (秒)

#ifdef _DEBUG
    // 連番画像から uv Curveにするためのもの
    Vec2f tileSize_            = {};
    Vec2f textureSize_         = {};
    float tilePerTime_         = 0.f;
    float startAnimationTime_  = 0.f;
    float animationTimeLength_ = 0.f;
#endif // _DEBUG

    AnimationState transformAnimationState_;
    AnimationState materialAnimationState_;

    InterpolationType transformInterpolationType_ = InterpolationType::LINEAR;
    InterpolationType colorInterpolationType_     = InterpolationType::LINEAR;
    InterpolationType uvInterpolationType_        = InterpolationType::LINEAR;

    /// transform animation
    AnimationCurve<Vec3f> scaleCurve_;
    AnimationCurve<Quaternion> rotateCurve_;
    AnimationCurve<Vec3f> translateCurve_;

    /// material animation
    AnimationCurve<Vec4f> colorCurve_;
    // uv
    AnimationCurve<Vec2f> uvScaleCurve_;
    AnimationCurve<float> uvRotateCurve_;
    AnimationCurve<Vec2f> uvTranslateCurve_;

public:
    float getDuration() const { return duration_; }
    float getCurrentTime() const { return currentTime_; }
    void setDuration(float _duration) { duration_ = _duration; }
    void setCurrentTime(float _currentTime) { currentTime_ = _currentTime; }

    bool getTransformAnimationIsLoop() const { return transformAnimationState_.isLoop_; }
    bool getTransformAnimationIsPlay() const { return transformAnimationState_.isPlay_; }
    bool getTransformAnimationIsEnd() const { return transformAnimationState_.isEnd_; }
    void setTransformAnimationIsLoop(bool _isLoop) { transformAnimationState_.isLoop_ = _isLoop; }
    void setTransformAnimationIsPlay(bool _isPlay) { transformAnimationState_.isPlay_ = _isPlay; }
    void setTransformAnimationIsEnd(bool _isEnd) { transformAnimationState_.isEnd_ = _isEnd; }

    bool getMaterialAnimationIsLoop() const { return materialAnimationState_.isLoop_; }
    bool getMaterialAnimationIsPlay() const { return materialAnimationState_.isPlay_; }
    bool getMaterialAnimationIsEnd() const { return materialAnimationState_.isEnd_; }
    void setMaterialAnimationIsLoop(bool _isLoop) { materialAnimationState_.isLoop_ = _isLoop; }
    void setMaterialAnimationIsPlay(bool _isPlay) { materialAnimationState_.isPlay_ = _isPlay; }
    void setMaterialAnimationIsEnd(bool _isEnd) { materialAnimationState_.isEnd_ = _isEnd; }

    InterpolationType getTransformInterpolationType() const { return transformInterpolationType_; }
    InterpolationType getColorInterpolationType() const { return colorInterpolationType_; }
    InterpolationType getUvInterpolationType() const { return uvInterpolationType_; }
    void setInterpolationType(InterpolationType _interpolationType) { transformInterpolationType_ = _interpolationType; }
    void setColorInterpolationType(InterpolationType _colorInterpolationType) { colorInterpolationType_ = _colorInterpolationType; }
    void setUvInterpolationType(InterpolationType _uvInterpolationType) { uvInterpolationType_ = _uvInterpolationType; }
};
