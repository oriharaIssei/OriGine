#pragma once

#include "component/IComponent.h"

/// engine
// component
#include "component/animation/AnimationData.h"
struct Material;

/// <summary>
/// Materialをアニメーションさせるコンポーネント
/// </summary>
class MaterialAnimation
    : public IComponent {
    friend void to_json(nlohmann::json& _json, const MaterialAnimation& _primitiveNodeAnimation);
    friend void from_json(const nlohmann::json& _json, MaterialAnimation& _primitiveNodeAnimation);

public:
    MaterialAnimation()           = default;
    ~MaterialAnimation() override = default;

    void Initialize(Entity* _entity) override;

    void Edit(Scene* _scene, Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;

    void Finalize() override;

    void Update(float _deltaTime, Material* _material);

    /// <summary>
    /// アニメーションを最初から再生する
    /// </summary>
    void PlayStart();
    /// <summary>
    /// アニメーションを停止する
    /// </summary>
    void Stop();

    /// <summary>
    /// 各ノードのキーフレーム時間を新しいdurationに合わせてリスケールする
    /// </summary>
    /// <param name="_newDuration"></param>
    void RescaleDuration(float _newDuration);

protected:
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

    AnimationState animationState_;

    int32_t materialIndex_ = 0; // 対象のマテリアルインデックス

    InterpolationType interpolationType_ = InterpolationType::LINEAR;

    /// material animation
    AnimationCurve<Vec4f> colorCurve_;
    // uv
    AnimationCurve<Vec2f> uvscaleCurve_;
    AnimationCurve<float> uvRotateCurve_;
    AnimationCurve<Vec2f> uvTranslateCurve_;

public:
    int32_t GetMaterialIndex() const { return materialIndex_; }
    void SetMaterialIndex(int32_t _materialIndex) { materialIndex_ = _materialIndex; }

    float GetDuration() const { return duration_; }
    float GetCurrentTime() const { return currentTime_; }
    void SetDuration(float _duration) { duration_ = _duration; }
    void SetCurrentTime(float _currentTime) { currentTime_ = _currentTime; }

    bool GetAnimationIsLoop() const { return animationState_.isLoop_; }
    bool GetAnimationIsPlay() const { return animationState_.isPlay_; }
    bool GetAnimationIsEnd() const { return animationState_.isEnd_; }
    void SetAnimationIsLoop(bool _isLoop) { animationState_.isLoop_ = _isLoop; }
    void SetAnimationIsPlay(bool _isPlay) { animationState_.isPlay_ = _isPlay; }
    void SetAnimationIsEnd(bool _isEnd) { animationState_.isEnd_ = _isEnd; }

    InterpolationType GetInterpolationType() const { return interpolationType_; }
    void SetInterpolationType(InterpolationType _uvInterpolationType) { interpolationType_ = _uvInterpolationType; }
};
