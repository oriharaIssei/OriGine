#pragma once

#include "ModelNodeAnimation.h"
#include "Matrix4x4.h"
#include "model/Model.h"
#include "Quaternion.h"

#include "Vector3.h"

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
    std::shared_ptr<AnimationData> Load(const std::string& directory, const std::string& filename);

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
        std::string directory                        = "";
        std::string filename                         = "";
        std::shared_ptr<AnimationData> animationData = nullptr;

        AnimationLoadTask() = default;
        AnimationLoadTask(const std::string& _directory, const std::string& _filename, std::shared_ptr<AnimationData> _animationData)
            : directory(_directory), filename(_filename), animationData(_animationData) {}
        ~AnimationLoadTask() = default;

        void Update() const;
    };

    // アニメーションデータのライブラリ
    std::unordered_map<std::string, int> animationDataLibrary_;
    std::vector<std::shared_ptr<AnimationData>> animationData_;

public:
    const AnimationData* getAnimationData(const std::string& name) const;
    const AnimationData* getAnimationData(int index) const { return animationData_[index].get(); }

};
