#pragma once

/// stl
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/// engine
#include "model/Model.h"
#include "ModelNodeAnimation.h"

/// math
#include "Matrix4x4.h"
#include "Quaternion.h"
#include "Vector3.h"

namespace OriGine {

/// <summary>
/// アニメーションのみを 管理するクラス(Modelは含まれない)
/// </summary>
class AnimationManager {
public:
    static AnimationManager* GetInstance() {
        static AnimationManager instance;
        return &instance;
    }

    void Initialize();
    void Finalize();

    /// <summary>
    /// アニメーションの読み込み,作成
    /// <param name="_directory">ディレクトリ</param>
    /// <param name="_filename">ファイル名(format を つける .gltf)</param>
    std::shared_ptr<AnimationData> Load(const std::string& _directory, const std::string& _filename);

    /// <summary>
    /// アニメーションの保存
    /// <param name="_directory">ディレクトリ</param>
    /// <param name="_filename">ファイル名(format を つけない .anm 固定)</param>
    void SaveAnimation(const std::string& _directory, const std::string& _filename, const AnimationData& _animationData);

    int addAnimationData(const std::string& _name, std::unique_ptr<AnimationData> _animationData);

private:
    /// <summary>
    /// アニメーションデータの読み込み
    /// <param name="_directory">ディレクトリ</param>
    /// <param name="_filename">ファイル名(format を つける .gltf)</param>
    AnimationData LoadAnimationData(const std::string& _directory, const std::string& _filename);
    /// <summary>
    /// アニメーションデータの読み込み
    /// <param name="_directory">ディレクトリ</param>
    /// <param name="_filename">ファイル名(format は つけない .gltf固定)</param>
    AnimationData LoadGltfAnimationData(const std::string& _directory, const std::string& _filename);
    /// <summary>
    /// アニメーションデータの読み込み
    /// <param name="_directory">ディレクトリ</param>
    /// <param name="_filename">ファイル名(format を つけない .anm固定)</param>
    AnimationData LoadMyAnimationData(const std::string& _directory, const std::string& _filename);

private:
    AnimationManager();
    ~AnimationManager();
    AnimationManager& operator=(const AnimationManager&) = delete;
    AnimationManager(const AnimationManager&)            = delete;

    struct AnimationLoadTask {
        AnimationLoadTask() = default;
        AnimationLoadTask(const std::string& _directory, const std::string& _filename, std::shared_ptr<AnimationData> _animationData)
            : directory(_directory), filename(_filename), animationData(_animationData) {}
        ~AnimationLoadTask() = default;

        void Update() const;

        std::string directory                        = "";
        std::string filename                         = "";
        std::shared_ptr<AnimationData> animationData = nullptr;
    };

    // アニメーションデータのライブラリ
    std::unordered_map<std::string, int> animationDataLibrary_;
    std::vector<std::shared_ptr<AnimationData>> animationData_;

public:
    const AnimationData* GetAnimationData(const std::string& _name) const;
    const AnimationData* GetAnimationData(int _index) const { return animationData_[_index].get(); }
};

} // namespace OriGine
