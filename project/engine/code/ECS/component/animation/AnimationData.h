#pragma once

/// stl
#include <string>
#include <unordered_map>

/// engine

/// math
#include <Quaternion.h>
#include <Vector.h>
#include <Vector3.h>
#include <Vector4.h>


#include "binaryIO/BinaryIO.h"

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
    T value; // キーフレームの 値
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

struct AnimationState {
    bool isPlay_ = false;
    bool isEnd_  = false;
    bool isLoop_ = false;
};

static const char* InterpolationTypeName[] = {
    "LINEAR",
    "STEP",
};

/// <summary>
/// ノードによるアニメーションを行う曲線
/// </summary>
struct ModelAnimationNode {
    ModelAnimationNode()  = default;
    ~ModelAnimationNode() = default;

    ModelAnimationNode(
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
    std::unordered_map<std::string, ModelAnimationNode> animationNodes_;
};

namespace CalculateValue {
float Linear(
    const std::vector<Keyframe<float>>& keyframes, float time);
Vec2f Linear(
    const std::vector<Keyframe<Vec2f>>& keyframes, float time);
Vec3f Linear(
    const std::vector<KeyframeVector3>& keyframes, float time);
Vec4f Linear(
    const std::vector<Keyframe<Vec4f>>& keyframes, float time);
Quaternion Linear(
    const std::vector<KeyframeQuaternion>& keyframes, float time);

float Step(
    const std::vector<Keyframe<float>>& keyframes, float time);
Vec2f Step(
    const std::vector<Keyframe<Vec2f>>& keyframes, float time);
Vec3f Step(
    const std::vector<KeyframeVector3>& keyframes, float time);
Vec4f Step(
    const std::vector<Keyframe<Vec4f>>& keyframes, float time);
Quaternion Step(
    const std::vector<KeyframeQuaternion>& keyframes, float time);
} // namespace CalculateValue
