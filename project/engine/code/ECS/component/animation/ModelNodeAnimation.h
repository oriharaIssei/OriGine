#pragma once

/// stl
// container
#include <unordered_map>
// string
#include <string>

/// engine
// assets
struct Model;
struct ModelNode;
#include "component/animation/AnimationData.h"
// component
#include "component/IComponent.h"

/// math
#include "math/Quaternion.h"
#include "Matrix4x4.h"
#include "Vector.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

/// <summary>
/// アニメーションの再生を行うクラス
/// </summary>
class ModelNodeAnimation
    : public IComponent {
    friend void to_json(nlohmann::json& j, const ModelNodeAnimation& t);
    friend void from_json(const nlohmann::json& j, ModelNodeAnimation& t);

public:
    ModelNodeAnimation() = default;
    ~ModelNodeAnimation() = default;

    void Initialize(GameEntity* _entity) override;
    void Edit(Scene* _scene,GameEntity* _entity,[[maybe_unused]] const std::string& _parentLabel) override;
    void Finalize() override;

    void UpdateModel(
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
        const ModelNodeAnimation* animation);

private:
    std::string directory_ = ""; // アニメーションファイルのディレクトリ
    std::string fileName_  = ""; // アニメーションファイル名

    std::shared_ptr<AnimationData> data_ = nullptr;

    //* アニメーションの再生時間(data にも あるが instance 毎に変更できるようにこちらで管理する)
    float duration_             = 0.0f; // (秒)
    float currentAnimationTime_ = 0.0f; // アニメーション の 経過時間 (秒)

    AnimationState animationState_;

public:
    bool isPlay() const { return animationState_.isPlay_; }
    void setPlay(bool _isPlay) { animationState_.isPlay_ = _isPlay; }

    bool isEnd() const { return animationState_.isEnd_; }
    void setEnd(bool _isEnd) { animationState_.isEnd_ = _isEnd; }

    bool isLoop() const { return animationState_.isLoop_; }
    void setLoop(bool _isLoop) { animationState_.isLoop_ = _isLoop; }

    float getDuration() const { return duration_; }
    void setDuration(float _duration) { duration_ = _duration; }

    float getCurrentAnimationTime() const { return currentAnimationTime_; }
    void setCurrentAnimationTime(float _currentAnimationTime) { currentAnimationTime_ = _currentAnimationTime; }

    AnimationData* getData() const { return data_.get(); }
    void setData(std::shared_ptr<AnimationData> _data) { data_ = std::move(_data); }

    Vec3f getCurrentScale(const std::string& nodeName) const;
    Quaternion getCurrentRotate(const std::string& nodeName) const;
    Vec3f getCurrentTranslate(const std::string& nodeName) const;
};
