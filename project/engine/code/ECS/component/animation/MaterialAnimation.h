#pragma once

#include "component/IComponent.h"

/// stl
#include <memory>

/// engine
// component
#include "component/animation/AnimationData.h"
struct Material;

class MaterialAnimation
    : public IComponent {
    friend void to_json(nlohmann::json& _json, const MaterialAnimation& _primitiveNodeAnimation);
    friend void from_json(const nlohmann::json& _json, MaterialAnimation& _primitiveNodeAnimation);

public:
    MaterialAnimation()           = default;
    ~MaterialAnimation() override = default;

    void Initialize(GameEntity* _entity) override;

    void Edit(Scene* _scene, GameEntity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;

    void Finalize() override;

    void Update(float _deltaTime, Material* _material);

    void PlayStart();
    void Stop();

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
    AnimationCurve<Vec2f> uvScaleCurve_;
    AnimationCurve<float> uvRotateCurve_;
    AnimationCurve<Vec2f> uvTranslateCurve_;

public:
    int32_t getMaterialIndex() const { return materialIndex_; }
    void setMaterialIndex(int32_t _materialIndex) { materialIndex_ = _materialIndex; }

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

    InterpolationType getInterpolationType() const { return interpolationType_; }
    void setInterpolationType(InterpolationType _uvInterpolationType) { interpolationType_ = _uvInterpolationType; }
};
