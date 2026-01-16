#pragma once

#include "component/IComponent.h"

/// ECS
// component
#include "AnimationData.h"
#include "component/effect/post/DissolveEffectParam.h"

namespace OriGine {

/// <summary>
/// Dissolve のパラメータをアニメーションさせるコンポーネント
/// </summary>
class DissolveAnimation
    : public IComponent {
    friend void to_json(nlohmann::json& _j, const DissolveAnimation& _comp);
    friend void from_json(const nlohmann::json& _j, DissolveAnimation& _comp);

public:
    DissolveAnimation();
    ~DissolveAnimation() override;

    void Initialize(Scene* _scene, EntityHandle _entity) override;
    void Finalize() override;

    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;

    void Update(float _deltaTime, DissolveEffectParam* _param);

    void PlayStart();
    void Stop();
    void RescaleDuration(float _newDuration);

private:
    void UpdateParam(DissolveEffectParam* _param);

private:
    float duration_    = 0.0f;
    float currentTime_ = 0.0f;

#ifdef _DEBUG
    bool isDebugPlay_ = false;
#endif

    AnimationState animationState_;
    InterpolationType interpolationType_ = InterpolationType::LINEAR;

    /// animation curves
    AnimationCurve<float> thresholdCurve_;
    AnimationCurve<float> edgeWidthCurve_;
    AnimationCurve<Vec4f> outLineColorCurve_;

public: // accessor
    float GetDuration() const { return duration_; }
    float GetCurrentTime() const { return currentTime_; }

    bool IsPlaying() const {
#ifdef _DEBUG
        return isDebugPlay_;
#else
        return animationState_.isPlay_;
#endif
    }

    bool IsLoop() const { return animationState_.isLoop_; }
    bool IsEnd() const { return animationState_.isEnd_; }

    AnimationCurve<float>& GetThresholdCurve() { return thresholdCurve_; }
    AnimationCurve<float>& GetEdgeWidthCurve() { return edgeWidthCurve_; }
    AnimationCurve<Vec4f>& GetOutLineColorCurve() { return outLineColorCurve_; }

    void SetDuration(float _duration) { duration_ = _duration; }
    void SetInterpolationType(InterpolationType _type) { interpolationType_ = _type; }
};

} // namespace OriGine
