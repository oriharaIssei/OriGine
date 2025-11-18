#pragma once
#include "component/IComponent.h"

/// stl
#include <memory>
#include <string>

/// engine
#include "AnimationData.h"
#include "model/Model.h"

class Scene;

/// <summary>
/// スキニングアニメーションコンポーネント
/// </summary>
class SkinningAnimationComponent
    : public IComponent {
    friend void to_json(nlohmann::json& j, const SkinningAnimationComponent& r);
    friend void from_json(const nlohmann::json& j, SkinningAnimationComponent& r);

public:
    SkinningAnimationComponent()           = default;
    ~SkinningAnimationComponent() override = default;

    void Initialize(Entity* _entity) override;
    void Edit(Scene* _scene, Entity* _entity, const std::string& _parentLabel);
    void Finalize() override;

    /// <summary>
    /// Animationを追加,読み込みする
    /// </summary>
    /// <param name="directory"></param>
    /// <param name="fileName"></param>
    void AddLoad(const std::string& directory, const std::string& fileName);

    /// <summary>
    /// currentAnimationIndex_ のアニメーションを再生する
    /// </summary>
    void Play();
    /// <summary>
    /// 指定したインデックスのアニメーションを再生する
    /// </summary>
    /// <param name="index">animationのIndexを指定する</param>
    void Play(int32_t index);
    /// <summary>
    /// 指定した名前のアニメーションを再生する
    /// </summary>
    /// <param name="name">animationの名前を指定する</param>
    void Play(const std::string& name);
    /// <summary>
    /// 指定したインデックスのアニメーションを次のアニメーションにセットする
    /// </summary>
    /// <param name="index">nextAnimationのIndexを指定する</param>
    /// <param name="_blendTime">ブレンドにかける時間(秒)</param>
    void PlayNext(int32_t index, float _blendTime = 0.1f);
    /// <summary>
    /// 指定した名前のアニメーションを次のアニメーションにセットする
    /// </summary>
    /// <param name="name">nextAnimationの名前を指定する</param>
    /// <param name="_blendTime">ブレンドにかける時間(秒)</param>
    void PlayNext(const std::string& name, float _blendTime = -0.1f);

    /// <summary>
    /// 再生中のアニメーションを停止する
    /// </summary>
    void Stop();

    /// <summary>
    /// スキニングされた頂点バッファを作成する
    /// </summary>
    void CreateSkinnedVertex(Scene* _scene);
    /// <summary>
    /// スキニングされた頂点バッファを削除する
    /// </summary>
    void DeleteSkinnedVertex();

public:
    struct UavBuffer {
        DxUavDescriptor descriptor;
        D3D12_VERTEX_BUFFER_VIEW vbView{};
        DxResource buffer;
    };

private:
    struct AnimationCombo {
        std::string directory_                        = "";
        std::string fileName_                         = "";
        std::shared_ptr<AnimationData> animationData_ = nullptr;

        bool prePlay_                  = false; // 前のアニメーションの状態
        AnimationState animationState_ = {false, false, false}; // アニメーションの状態
        float duration_                = 0.0f;
        float currentTime_             = 0.0f;
        float playbackSpeed_           = 1.0f; // 再生速度
    };
    struct AnimationBlendData {
        int32_t targetAnimationIndex_ = -1; // 対象のアニメーションインデックス
        float blendDuration_          = 0.1f; // ブレンドにかかる時間
        float currentTime_            = 0.0f; // 現在のブレンド時間
    };

private:
    Entity* entity_                = nullptr;
    int32_t bindModeMeshRendererIndex_ = -1;

    std::unordered_map<std::string, int32_t> animationIndexBinder_;
    std::vector<AnimationCombo> animationTable_; // アニメーションの組み合わせ

    int32_t currentAnimationIndex_                           = 0; // 現在のアニメーションインデックス
    std::optional<AnimationBlendData> blendingAnimationData_ = std::nullopt; // 次のアニメーションインデックス (存在しない場合は nullopt)

    std::vector<UavBuffer> skinnedVertexBuffer_; // スキニングされた頂点バッファ. size = meshSize

    Skeleton skeleton_;

public:
    const std::vector<AnimationCombo>& GetAnimationTable() const {
        return animationTable_;
    }
    const std::unordered_map<std::string, int32_t>& GetAnimationIndexBinder() const {
        return animationIndexBinder_;
    }

    const std::vector<UavBuffer>& GetSkinnedVertexBuffers() const {
        return skinnedVertexBuffer_;
    }

    const UavBuffer& GetSkinnedVertexBuffer(int32_t index) const {
        return skinnedVertexBuffer_.at(index);
    }

    int32_t GetAnimationIndex(const std::string& name) const {
        auto it = animationIndexBinder_.find(name);
        if (it != animationIndexBinder_.end()) {
            return it->second;
        }
        return -1;
    }

    bool IsTransitioning() const {
        return blendingAnimationData_.has_value();
    }
    int32_t GetNextAnimationIndex() const {
        return blendingAnimationData_.has_value() ? blendingAnimationData_->targetAnimationIndex_ : -1;
    }
    float GetBlendTime() const {
        return blendingAnimationData_.has_value() ? blendingAnimationData_->blendDuration_ : 0.0f;
    }
    void SetBlendTime(float time) {
        if (blendingAnimationData_.has_value()) {
            blendingAnimationData_->blendDuration_ = time;
        }
    }
    float GetBlendCurrentTime() const {
        return blendingAnimationData_.has_value() ? blendingAnimationData_->currentTime_ : 0.0f;
    }
    void SetBlendCurrentTime(float time) {
        if (blendingAnimationData_.has_value()) {
            blendingAnimationData_->currentTime_ = time;
        }
    }
    void EndTransition() {
        if (!blendingAnimationData_.has_value()) {
            LOG_WARN("No blending animation data to end.");
            return;
        }
        currentAnimationIndex_                                          = blendingAnimationData_.value().targetAnimationIndex_; // トランジション後のアニメーションインデックスを設定
        animationTable_[currentAnimationIndex_].animationState_.isPlay_ = true;
        animationTable_[currentAnimationIndex_].animationState_.isEnd_  = false;

        blendingAnimationData_ = std::nullopt; // トランジションを終了
    }

    int32_t GetCurrentAnimationIndex() const {
        return currentAnimationIndex_;
    }
    void SetCurrentAnimationIndex(int32_t index) {
        if (index >= 0 && index < static_cast<int32_t>(animationTable_.size())) {
            currentAnimationIndex_ = index;
        }
    }

    const std::string& GetDirectory(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].directory_; }
    const std::string& GetFileName(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].fileName_; }
    const std::shared_ptr<AnimationData>& GetAnimationData(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].animationData_; }
    const Skeleton& GetSkeleton() const {
        return skeleton_;
    }
    Skeleton& GetSkeletonRef() { return skeleton_; }

    bool IsPlay(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].animationState_.isPlay_; }
    bool IsLoop(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].animationState_.isLoop_; }
    bool IsEnd(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].animationState_.isEnd_; }
    void SetIsPlay(int32_t _animationIndex = 0, bool isPlay = false) { animationTable_[_animationIndex].animationState_.isPlay_ = isPlay; }
    void SetIsLoop(int32_t _animationIndex = 0, bool isLoop = false) { animationTable_[_animationIndex].animationState_.isLoop_ = isLoop; }
    void SetIsEnd(int32_t _animationIndex = 0, bool isEnd = false) { animationTable_[_animationIndex].animationState_.isEnd_ = isEnd; }

    bool IsPrePlay(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].prePlay_; }
    void SetIsPrePlay(int32_t _animationIndex = 0, bool isPlay = false) { animationTable_[_animationIndex].prePlay_ = isPlay; }

    int32_t GetBindModeMeshRendererIndex() const { return bindModeMeshRendererIndex_; }
    void SetBindModeMeshRendererIndex(int32_t index) { bindModeMeshRendererIndex_ = index; }

    float GetAnimationDuration(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].duration_; }
    float GetAnimationCurrentTime(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].currentTime_; }
    float GetPlaybackSpeed(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].playbackSpeed_; }
    void SetAnimationDuration(int32_t _animationIndex = 0, float duration = 1.f) { animationTable_[_animationIndex].duration_ = duration; }
    void SetAnimationCurrentTime(int32_t _animationIndex = 0, float time = 0.f) { animationTable_[_animationIndex].currentTime_ = time; }
    void SetPlaybackSpeed(int32_t _animationIndex = 0, float speed = 1.f) { animationTable_[_animationIndex].playbackSpeed_ = speed; }

    const std::string& GetDirectory(const std::string& name) const {
        int32_t idx = GetAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            return animationTable_[idx].directory_;
        }
        static const std::string empty = "";
        return empty;
    }

    const std::string& GetFileName(const std::string& name) const {
        int32_t idx = GetAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            return animationTable_[idx].fileName_;
        }
        static const std::string empty = "";
        return empty;
    }

    const std::shared_ptr<AnimationData>& GetAnimationData(const std::string& name) const {
        int32_t idx = GetAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            return animationTable_[idx].animationData_;
        }
        static const std::shared_ptr<AnimationData> nullData = nullptr;
        return nullData;
    }

    bool IsPlay(const std::string& name) const {
        int32_t idx = GetAnimationIndex(name);
        return (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) ? animationTable_[idx].animationState_.isPlay_ : false;
    }
    bool IsLoop(const std::string& name) const {
        int32_t idx = GetAnimationIndex(name);
        return (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) ? animationTable_[idx].animationState_.isLoop_ : false;
    }
    bool IsEnd(const std::string& name) const {
        int32_t idx = GetAnimationIndex(name);
        return (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) ? animationTable_[idx].animationState_.isEnd_ : false;
    }

    void SetIsPlay(const std::string& name, bool isPlay) {
        int32_t idx = GetAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            animationTable_[idx].animationState_.isPlay_ = isPlay;
        }
    }
    void SetIsLoop(const std::string& name, bool isLoop) {
        int32_t idx = GetAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            animationTable_[idx].animationState_.isLoop_ = isLoop;
        }
    }
    void SetIsEnd(const std::string& name, bool isEnd) {
        int32_t idx = GetAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            animationTable_[idx].animationState_.isEnd_ = isEnd;
        }
    }

    float GetAnimationDuration(const std::string& name) const {
        int32_t idx = GetAnimationIndex(name);
        return (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) ? animationTable_[idx].duration_ : 0.0f;
    }
    float GetAnimationCurrentTime(const std::string& name) const {
        int32_t idx = GetAnimationIndex(name);
        return (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) ? animationTable_[idx].currentTime_ : 0.0f;
    }
    float GetPlaybackSpeed(const std::string& name) const {
        int32_t idx = GetAnimationIndex(name);
        return (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) ? animationTable_[idx].playbackSpeed_ : 1.0f;
    }
    void SetAnimationDuration(const std::string& name, float duration) {
        int32_t idx = GetAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            animationTable_[idx].duration_ = duration;
        }
    }
    void SetAnimationCurrentTime(const std::string& name, float time) {
        int32_t idx = GetAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            animationTable_[idx].currentTime_ = time;
        }
    }
    void SetPlaybackSpeed(const std::string& name, float speed) {
        int32_t idx = GetAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            animationTable_[idx].playbackSpeed_ = speed;
        }
    }
};
