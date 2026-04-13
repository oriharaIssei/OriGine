#pragma once

/// stl
#include <string>
#include <unordered_map>

/// math
#include <Quaternion.h>
#include <Vector.h>
#include <Vector3.h>
#include <Vector4.h>

namespace OriGine {

/// <summary>
/// 時間と紐づけられた値を表すクラス
/// </summary>
template <typename T>
struct Keyframe {
    Keyframe() = default;
    Keyframe(float _time, const T& _value)
        : time(_time), value(_value) {}
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
/// 各軸ごとに分離されたアニメーション曲線データ
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="Dimension"></typeparam>
template <int Dimension, typename T>
struct SeparatedAnimCurve {
    // 現在の次元数を取得する定数
    static constexpr int kDim = Dimension;
    using CurveTrack          = std::vector<Keyframe<T>>;

    SeparatedAnimCurve()  = default;
    ~SeparatedAnimCurve() = default;

    // 各軸ごとのキーフレームリスト
    // curves[0] = X軸, curves[1] = Y軸...
    CurveTrack curves[Dimension];
};

template <typename T>
using Separated2dAnimCurve = SeparatedAnimCurve<2, T>;
template <typename T>
using Separated3dAnimCurve = SeparatedAnimCurve<3, T>;
template <typename T>
using Separated4dAnimCurve = SeparatedAnimCurve<4, T>;

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

/// <summary>
/// 補間関数トレイト（デフォルト: Lerp）
/// </summary>
template <typename T>
struct InterpolationTraits {
    static T Interpolate(const T& a, const T& b, float t) {
        return Lerp(a, b, t);
    }
};

/// <summary>
/// float用の特殊化（std::lerp使用）
/// </summary>
template <>
struct InterpolationTraits<float> {
    static float Interpolate(float a, float b, float t) {
        return std::lerp(a, b, t);
    }
};

/// <summary>
/// Quaternion用の特殊化（Slerp使用）
/// </summary>
template <>
struct InterpolationTraits<Quaternion> {
    static Quaternion Interpolate(const Quaternion& a, const Quaternion& b, float t) {
        return Slerp(a, b, t);
    }
};

/// <summary>
/// デフォルト値トレイト
/// </summary>
template <typename T>
struct DefaultValueTraits {
    static T Default() { return T(); }
};

template <>
struct DefaultValueTraits<float> {
    static float Default() { return 0.f; }
};

/// <summary>
/// 線形補間でキーフレーム値を計算
/// </summary>
template <typename T>
T Linear(const std::vector<Keyframe<T>>& _keyframes, float _time) {
    // 例外処理
    if (_keyframes.empty()) {
        return DefaultValueTraits<T>::Default();
    }
    if (_keyframes.size() == 1 || _time <= _keyframes[0].time) {
        return _keyframes[0].value;
    }

    for (size_t index = 0; index < _keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // index と nextIndex の 2つを取得して 現時刻が 範囲内か
        if (_keyframes[index].time <= _time && _time <= _keyframes[nextIndex].time) {
            // 範囲内 で 補間
            float t = (_time - _keyframes[index].time) / (_keyframes[nextIndex].time - _keyframes[index].time);
            return InterpolationTraits<T>::Interpolate(_keyframes[index].value, _keyframes[nextIndex].value, t);
        }
    }
    // 登録されている時間より 後ろ -> 最後の値を返す
    return _keyframes.back().value;
}

/// <summary>
/// ステップ補間でキーフレーム値を計算（補間なし）
/// </summary>
template <typename T>
T Step(const std::vector<Keyframe<T>>& _keyframes, float _time) {
    // 例外処理
    if (_keyframes.empty()) {
        return DefaultValueTraits<T>::Default();
    }
    if (_keyframes.size() == 1 || _time <= _keyframes[0].time) {
        return _keyframes[0].value;
    }

    for (size_t index = 0; index < _keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // index と nextIndex の 2つを取得して 現時刻が 範囲内か
        if (_keyframes[index].time <= _time && _time <= _keyframes[nextIndex].time) {
            return _keyframes[index].value;
        }
    }
    // 登録されている時間より 後ろ -> 最後の値を返す
    return _keyframes.back().value;
}

} // namespace CalculateValue

} // namespace OriGine
