#pragma once

#include "Matrix4x4.h"
#include "Quaternion.h"
#include "Thread/Thread.h"
#include "Vector3.h"
#include "model/Model.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#pragma region Animation
template <typename T>
struct Keyframe {
    Keyframe() = default;
    Keyframe(float time, const T& value)
        : time(time), value(value) {}
    ~Keyframe() = default;

    float time; // キーフレームの時刻
    T value;    // キーフレームの 値
};
using KeyframeVector3    = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;

template <typename T>
using AnimationCurve = std::vector<Keyframe<T>>;

struct NodeAnimation {
    AnimationCurve<Vector3> scale;
    AnimationCurve<Quaternion> rotate;
    AnimationCurve<Vector3> translate;
};

/// <summary>
/// アニメーションデータ
/// </summary>
struct AnimationData {
    AnimationData() = default;
    AnimationData(float _duration)
        : duration(duration) {}
    ~AnimationData() = default;

    float duration;
    std::unordered_map<std::string, NodeAnimation> nodeAnimations;
};

struct Animation {
    Animation()  = default;
    ~Animation() = default;

    AnimationData* data;
    //* アニメーションの再生時間(data にも あるが instance 毎に変更できるようにこちらで管理する)
    float duration;             // (秒)
    float currentAnimationTime; // アニメーション の 経過時間 (秒)

    void UpdateTime(float deltaTime) {
        // 時間更新
        currentAnimationTime += deltaTime;
        // リピート
        currentAnimationTime = std::fmod(currentAnimationTime, duration);
    }
    Matrix4x4 CalculateNodeLocal(const std::string& nodeName) const;
    /// <summary>
    ///  指定時間の 値を 計算し 取得
    /// </summary>
    /// <param name="keyframes"></param>
    /// <param name="time"></param>
    /// <returns></returns>
    Vector3 CalculateValue(
        const std::vector<KeyframeVector3>& keyframes, float time) const;
    Quaternion CalculateValue(
        const std::vector<KeyframeQuaternion>& keyframes, float time) const;
};

void ApplyAnimationToNodes(
    const ModelNode& node,
    const Matrix4x4& parentTransform,
    const Animation& animation,
    std::map<std::string, Matrix4x4>& outTransforms);
#pragma endregion

#pragma region AnimationManager
class AnimationManager {
public:
    static AnimationManager& GetInstance() {
        static AnimationManager instance;
        return instance;
    }

    void Init();
    void Finalize();

    /// <summary>
    /// アニメーションデータの読み込み
    ///< param name="directory">ディレクトリ</param>
    ///< param name="filename">ファイル名(format を つける .gltf)</param>
    Animation Load(const std::string& directory, const std::string& filename);
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

    struct AnimationLoad {
        std::string directory;
        std::string filename;
        AnimationData* animationData;
        Animation* animation;

        AnimationLoad() = default;
        AnimationLoad(const std::string& directory, const std::string& filename, AnimationData* animationData)
            : directory(directory), filename(filename), animationData(animationData) {}
        ~AnimationLoad() = default;

        void Update();
    };

    TaskThread<AnimationLoad> loadThread_;

    // アニメーションデータのライブラリ
    std::unordered_map<std::string, int> animationDataLibrary_;
    std::vector<std::unique_ptr<AnimationData>> animationData_;

    // 再生中のアニメーション
    std::vector<std::unique_ptr<Animation>> playingAnimations_;

public:
};
#pragma endregion
