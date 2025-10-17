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

/// <summary>
/// PrimtiveをNode単位でアニメーションさせるコンポーネント
/// </summary>
class PrimitiveNodeAnimation
    : public IComponent {
    friend void to_json(nlohmann::json& _json, const PrimitiveNodeAnimation& _primitiveNodeAnimation);
    friend void from_json(const nlohmann::json& _json, PrimitiveNodeAnimation& _primitiveNodeAnimation);

public:
    PrimitiveNodeAnimation()           = default;
    ~PrimitiveNodeAnimation() override = default;

    void Initialize(GameEntity* _entity) override;

    void Edit(Scene* _scene, GameEntity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;

    void Finalize() override;

    void Update(float _deltaTime, Transform* _transform);

    void PlayStart();
    void Stop();

protected:
    /// <summary>
    /// Transformに対してアニメーションを適用する
    /// </summary>
    /// <param name="_transform"></param>
    void UpdateTransformAnimation(Transform* _transform);

private:
    float duration_    = 0.0f; // (秒)
    float currentTime_ = 0.0f; // (秒)

    AnimationState animationState_;

    InterpolationType interpolationType_ = InterpolationType::LINEAR;

    /// transform animation
    AnimationCurve<Vec3f> scaleCurve_;
    AnimationCurve<Quaternion> rotateCurve_;
    AnimationCurve<Vec3f> translateCurve_;

public:
    float getDuration() const { return duration_; }
    float getCurrentTime() const { return currentTime_; }
    void setDuration(float _duration) { duration_ = _duration; }
    void setCurrentTime(float _currentTime) { currentTime_ = _currentTime; }

    bool getAnimationIsLoop() const { return animationState_.isLoop_; }
    bool getAnimationIsPlay() const { return animationState_.isPlay_; }
    bool getAnimationIsEnd() const { return animationState_.isEnd_; }
    void setAnimationIsLoop(bool _isLoop) { animationState_.isLoop_ = _isLoop; }
    void setAnimationIsPlay(bool _isPlay) { animationState_.isPlay_ = _isPlay; }
    void setAnimationIsEnd(bool _isEnd) { animationState_.isEnd_ = _isEnd; }

    InterpolationType getTransformInterpolationType() const { return interpolationType_; }
    void setInterpolationType(InterpolationType _interpolationType) { interpolationType_ = _interpolationType; }
};
