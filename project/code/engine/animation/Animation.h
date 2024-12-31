#pragma once

#include "Matrix4x4.h"
#include "Quaternion.h"
#include "Vector3.h"
#include "globalVariables/SerializedField.h"

#include <string>
#include <unordered_map>

struct Model;
struct ModelNode;

template <typename T>
struct Keyframe{
    Keyframe() = default;
    Keyframe(float time,const T& value)
        : time(time),value(value){}
    ~Keyframe() = default;

    float time; // キーフレームの時刻
    T value;    // キーフレームの 値
};
using KeyframeVector3    = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;

template <typename T>
using AnimationCurve = std::vector<Keyframe<T>>;

struct NodeAnimation{
    AnimationCurve<Vector3> scale;
    AnimationCurve<Quaternion> rotate;
    AnimationCurve<Vector3> translate;
};

/// <summary>
/// アニメーションデータ
/// </summary>
struct AnimationData{
    AnimationData() = default;
    AnimationData(float _duration)
        : duration(duration){}
    ~AnimationData() = default;

    float duration = 0.0f;
    std::unordered_map<std::string,NodeAnimation> nodeAnimations;
};

struct Animation{
    Animation() = default;
    Animation(AnimationData* _data)
        : data(_data){
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
    ///  指定時間の 値を 計算し 取得
    /// </summary>
    /// <param name="keyframes"></param>
    /// <param name="time"></param>
    /// <returns></returns>
    Vector3 CalculateValue(
        const std::vector<KeyframeVector3>& keyframes,float time) const;
    Quaternion CalculateValue(
        const std::vector<KeyframeQuaternion>& keyframes,float time) const;

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
    bool isEnd() const{ return isEnd_; }

    float getDuration() const{ return duration; }
    void setDuration(float _duration){ duration = _duration; }

    float getCurrentAnimationTime() const{ return currentAnimationTime; }
    void setCurrentAnimationTime(float _currentAnimationTime){ currentAnimationTime = _currentAnimationTime; }

    AnimationData* getData() const{ return data; }
    void setData(AnimationData* _data){ data = _data; }

    Vector3 getCurrentScale(const std::string& nodeName)const;
    Quaternion getCurrentRotate(const std::string& nodeName)const;
    Vector3 getCurrentTranslate(const std::string& nodeName)const;
};



struct AnimationSetting{
    AnimationSetting(const std::string& _name)
        : name(_name.c_str()),
        targetModelDirection("Animations",_name,"targetModelDirection"),
        targetModelFileName("Animations",_name,"targetModelFileName"),
        targetAnimationDirection("Animations",_name,"targetAnimationDirection"){
        targetAnimationDirection.setValue("resource/Animations");
    }
    ~AnimationSetting(){}
    // アニメーション名
    std::string name;
    // アニメーション対象のモデル名
    SerializedField<std::string> targetModelDirection;
    SerializedField<std::string> targetModelFileName;
    // アニメーション対象のアニメーション名
    SerializedField<std::string> targetAnimationDirection;
    //! targetAnimationFileName_ = name; とする
};
