#include "AnimationManager.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <fstream>

AnimationManager::AnimationManager() {
    // Constructor implementation
}

AnimationManager::~AnimationManager() {
    // Destructor implementation
}

void AnimationManager::Init() {
    loadThread_ = std::make_unique<TaskThread<AnimationLoadTask>>();
    loadThread_->Init(1);
}

void AnimationManager::Finalize() {
    loadThread_->Finalize();
}

std::unique_ptr<Animation> AnimationManager::Load(const std::string& directory, const std::string& filename) {
    std::string filePath              = directory + "/" + filename;
    std::unique_ptr<Animation> result = std::make_unique<Animation>();

    auto animationIndex = animationDataLibrary_.find(filePath);
    if (animationIndex != animationDataLibrary_.end()) {
        result->setData(animationData_[animationIndex->second].get());
        result->setDuration(result->getData()->duration);
    } else {
        // 新しい ポインタを作成
        animationData_.push_back(std::make_unique<AnimationData>());
        animationDataLibrary_[filePath] = static_cast<int>(animationData_.size() - 1);

        ///===========================================
        /// TaskThread に ロードタスクを追加
        ///===========================================
        AnimationLoadTask task(directory, filename, animationData_.back().get(), result.get());
        loadThread_->pushTask(task);
    }
    return result;
}

AnimationData AnimationManager::LoadAnimationData(const std::string& directory, const std::string& filename) {
    if (filename.find(".gltf") != std::string::npos) {
        return LoadGltfAnimationData(directory, filename);
    } else if (filename.find(".anm") != std::string::npos) {
        return LoadMyAnimationData(directory, filename);
    }
    return AnimationData();
}

AnimationData AnimationManager::LoadGltfAnimationData(const std::string& directory, const std::string& filename) {
    AnimationData result;
    Assimp::Importer importer;

    std::string filePath = directory + "/" + filename;
    const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);

    // アニメーションがなかったら assert
    assert(scene->mNumAnimations != 0);

    aiAnimation* animationAssimp = scene->mAnimations[0];
    /// 時間の単位を 秒 に 合わせる
    // mTicksPerSecond ： 周波数
    // mDuration      : mTicksPerSecond で 指定された 周波数 における長さ
    result.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);

    for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
        aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
        NodeAnimation& nodeAnimation    = result.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];

        ///=============================================
        /// Scale 解析
        ///=============================================
        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
            aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
            KeyframeVector3 keyframe;
            // 時間単位を 秒 に変換
            keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
            // スケール値をそのまま使用
            keyframe.value = {keyAssimp.mValue[X], keyAssimp.mValue[Y], keyAssimp.mValue[Z]};
            nodeAnimation.scale.push_back(keyframe);
        }

        ///=============================================
        /// Rotate 解析
        ///=============================================
        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
            aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
            KeyframeQuaternion keyframe;
            // 時間単位を 秒 に変換
            keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
            // クォータニオンの値を変換 (右手座標系 → 左手座標系)
            keyframe.value = {
                keyAssimp.mValue .x,
                -keyAssimp.mValue.y,
                -keyAssimp.mValue.z,
                keyAssimp.mValue .w};
            nodeAnimation.rotate.push_back(keyframe);
        }

        ///=============================================
        /// Translate 解析
        ///=============================================
        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
            aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
            KeyframeVector3 keyframe;
            // 時間単位を 秒 に変換
            keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
            // 元が 右手座標系 なので 左手座標系 に 変換する
            keyframe.value = {-keyAssimp.mValue[X], keyAssimp.mValue[Y], keyAssimp.mValue[Z]};
            nodeAnimation.translate.push_back(keyframe);
        }
    }

    return result;
}

