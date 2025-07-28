#pragma once
#include "component/IComponent.h"

/// stl
#include <memory>
#include <string>

/// engine
#include "AnimationData.h"
#include "model/Model.h"

class Scene;

class SkinningAnimationComponent
    : public IComponent {
    friend void to_json(nlohmann::json& j, const SkinningAnimationComponent& r);
    friend void from_json(const nlohmann::json& j, SkinningAnimationComponent& r);

public:
    SkinningAnimationComponent()           = default;
    ~SkinningAnimationComponent() override = default;

    void Initialize(GameEntity* _entity) override;
    void Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel);
    void Finalize() override;

    /// <summary>
    /// Animationを追加,読み込みする
    /// </summary>
    /// <param name="directory"></param>
    /// <param name="fileName"></param>
    void addLoad(const std::string& directory, const std::string& fileName);

    void Play();
    void Play( int32_t index);
    void Play( const std::string& name);
    void PlayNext(int32_t index, float _blendTime = 0.1f);
    void PlayNext(const std::string& name, float _blendTime = -0.1f);

    void Stop();

    void CreateSkinnedVertex(Scene* _scene);
    void DeleteSkinnedVertex();

public:
    struct UavBuffer {
        std::shared_ptr<DxUavDescriptor> descriptor;
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
    GameEntity* entity_                = nullptr;
    int32_t bindModeMeshRendererIndex_ = -1;

    std::unordered_map<std::string, int32_t> animationIndexBinder_;
    std::vector<AnimationCombo> animationTable_; // アニメーションの組み合わせ

    int32_t currentAnimationIndex_                           = 0; // 現在のアニメーションインデックス
    std::optional<AnimationBlendData> blendingAnimationData_ = std::nullopt; // 次のアニメーションインデックス (存在しない場合は nullopt)

    std::vector<UavBuffer> skinnedVertexBuffer_; // スキニングされた頂点バッファ. size = meshSize

    Skeleton skeleton_;

public:
    const std::vector<AnimationCombo>& getAnimationTable() const {
        return animationTable_;
    }
    const std::unordered_map<std::string, int32_t>& getAnimationIndexBinder() const {
        return animationIndexBinder_;
    }

    const std::vector<UavBuffer>& getSkinnedVertexBuffers() const {
        return skinnedVertexBuffer_;
    }

    const UavBuffer& getSkinnedVertexBuffer(int32_t index) const {
        return skinnedVertexBuffer_.at(index);
    }

    int32_t getAnimationIndex(const std::string& name) const {
        auto it = animationIndexBinder_.find(name);
        if (it != animationIndexBinder_.end()) {
            return it->second;
        }
        return -1;
    }

    bool isTransitioning() const {
        return blendingAnimationData_.has_value();
    }
    int32_t getNextAnimationIndex() const {
        return blendingAnimationData_.has_value() ? blendingAnimationData_->targetAnimationIndex_ : -1;
    }
    float getBlendTime() const {
        return blendingAnimationData_.has_value() ? blendingAnimationData_->blendDuration_ : 0.0f;
    }
    void setBlendTime(float time) {
        if (blendingAnimationData_.has_value()) {
            blendingAnimationData_->blendDuration_ = time;
        }
    }
    float getBlendCurrentTime() const {
        return blendingAnimationData_.has_value() ? blendingAnimationData_->currentTime_ : 0.0f;
    }
    void setBlendCurrentTime(float time) {
        if (blendingAnimationData_.has_value()) {
            blendingAnimationData_->currentTime_ = time;
        }
    }
    void endTransition() {
        if (!blendingAnimationData_.has_value()) {
            LOG_WARN("No blending animation data to end.");
            return;
        }
        currentAnimationIndex_                                          = blendingAnimationData_.value().targetAnimationIndex_; // トランジション後のアニメーションインデックスを設定
        animationTable_[currentAnimationIndex_].animationState_.isPlay_ = true;
        animationTable_[currentAnimationIndex_].animationState_.isEnd_  = false;

        blendingAnimationData_ = std::nullopt; // トランジションを終了
    }

    int32_t getCurrentAnimationIndex() const {
        return currentAnimationIndex_;
    }
    void setCurrentAnimationIndex(int32_t index) {
        if (index >= 0 && index < static_cast<int32_t>(animationTable_.size())) {
            currentAnimationIndex_ = index;
        }
    }

    const std::string& getDirectory(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].directory_; }
    const std::string& getFileName(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].fileName_; }
    const std::shared_ptr<AnimationData>& getAnimationData(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].animationData_; }
    const Skeleton& getSkeleton() const {
        return skeleton_;
    }
    Skeleton& getSkeletonRef() { return skeleton_; }

    bool isPlay(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].animationState_.isPlay_; }
    bool isLoop(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].animationState_.isLoop_; }
    bool isEnd(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].animationState_.isEnd_; }
    void setIsPlay(int32_t _animationIndex = 0, bool isPlay = false) { animationTable_[_animationIndex].animationState_.isPlay_ = isPlay; }
    void setIsLoop(int32_t _animationIndex = 0, bool isLoop = false) { animationTable_[_animationIndex].animationState_.isLoop_ = isLoop; }
    void setIsEnd(int32_t _animationIndex = 0, bool isEnd = false) { animationTable_[_animationIndex].animationState_.isEnd_ = isEnd; }

    bool isPrePlay(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].prePlay_; }
    void setIsPrePlay(int32_t _animationIndex = 0, bool isPlay = false) { animationTable_[_animationIndex].prePlay_ = isPlay; }

    int32_t getBindModeMeshRendererIndex() const { return bindModeMeshRendererIndex_; }
    void setBindModeMeshRendererIndex(int32_t index) { bindModeMeshRendererIndex_ = index; }

    float getDuration(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].duration_; }
    float getCurrentTime(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].currentTime_; }
    float getPlaybackSpeed(int32_t _animationIndex = 0) const { return animationTable_[_animationIndex].playbackSpeed_; }
    void setDuration(int32_t _animationIndex = 0, float duration = 1.f) { animationTable_[_animationIndex].duration_ = duration; }
    void setCurrentTime(int32_t _animationIndex = 0, float time = 0.f) { animationTable_[_animationIndex].currentTime_ = time; }
    void setPlaybackSpeed(int32_t _animationIndex = 0, float speed = 1.f) { animationTable_[_animationIndex].playbackSpeed_ = speed; }

    const std::string& getDirectory(const std::string& name) const {
        int32_t idx = getAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            return animationTable_[idx].directory_;
        }
        static const std::string empty = "";
        return empty;
    }

    const std::string& getFileName(const std::string& name) const {
        int32_t idx = getAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            return animationTable_[idx].fileName_;
        }
        static const std::string empty = "";
        return empty;
    }

    const std::shared_ptr<AnimationData>& getAnimationData(const std::string& name) const {
        int32_t idx = getAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            return animationTable_[idx].animationData_;
        }
        static const std::shared_ptr<AnimationData> nullData = nullptr;
        return nullData;
    }

    bool isPlay(const std::string& name) const {
        int32_t idx = getAnimationIndex(name);
        return (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) ? animationTable_[idx].animationState_.isPlay_ : false;
    }
    bool isLoop(const std::string& name) const {
        int32_t idx = getAnimationIndex(name);
        return (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) ? animationTable_[idx].animationState_.isLoop_ : false;
    }
    bool isEnd(const std::string& name) const {
        int32_t idx = getAnimationIndex(name);
        return (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) ? animationTable_[idx].animationState_.isEnd_ : false;
    }

    void setIsPlay(const std::string& name, bool isPlay) {
        int32_t idx = getAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            animationTable_[idx].animationState_.isPlay_ = isPlay;
        }
    }
    void setIsLoop(const std::string& name, bool isLoop) {
        int32_t idx = getAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            animationTable_[idx].animationState_.isLoop_ = isLoop;
        }
    }
    void setIsEnd(const std::string& name, bool isEnd) {
        int32_t idx = getAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            animationTable_[idx].animationState_.isEnd_ = isEnd;
        }
    }

    float getDuration(const std::string& name) const {
        int32_t idx = getAnimationIndex(name);
        return (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) ? animationTable_[idx].duration_ : 0.0f;
    }
    float getCurrentTime(const std::string& name) const {
        int32_t idx = getAnimationIndex(name);
        return (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) ? animationTable_[idx].currentTime_ : 0.0f;
    }
    float getPlaybackSpeed(const std::string& name) const {
        int32_t idx = getAnimationIndex(name);
        return (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) ? animationTable_[idx].playbackSpeed_ : 1.0f;
    }
    void setDuration(const std::string& name, float duration) {
        int32_t idx = getAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            animationTable_[idx].duration_ = duration;
        }
    }
    void setCurrentTime(const std::string& name, float time) {
        int32_t idx = getAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            animationTable_[idx].currentTime_ = time;
        }
    }
    void setPlaybackSpeed(const std::string& name, float speed) {
        int32_t idx = getAnimationIndex(name);
        if (idx >= 0 && idx < static_cast<int32_t>(animationTable_.size())) {
            animationTable_[idx].playbackSpeed_ = speed;
        }
    }
};
