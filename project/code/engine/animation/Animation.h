#pragma once

///stl
//container
#include <unordered_map>
//string
#include <string>

///engine
//assets
struct Model;
struct ModelNode;

//lib
#include "globalVariables/SerializedField.h"
//math
#include "Matrix4x4.h"
#include "Quaternion.h"
#include "Vector3.h"

/// <summary>
/// 時間と紐づけられた値を表すクラス
/// </summary>
template <typename T>
struct Keyframe {
    Keyframe() = default;
    Keyframe(float time, const T& value)
        : time(time), value(value) {}
    ~Keyframe() = default;

    float time; // キーフレームの時刻
    T value;    // キーフレームの 値
};

// ============= using ================= //
template <typename T>
using KeyFrame           = Keyframe<T>;
using KeyframeVector3    = Keyframe<Vec3f>;
using KeyframeQuaternion = Keyframe<Quaternion>;

template <typename T>
using AnimationCurve = std::vector<Keyframe<T>>;

/// <summary>
/// 補間の種類
/// </summary>
enum class InterpolationType {
    LINEAR,
    STEP,

    COUNT
};

/// <summary>
/// ノードによるアニメーションを行う曲線
/// </summary>
struct NodeAnimation {
    NodeAnimation()  = default;
    ~NodeAnimation() = default;

    NodeAnimation(
        const AnimationCurve<Vec3f>& _scale,
        const AnimationCurve<Quaternion>& _rotate,
        const AnimationCurve<Vec3f>& _translate,
        InterpolationType _interpolationType)
        : scale(_scale),
          rotate(_rotate),
          translate(_translate),
          interpolationType(_interpolationType) {}

    AnimationCurve<Vec3f> scale;
    AnimationCurve<Quaternion> rotate;
    AnimationCurve<Vec3f> translate;
    InterpolationType interpolationType = InterpolationType::LINEAR; // デフォルトはLINEAR
};

/// <summary>
/// アニメーションデータ
/// </summary>
struct AnimationData {
    AnimationData() = default;
    AnimationData(float _duration)
        : duration(_duration) {}
    ~AnimationData() = default;

    float duration = 0.0f;
    std::unordered_map<std::string, NodeAnimation> nodeAnimations;
};

namespace CalculateValue {
    float LINEAR(
        const std::vector<Keyframe<float>>& keyframes, float time);
    Vec3f LINEAR(
        const std::vector<KeyframeVector3>& keyframes, float time);
    Vec4f LINEAR(
        const std::vector<Keyframe<Vec4f>>& keyframes, float time);
    Quaternion LINEAR(
        const std::vector<KeyframeQuaternion>& keyframes, float time);

    float Step(
        const std::vector<Keyframe<float>>& keyframes, float time);
    Vec3f Step(
        const std::vector<KeyframeVector3>& keyframes, float time);
    Vec4f Step(
        const std::vector<Keyframe<Vec4f>>& keyframes, float time);
    Quaternion Step(
        const std::vector<KeyframeQuaternion>& keyframes, float time);
} // namespace CalculateValue

/// <summary>
/// アニメーションの再生を行うクラス
/// </summary>
struct Animation {
    Animation() = default;
    Animation(AnimationData* _data)
        : data(_data) {
        duration = data->duration;
    }

    ~Animation() = default;

    void Update(
        float deltaTime,
        Model* model,
        const Matrix4x4& parentTransform);

private:
    Matrix4x4 CalculateNodeLocal(const std::string& nodeName) const;

    /// <summary>
    /// ノードにアニメーションを適用
    /// </summary>
    /// <param name="node">root Node</param>
    /// <param name="parentTransform">rootNode ParentMatrix</param>
    /// <param name="animation">animation</param>
    void ApplyAnimationToNodes(
        ModelNode& node,
        const Matrix4x4& parentTransform,
        const Animation* animation);

private:
    AnimationData* data = nullptr;
    //* アニメーションの再生時間(data にも あるが instance 毎に変更できるようにこちらで管理する)
    float duration             = 0.0f; // (秒)
    float currentAnimationTime = 0.0f; // アニメーション の 経過時間 (秒)

    bool isEnd_ = false; // アニメーションが終了したか

public:
    bool isEnd() const { return isEnd_; }

    float getDuration() const { return duration; }
    void setDuration(float _duration) { duration = _duration; }

    float getCurrentAnimationTime() const { return currentAnimationTime; }
    void setCurrentAnimationTime(float _currentAnimationTime) { currentAnimationTime = _currentAnimationTime; }

    AnimationData* getData() const { return data; }
    void setData(AnimationData* _data) { data = _data; }

    Vec3f getCurrentScale(const std::string& nodeName) const;
    Quaternion getCurrentRotate(const std::string& nodeName) const;
    Vec3f getCurrentTranslate(const std::string& nodeName) const;
};

struct AnimationSetting {
    AnimationSetting(const std::string& _name)
        : name(_name.c_str()),
          targetModelDirection("Animations", _name, "targetModelDirection"),
          targetModelFileName("Animations", _name, "targetModelFileName"),
          targetAnimationDirection("Animations", _name, "targetAnimationDirection") {}
    ~AnimationSetting() {}
    // アニメーション名
    std::string name;
    // アニメーション対象のモデル名
    SerializedField<std::string> targetModelDirection;
    SerializedField<std::string> targetModelFileName;
    // アニメーション対象のアニメーション名
    SerializedField<std::string> targetAnimationDirection;
    //! targetAnimationFileName_ = name; とする
};
