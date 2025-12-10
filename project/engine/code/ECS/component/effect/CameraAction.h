#pragma once
#include "component/IComponent.h"

/// ECS
// component
#include "component/animation/AnimationData.h"

namespace OriGine {

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
    void Edit(Scene* _scene, Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;
    void Finalize() override;

    void Play() {
        animationState_.isPlay_ = true;
        animationState_.isEnd_  = false;
        currentTime_            = 0.0f;
    }
    void PlayContinue() {
        animationState_.isPlay_ = true;
        animationState_.isEnd_  = false;
    }
    void Stop() {
        animationState_.isPlay_ = false;
        animationState_.isEnd_  = true;
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
    bool IsEnd() const { return animationState_.isEnd_; }

    bool isLooping() const { return animationState_.isLoop_; }
    void SetIsLoop(bool _isLoop) { animationState_.isLoop_ = _isLoop; }

    float GetDuration() const { return duration_; }
    void SetDuration(float duration) { duration_ = duration; }
    float GetTime() const { return currentTime_; }
    void SetCurrentTime(float _currentTime) { currentTime_ = _currentTime; }

    const AnimationCurve<float>& GetFovCurve() const { return fovCurve_; }
    const AnimationCurve<float>& GetAspectRatioCurve() const { return aspectRatioCurve_; }
    const AnimationCurve<float>& GetNearZCurve() const { return nearZCurve_; }
    const AnimationCurve<float>& GetFarZCurve() const { return farZCurve_; }
    const AnimationCurve<Vec3f>& GetPositionCurve() const { return positionCurve_; }
    const AnimationCurve<Quaternion>& GetRotationCurve() const { return rotationCurve_; }
    void SetFovCurve(const AnimationCurve<float>& curve) { fovCurve_ = curve; }
    void SetAspectRatioCurve(const AnimationCurve<float>& curve) { aspectRatioCurve_ = curve; }
    void SetNearZCurve(const AnimationCurve<float>& curve) { nearZCurve_ = curve; }
    void SetFarZCurve(const AnimationCurve<float>& curve) { farZCurve_ = curve; }
    void SetPositionCurve(const AnimationCurve<Vec3f>& curve) { positionCurve_ = curve; }
    void SetRotationCurve(const AnimationCurve<Quaternion>& curve) { rotationCurve_ = curve; }
};

} // namespace OriGine
