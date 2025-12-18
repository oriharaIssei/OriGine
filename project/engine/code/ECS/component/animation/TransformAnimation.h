#pragma once

#include "component/IComponent.h"

/// stl
#include <string>

/// ECS
// component
#include "component/transform/Transform.h"

/// data
// animation
#include "AnimationData.h"

namespace OriGine {

/// <summary>
/// Transform をアニメーションさせるコンポーネント
/// </summary>
class TransformAnimation
    : public IComponent {
    friend void to_json(nlohmann::json&, const TransformAnimation&);
    friend void from_json(const nlohmann::json&, TransformAnimation&);

public:
    TransformAnimation();
    ~TransformAnimation() override;

    void Initialize(Entity* _entity) override;
    void Finalize() override;

    void Edit(Scene* _scene, Entity* _entity, const ::std::string& _parentLabel) override;

    void Update(float _deltaTime, Transform* _transform);

    void PlayStart();
    void Stop();

    void RescaleDuration(float _newDuration);

private:
    void UpdateTransform(Transform* _transform);

private:
    int32_t targetTransformIndex_ = -1;
#ifdef DEBUG
    bool isDebugPlay_ = false;
#endif // DEBUG

    float duration_    = 0.0f;
    float currentTime_ = 0.0f;

    AnimationState animationState_;
    InterpolationType interpolationType_ = InterpolationType::LINEAR;

    /// animation curves
    AnimationCurve<Vec3f> scaleCurve_;
    AnimationCurve<Quaternion> rotateCurve_;
    AnimationCurve<Vec3f> translateCurve_;

public: // アクセッサ
    float GetDuration() const { return duration_; }
    float GetCurrentTime() const { return currentTime_; }

    bool IsPlaying() const {
#ifdef _DEBUG
        return isDebugPlay_;
#else
        return animationState_.isPlay_;
#endif // _DEBUG
    }
    bool IsLoop() const { return animationState_.isLoop_; }
    bool IsEnd() const { return animationState_.isEnd_; }

    void EndAnimation() {
#ifdef _DEBUG
        isDebugPlay_ = false;
#else
        animationState_.isPlay_ = false;
#endif // _DEBUG
        animationState_.isEnd_ = true;
        currentTime_           = duration_;
    }

    int32_t GetTargetTransformIndex() const { return targetTransformIndex_; }
    void SetTargetTransformIndex(int32_t _idx) { targetTransformIndex_ = _idx; }

    AnimationCurve<Vec3f>& GetScaleCurve() { return scaleCurve_; }
    AnimationCurve<Quaternion>& GetRotateCurve() { return rotateCurve_; }
    AnimationCurve<Vec3f>& GetTranslateCurve() { return translateCurve_; }

    void SetDuration(float _duration) { duration_ = _duration; }
    void SetInterpolationType(InterpolationType _type) { interpolationType_ = _type; }
};

} // namespace OriGine
