#pragma once
#include "component/IComponent.h"

/// stl
#include <memory>
#include <string>

/// engine
#include "AnimationData.h"
#include "model/Model.h"

class SkinningAnimationComponent
    : public IComponent {
    friend void to_json(nlohmann::json& j, const SkinningAnimationComponent& r);
    friend void from_json(const nlohmann::json& j, SkinningAnimationComponent& r);

public:
    SkinningAnimationComponent()           = default;
    ~SkinningAnimationComponent() override = default;

    void Initialize(GameEntity* _entity) override;
    bool Edit();
    void Finalize() override;

    void Load(const std::string& directory, const std::string& fileName);

    void Play();
    void Stop();

    void CreateSkinnedVertex();
    void DeleteSkinnedVertex();

public:
    struct UavBuffer {
        std::shared_ptr<DxUavDescriptor> descriptor;
        D3D12_VERTEX_BUFFER_VIEW vbView{};
        DxResource buffer;
    };

private:
    GameEntity* entity_                = nullptr;
    int32_t bindModeMeshRendererIndex_ = -1;

    std::string directory_                        = "";
    std::string fileName_                         = "";
    std::shared_ptr<AnimationData> animationData_ = nullptr;

    AnimationState animationState_ = {false, false, false}; // アニメーションの状態
    float duration_                = 0.0f;
    float currentTime_             = 0.0f;
    float playbackSpeed_           = 1.0f; // 再生速度

    std::vector<UavBuffer> skinnedVertexBuffer_; // スキニングされた頂点バッファ. size = meshSize

    Skeleton skeleton_;

public:
    const std::string& getDirectory() const { return directory_; }
    const std::string& getFileName() const { return fileName_; }
    const std::shared_ptr<AnimationData>& getAnimationData() const { return animationData_; }
    const Skeleton& getSkeleton() const { return skeleton_; }
    Skeleton& getSkeletonRef() { return skeleton_; }

    const UavBuffer& getSkinnedVertexBuffer(int32_t index) const {
        return skinnedVertexBuffer_.at(index);
    }

    bool isPlay() const { return animationState_.isPlay_; }
    bool isLoop() const { return animationState_.isLoop_; }
    bool isEnd() const { return animationState_.isEnd_; }
    void setIsPlay(bool isPlay) { animationState_.isPlay_ = isPlay; }
    void setIsLoop(bool isLoop) { animationState_.isLoop_ = isLoop; }
    void setIsEnd(bool isEnd) { animationState_.isEnd_ = isEnd; }

    int32_t getBindModeMeshRendererIndex() const { return bindModeMeshRendererIndex_; }
    void setBindModeMeshRendererIndex(int32_t index) { bindModeMeshRendererIndex_ = index; }

    float getDuration() const { return duration_; }
    float getCurrentTime() const { return currentTime_; }
    float getPlaybackSpeed() const { return playbackSpeed_; }
    void setDuration(float duration) { duration_ = duration; }
    void setCurrentTime(float time) { currentTime_ = time; }
    void setPlaybackSpeed(float speed) { playbackSpeed_ = speed; }
};
