#pragma once

/// stl
// container
#include <unordered_map>
// string
#include <string>

/// engine
// asSets
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
/// ModelNodeアニメーションの再生を行うクラス
/// </summary>
class ModelNodeAnimation
    : public OriGine::IComponent {
    friend void to_json(nlohmann::json& j, const ModelNodeAnimation& t);
    friend void from_json(const nlohmann::json& j, ModelNodeAnimation& t);

public:
    ModelNodeAnimation() = default;
    ~ModelNodeAnimation() = default;

    void Initialize(Entity* _entity) override;
    void Edit(Scene* _scene,Entity* _entity,[[maybe_unused]] const std::string& _parentLabel) override;
    void Finalize() override;

    void UpdateModel(
        float deltaTime,
        Model* model,
        const Matrix4x4& parentTransform);

private:
    /// <summary>
    /// Nodeアニメーションの現在のローカル行列を計算
    /// </summary>
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
    bool IsPlay() const { return animationState_.isPlay_; }
    void SetPlay(bool _isPlay) { animationState_.isPlay_ = _isPlay; }

    bool IsEnd() const { return animationState_.isEnd_; }
    void SetEnd(bool _isEnd) { animationState_.isEnd_ = _isEnd; }

    bool IsLoop() const { return animationState_.isLoop_; }
    void SetLoop(bool _isLoop) { animationState_.isLoop_ = _isLoop; }

    float GetDuration() const { return duration_; }
    void SetDuration(float _duration) { duration_ = _duration; }

    float GetCurrentAnimationTime() const { return currentAnimationTime_; }
    void SetCurrentAnimationTime(float _currentAnimationTime) { currentAnimationTime_ = _currentAnimationTime; }

    AnimationData* GetData() const { return data_.get(); }
    void SetData(std::shared_ptr<AnimationData> _data) { data_ = std::move(_data); }

    Vec3f GetCurrentScale(const std::string& nodeName) const;
    Quaternion GetCurrentRotate(const std::string& nodeName) const;
    Vec3f GetCurrentTranslate(const std::string& nodeName) const;
};
