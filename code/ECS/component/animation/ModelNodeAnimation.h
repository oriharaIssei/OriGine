#pragma once

/// stl
// container
#include <unordered_map>
// string
#include <string>

/// engin
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

namespace OriGine {
// 前方宣言
struct Model;
struct ModelNode;

/// <summary>
/// ModelNodeアニメーションの再生を行うクラス
/// </summary>
class ModelNodeAnimation
    : public IComponent {
    friend void to_json(nlohmann::json& _j, const ModelNodeAnimation& _comp);
    friend void from_json(const nlohmann::json& _j, ModelNodeAnimation& _comp);

public:
    ModelNodeAnimation()  = default;
    ~ModelNodeAnimation() = default;

    void Initialize(Scene* _scene, EntityHandle _entity) override;
    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;
    void Finalize() override;

    void UpdateModel(
        float _deltaTime,
        Model* _model,
        const Matrix4x4& _parentTransform);

private:
    /// <summary>
    /// Nodeアニメーションの現在のローカル行列を計算
    /// </summary>
    Matrix4x4 CalculateNodeLocal(const std::string& _nodeName) const;

    /// <summary>
    /// ノードにアニメーションを適用
    /// </summary>
    /// <param name="_node">root Node</param>
    /// <param name="_parentTransform">rootNode ParentMatrix</param>
    /// <param name="_animation">animation</param>
    void ApplyAnimationToNodes(
        ModelNode& _node,
        const Matrix4x4& _parentTransform,
        const ModelNodeAnimation* _animation);

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

    Vec3f GetCurrentScale(const std::string& _nodeName) const;
    Quaternion GetCurrentRotate(const std::string& _nodeName) const;
    Vec3f GetCurrentTranslate(const std::string& _nodeName) const;
};

} // namespace OriGine
