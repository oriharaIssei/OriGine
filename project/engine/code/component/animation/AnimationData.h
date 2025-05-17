#pragma once

/// stl
#include <string>
#include <unordered_map>

/// engine
#include "Thread/Thread.h"

/// math
#include <Quaternion.h>
#include <Vector.h>
#include <Vector3.h>
#include <Vector4.h>

/// lib
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
    bool isLoop_ = false;
    bool isPlay_ = false;
    bool isEnd_  = false;
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

    LoadState loadState = LoadState::Unloaded;
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

/// =====================================================
// Write
/// =====================================================
inline void WriteCurve(const std::string& _curveName, const AnimationCurve<float>& curve, BinaryWriter& _writer) {
    _curveName;
    curve;
    _writer;
    /* size_t size = curve.size();
    _bin.setData("size", size);

    int32_t index        = 0;
    std::string indexStr = "";
    for (const auto& keyframe : curve) {
        BinaryItem keyBin;
        indexStr = std::to_string(index++);

        keyBin.setData("time", keyframe.time);
        keyBin.setData("value", keyframe.value);

        _bin.setData(_curveName + indexStr, keyBin);
    }*/
}
inline void WriteCurve(const std::string& _curveName, const AnimationCurve<Vector2f>& curve, BinaryWriter& _writer) {
    _curveName;
    curve;
    _writer;
    /* size_t size = curve.size();
    _bin.setData("size", size);

    int32_t index        = 0;
    std::string indexStr = "";
    for (const auto& keyframe : curve) {
        BinaryItem keyBin;
        indexStr = std::to_string(index++);

        keyBin.setData("time", keyframe.time);
        keyBin.setData("value", keyframe.value);

        _bin.setData(_curveName + indexStr, keyBin);
    }*/
}
inline void WriteCurve(const std::string& _curveName, const AnimationCurve<Vector3f>& curve, BinaryWriter& _writer) {
    _curveName;
    curve;
    _writer;
    /*size_t size = curve.size();
    _bin.setData("size", size);

    int32_t index        = 0;
    std::string indexStr = "";
    for (const auto& keyframe : curve) {
        BinaryItem keyBin;
        indexStr = std::to_string(index++);

        keyBin.setData("time", keyframe.time);
        keyBin.setData("value", keyframe.value);

        _bin.setData(_curveName + indexStr, keyBin);
    }*/
}
inline void WriteCurve(const std::string& _curveName, const AnimationCurve<Vector4f>& curve, BinaryWriter& _writer) {
    _curveName;
    curve;
    _writer;
    /*size_t size = curve.size();
    _bin.setData("size", size);

    int32_t index        = 0;
    std::string indexStr = "";
    for (const auto& keyframe : curve) {
        BinaryItem keyBin;
        indexStr = std::to_string(index++);

        keyBin.setData("time", keyframe.time);
        keyBin.setData("value", keyframe.value);

        _bin.setData(_curveName + indexStr, keyBin);
    }*/
}
inline void WriteCurve(const std::string& _curveName, const AnimationCurve<Quaternion>& curve, BinaryWriter& _writer) {
    _curveName;
    curve;
    _writer;
    /*size_t size = curve.size();
    _bin.setData("size", size);

    int32_t index        = 0;
    std::string indexStr = "";
    for (const auto& keyframe : curve) {
        BinaryItem keyBin;
        indexStr = std::to_string(index++);

        keyBin.setData("time", keyframe.time);
        keyBin.setData("value", keyframe.value);

        _bin.setData(_curveName + indexStr, keyBin);
    }*/
}

/// =====================================================
// Read
/// =====================================================
inline void ReadCurve(const std::string& _curveName, AnimationCurve<float>& curve, BinaryReader& _reader) {
    _curveName;
    curve;
    _reader;
    /*size_t size;
    size = _bin.getData<size_t>("size");
    curve.resize(size);

    int32_t index        = 0;
    std::string indexStr = "";
    for (auto& keyframe : curve) {
        indexStr = std::to_string(index);

        BinaryItem child = _bin.getChild(_curveName + indexStr);
        keyframe.time    = child.getData<float>("time");
        keyframe.value   = child.getData<float>("value");

        ++index;
    }*/
}
inline void ReadCurve(const std::string& _curveName, AnimationCurve<Vector2f>& curve, BinaryReader& _reader) {
    _curveName;
    curve;
    _reader;
    /* size_t size;
    size = _bin.getData<size_t>("size");
    curve.resize(size);

    int32_t index        = 0;
    std::string indexStr = "";
    for (auto& keyframe : curve) {
        indexStr = std::to_string(index);

        BinaryItem child = _bin.getChild(_curveName + indexStr);
        keyframe.time    = child.getData<float>("time");
        keyframe.value   = child.getData<Vector2f>("value");

        ++index;
    }*/
}
inline void ReadCurve(const std::string& _curveName, AnimationCurve<Vector3f>& curve, BinaryReader& _reader) {
    _curveName;
    curve;
    _reader;
    /*size_t size;
    size = _bin.getData<size_t>("size");
    curve.resize(size);

    int32_t index        = 0;
    std::string indexStr = "";
    for (auto& keyframe : curve) {
        indexStr = std::to_string(index);

        BinaryItem child = _bin.getChild(_curveName + indexStr);
        keyframe.time    = child.getData<float>("time");
        keyframe.value   = child.getData<Vector3f>("value");

        ++index;
    }*/
}
inline void ReadCurve(const std::string& _curveName, AnimationCurve<Vector4f>& curve, BinaryReader& _reader) {
    _curveName;
    curve;
    _reader;
    /*size_t size;
    size = _bin.getData<size_t>("size");
    curve.resize(size);

    int32_t index        = 0;
    std::string indexStr = "";
    for (auto& keyframe : curve) {
        indexStr = std::to_string(index);

        BinaryItem child = _bin.getChild(_curveName + indexStr);
        keyframe.time    = child.getData<float>("time");
        keyframe.value   = child.getData<Vector4f>("value");

        ++index;
    }*/
}
inline void ReadCurve(const std::string& _curveName, AnimationCurve<Quaternion>& curve, BinaryReader& _reader) {
    _curveName;
    curve;
    _reader;
    /* size_t size;
    size = _bin.getData<size_t>("size");
    curve.resize(size);

    int32_t index        = 0;
    std::string indexStr = "";
    for (auto& keyframe : curve) {
        indexStr = std::to_string(index);

        BinaryItem child = _bin.getChild(_curveName + indexStr);
        keyframe.time    = child.getData<float>("time");
        keyframe.value   = child.getData<Quaternion>("value");

        ++index;
    }*/
}
