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
    friend void to_json(nlohmann::json& _j, const TransformAnimation& _comp);
    friend void from_json(const nlohmann::json& _j, TransformAnimation& _comp);

public:
    /// <summary>
    /// 各軸ごとの反転フラグ
    /// </summary>
    struct FlipMask {
        bool x = false;
        bool y = false;
        bool z = false;
    };

public:
    TransformAnimation();
    ~TransformAnimation() override;

    /// <summary>
    /// 再生時刻を 0 にリセットする
    /// </summary>
    void Initialize(Scene* _scene, const EntityHandle& _entity) override;
    /// <summary>
    /// 保持しているキーフレーム・状態をすべて破棄する
    /// </summary>
    void Finalize() override;

    void Edit(Scene* _scene, const EntityHandle& _entity, const ::std::string& _parentLabel) override;

    /// <summary>
    /// 再生時刻を進め、対象の Transform にキーフレーム補間結果を反映する
    /// </summary>
    /// <param name="_deltaTime">前フレームからの経過時間(秒)</param>
    /// <param name="_transform">値を書き込む対象の Transform</param>
    void Update(float _deltaTime, Transform* _transform);

    /// <summary>
    /// 再生時刻を先頭に戻して再生を開始する
    /// </summary>
    void PlayStart();
    /// <summary>
    /// 再生を停止し、終了状態にする
    /// </summary>
    void Stop();

    /// <summary>
    /// 各キーフレームの時刻を新しい再生時間の比率にあわせて再スケールする
    /// (キー間の相対位置を保ったまま全体の長さだけを変更する)
    /// </summary>
    /// <param name="_newDuration">変更後の再生時間(秒)</param>
    void RescaleDuration(float _newDuration);

private:
    /// <summary>
    /// 現在の補間方式・現在時刻をもとに各カーブから値を計算し、反転を適用して Transform へ書き込む
    /// </summary>
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

    /// flip masks (軸ごとの反転)
    FlipMask scaleFlip_;
    FlipMask rotateFlip_;
    FlipMask translateFlip_;

    // --- 反転ヘルパー ---
    static Vec3f ApplyFlip(Vec3f _val, const FlipMask& _flip);
    static Quaternion ApplyFlipQ(Quaternion _val, const FlipMask& _flip);

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

    FlipMask& GetScaleFlip() { return scaleFlip_; }
    FlipMask& GetRotateFlip() { return rotateFlip_; }
    FlipMask& GetTranslateFlip() { return translateFlip_; }

    const FlipMask& GetScaleFlip() const { return scaleFlip_; }
    const FlipMask& GetRotateFlip() const { return rotateFlip_; }
    const FlipMask& GetTranslateFlip() const { return translateFlip_; }

    void SetScaleFlip(const FlipMask& _flip) { scaleFlip_ = _flip; }
    void SetRotateFlip(const FlipMask& _flip) { rotateFlip_ = _flip; }
    void SetTranslateFlip(const FlipMask& _flip) { translateFlip_ = _flip; }
};

} // namespace OriGine
