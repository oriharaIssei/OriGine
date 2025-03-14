#pragma once

#include "Animation.h"
#include "Matrix4x4.h"
#include "Quaternion.h"
#include "Thread/Thread.h"
#include "Vector3.h"
#include "model/Model.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/// <summary>
/// アニメーションのみを 管理するクラス(Modelは含まれない)
/// </summary>
class AnimationManager {
public:
    static AnimationManager* getInstance() {
        static AnimationManager instance;
        return &instance;
    }

    void Initialize();
    void Finalize();

    /// <summary>
    /// アニメーションの読み込み,作成
    ///< param name="directory">ディレクトリ</param>
    ///< param name="filename">ファイル名(format を つける .gltf)</param>
    std::unique_ptr<Animation> Load(const std::string& directory, const std::string& filename);

    /// <summary>
    /// アニメーションの保存
    ///< param name="directory">ディレクトリ</param>
    ///< param name="filename">ファイル名(format を つけない .anm 固定)</param>
    void SaveAnimation(const std::string& directory, const std::string& filename, const AnimationData& animationData);

    int addAnimationData(const std::string& name, std::unique_ptr<AnimationData> animationData);

private:
    /// <summary>
    /// アニメーションデータの読み込み
    ///< param name="directory">ディレクトリ</param>
    ///< param name="filename">ファイル名(format を つける .gltf)</param>
    AnimationData LoadAnimationData(const std::string& directory, const std::string& filename);
    /// <summary>
    /// アニメーションデータの読み込み
    ///< param name="directory">ディレクトリ</param>
    ///< param name="filename">ファイル名(format は つけない .gltf固定)</param>
    AnimationData LoadGltfAnimationData(const std::string& directory, const std::string& filename);
    /// <summary>
    /// アニメーションデータの読み込み
    ///< param name="directory">ディレクトリ</param>
    ///< param name="filename">ファイル名(format を つけない .anm固定)</param>
    AnimationData LoadMyAnimationData(const std::string& directory, const std::string& filename);

private:
    AnimationManager();
    ~AnimationManager();
    AnimationManager& operator=(const AnimationManager&) = delete;
    AnimationManager(const AnimationManager&)            = delete;

    struct AnimationLoadTask {
        std::string directory        = "";
        std::string filename         = "";
        AnimationData* animationData = nullptr;
        Animation* animation         = nullptr;

        AnimationLoadTask() = default;
        AnimationLoadTask(const std::string& _directory, const std::string& _filename, AnimationData* _animationData, Animation* _animation)
            : directory(_directory), filename(_filename), animationData(_animationData), animation(_animation) {}
        ~AnimationLoadTask() = default;

        void Update();
    };

    std::unique_ptr<TaskThread<AnimationLoadTask>> loadThread_ = nullptr;

    // アニメーションデータのライブラリ
    std::unordered_map<std::string, int> animationDataLibrary_;
    std::vector<std::unique_ptr<AnimationData>> animationData_;

    // 再生中のアニメーション
    std::vector<Animation*> playingAnimations_;

public:
    const AnimationData* getAnimationData(const std::string& name) const;
    const AnimationData* getAnimationData(int index) const { return animationData_[index].get(); }

    void Play(Animation* animation) { playingAnimations_.push_back(animation); }
};
