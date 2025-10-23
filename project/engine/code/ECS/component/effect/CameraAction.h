#pragma once
#include "component/IComponent.h"

#include "component/animation/AnimationData.h"

/// <summary>
/// カメラをアニメーションさせるコンポーネント
/// </summary>
class CameraAction
    : public IComponent {
    friend void to_json(nlohmann::json& j, const CameraAction& action);
    friend void from_json(const nlohmann::json& j, CameraAction& action);

public:
    CameraAction();
    ~CameraAction();

    void Initialize(Entity* _entity) override;
    void Edit(Scene* _scene,Entity* _entity,[[maybe_unused]] const std::string& _parentLabel) override;
    void Finalize() override;

    void Play() {
        animationState_.isPlay_ = true;
        animationState_.isEnd_  = false;
        currentTime_            = duration_;
    }
    void Stop() {
        animationState_.isPlay_ = false;
        animationState_.isEnd_  = true;
        currentTime_            = 0.0f;
    }

private:
    float duration_    = 1.f; // アニメーションの総時間
    float currentTime_ = 0.0f;
    AnimationState animationState_; // アニメーション状態

    // カメラのアニメーションカーブ
    AnimationCurve<float> fovCurve_; // FOVのアニメーションカーブ
    AnimationCurve<float> aspectRatioCurve_; // アスペクト比のアニメーションカーブ
    AnimationCurve<float> nearZCurve_; // 近接深度のアニメーションカーブ
    AnimationCurve<float> farZCurve_; // 遠方深度のアニメーションカーブ

    AnimationCurve<Vec3f> positionCurve_; // カメラ位置のアニメーションカーブ
    AnimationCurve<Quaternion> rotationCurve_; // カメラ回転のアニメーションカーブ
public:
    bool isPlaying() const { return animationState_.isPlay_; }
    bool isEnd() const { return animationState_.isEnd_; }

    bool isLooping() const { return animationState_.isLoop_; }
    void setIsLoop(bool _isLoop) { animationState_.isLoop_ = _isLoop; }

    float getDuration() const { return duration_; }
    void setDuration(float duration) { duration_ = duration; }
    float getCurrentTime() const { return currentTime_; }
    void setCurrentTime(float _currentTime) { currentTime_ = _currentTime; }

    const AnimationCurve<float>& getFovCurve() const { return fovCurve_; }
    const AnimationCurve<float>& getAspectRatioCurve() const { return aspectRatioCurve_; }
    const AnimationCurve<float>& getNearZCurve() const { return nearZCurve_; }
    const AnimationCurve<float>& getFarZCurve() const { return farZCurve_; }
    const AnimationCurve<Vec3f>& getPositionCurve() const { return positionCurve_; }
    const AnimationCurve<Quaternion>& getRotationCurve() const { return rotationCurve_; }
    void setFovCurve(const AnimationCurve<float>& curve) { fovCurve_ = curve; }
    void setAspectRatioCurve(const AnimationCurve<float>& curve) { aspectRatioCurve_ = curve; }
    void setNearZCurve(const AnimationCurve<float>& curve) { nearZCurve_ = curve; }
    void setFarZCurve(const AnimationCurve<float>& curve) { farZCurve_ = curve; }
    void setPositionCurve(const AnimationCurve<Vec3f>& curve) { positionCurve_ = curve; }
    void setRotationCurve(const AnimationCurve<Quaternion>& curve) { rotationCurve_ = curve; }
};