AnimationData AnimationManager::LoadMyAnimationData(const std::string& directory, const std::string& filename) {
    ///===========================================
    /// ファイル読み込み
    ///===========================================
    std::string filePath = directory + "/" + filename;
    std::ifstream ifs(filePath, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("Failed to open file for reading");
    }

    AnimationData animationData;

    // duration を読み込み
    ifs.read(reinterpret_cast<char*>(&animationData.duration), sizeof(animationData.duration));

    // nodeAnimations のサイズを読み込み
    size_t nodeAnimationsSize;
    ifs.read(reinterpret_cast<char*>(&nodeAnimationsSize), sizeof(nodeAnimationsSize));

    // 各ノードのアニメーションデータを読み込み
    for (size_t i = 0; i < nodeAnimationsSize; ++i) {
        // ノード名の長さとノード名を読み込み
        size_t nodeNameLength;
        ifs.read(reinterpret_cast<char*>(&nodeNameLength), sizeof(nodeNameLength));
        std::string nodeName(nodeNameLength, '\0');
        ifs.read(&nodeName[0], nodeNameLength);

        NodeAnimation nodeAnimation;

        // scale, rotate, translate の各アニメーションカーブを読み込み
        auto readCurve = [&ifs](auto& curve) {
            size_t size;
            ifs.read(reinterpret_cast<char*>(&size), sizeof(size));
            curve.resize(size);
            for (auto& keyframe : curve) {
                ifs.read(reinterpret_cast<char*>(&keyframe.time), sizeof(keyframe.time));
                ifs.read(reinterpret_cast<char*>(&keyframe.value), sizeof(keyframe.value));
            }
        };

        readCurve(nodeAnimation.scale);
        readCurve(nodeAnimation.rotate);
        readCurve(nodeAnimation.translate);

        animationData.nodeAnimations[nodeName] = nodeAnimation;
    }

    return animationData;
}

void AnimationManager::SaveAnimation(const std::string& directory, const std::string& filename, const AnimationData& animationData) {
    std::string filePath = directory + "/" + filename + ".anm";
    std::ofstream ofs(filePath, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Failed to open file for writing");
    }

    // duration を保存
    ofs.write(reinterpret_cast<const char*>(&animationData.duration), sizeof(animationData.duration));

    // nodeAnimations のサイズを保存
    size_t nodeAnimationsSize = animationData.nodeAnimations.size();
    ofs.write(reinterpret_cast<const char*>(&nodeAnimationsSize), sizeof(nodeAnimationsSize));

    // 各ノードのアニメーションデータを保存
    for (const auto& [nodeName, nodeAnimation] : animationData.nodeAnimations) {
        // ノード名の長さとノード名を保存
        size_t nodeNameLength = nodeName.size();
        ofs.write(reinterpret_cast<const char*>(&nodeNameLength), sizeof(nodeNameLength));
        ofs.write(nodeName.c_str(), nodeNameLength);

        // scale, rotate, translate の各アニメーションカーブを保存
        auto writeCurve = [&ofs](const auto& curve) {
            size_t size = curve.size();
            ofs.write(reinterpret_cast<const char*>(&size), sizeof(size));
            for (const auto& keyframe : curve) {
                ofs.write(reinterpret_cast<const char*>(&keyframe.time), sizeof(keyframe.time));
                ofs.write(reinterpret_cast<const char*>(&keyframe.value), sizeof(keyframe.value));
            }
        };

        writeCurve(nodeAnimation.scale);
        writeCurve(nodeAnimation.rotate);
        writeCurve(nodeAnimation.translate);
    }
}

int AnimationManager::addAnimationData(const std::string& name, std::unique_ptr<AnimationData> animationData) {
    auto animationIndex = animationDataLibrary_.find(name);
    if (animationIndex != animationDataLibrary_.end()) {
        return animationIndex->second;
    }
    animationDataLibrary_[name] = static_cast<int>(animationData_.size());
    animationData_.push_back(std::move(animationData));
    return animationDataLibrary_[name];
}

const AnimationData* AnimationManager::getAnimationData(const std::string& name) const {
    auto animationIndex = animationDataLibrary_.find(name);
    if (animationIndex != animationDataLibrary_.end()) {
        return animationData_[animationIndex->second].get();
    }
    return nullptr;
}

void AnimationManager::AnimationLoadTask::Update() {
    *animationData = AnimationManager::getInstance()->LoadAnimationData(directory, filename);
    animation->setData(animationData);
    animation->setDuration(animationData->duration);
}
