#include "AnimationManager.h"

/// stl
#include <fstream>
/// externals
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

using namespace OriGine;

AnimationManager::AnimationManager() {
    // Constructor implementation
}

AnimationManager::~AnimationManager() {
    // Destructor implementation
}

void AnimationManager::Initialize() {}

void AnimationManager::Finalize() {}

std::shared_ptr<AnimationData> AnimationManager::Load(const std::string& _directory, const std::string& _filename) {
    std::string filePath                  = _directory + "/" + _filename;
    std::shared_ptr<AnimationData> result = nullptr;

    auto animationIndex = animationDataLibrary_.find(filePath);
    if (animationIndex != animationDataLibrary_.end()) {
        result = animationData_[animationIndex->second];
    } else {
        // 新しい ポインタを作成
        animationData_.push_back(std::make_shared<AnimationData>());
        animationDataLibrary_[filePath] = static_cast<int>(animationData_.size() - 1);

        result = animationData_.back();

        ///===========================================
        /// TaskThread に ロードタスクを追加
        ///===========================================
        AnimationLoadTask task(_directory, _filename, animationData_.back());
        task.Update();
    }
    return result;
}

AnimationData AnimationManager::LoadAnimationData(const std::string& _directory, const std::string& _filename) {
    if (_filename.find(".gltf") != std::string::npos) {
        return LoadGltfAnimationData(_directory, _filename);
    } else if (_filename.find(".anm") != std::string::npos) {
        return LoadMyAnimationData(_directory, _filename);
    }
    return AnimationData();
}

AnimationData AnimationManager::LoadGltfAnimationData(const std::string& _directory, const std::string& _filename) {
    AnimationData result;
    Assimp::Importer importer;

    // =============================== アニメーションデータの読み込み =============================== //
    std::string filePath = _directory + "/" + _filename;
    const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);

    // アニメーションがなかったら assert
    assert(scene->mNumAnimations != 0);

    aiAnimation* animationAssimp = scene->mAnimations[0];
    /// 時間の単位を 秒 に 合わせる
    // mTicksPerSecond ： 周波数
    // mDuration      : mTicksPerSecond で 指定された 周波数 における長さ
    result.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);

    ///=============================================
    /// ノードアニメーションの解析
    ///=============================================
    for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
        aiNodeAnim* nodeAnimationAssimp   = animationAssimp->mChannels[channelIndex];
        ModelAnimationNode& nodeAnimation = result.animationNodes_[nodeAnimationAssimp->mNodeName.C_Str()];

        // =============================== InterpolationType =============================== //
        nodeAnimation.interpolationType = static_cast<InterpolationType>(nodeAnimationAssimp->mPreState);
        // =============================== Scale =============================== //
        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
            aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
            KeyframeVector3 keyframe;
            // 時間単位を 秒 に変換
            keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
            // スケール値をそのまま使用
            keyframe.value = {keyAssimp.mValue[X], keyAssimp.mValue[Y], keyAssimp.mValue[Z]};
            nodeAnimation.scale.push_back(keyframe);
        }

        // =============================== Rotate =============================== //
        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
            aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
            KeyframeQuaternion keyframe;
            // 時間単位を 秒 に変換
            keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
            // クォータニオンの値を変換 (右手座標系 → 左手座標系)
            keyframe.value = Quaternion(
                keyAssimp.mValue.x,
                -keyAssimp.mValue.y,
                -keyAssimp.mValue.z,
                keyAssimp.mValue.w);
            nodeAnimation.rotate.push_back(keyframe);
        }

        // =============================== Translate =============================== //
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

AnimationData AnimationManager::LoadMyAnimationData(const std::string& _directory, const std::string& _filename) {
    ///===========================================
    /// ファイル読み込み
    ///===========================================
    std::string filePath = _directory + "/" + _filename;
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

        ModelAnimationNode nodeAnimation;

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

        animationData.animationNodes_[nodeName] = nodeAnimation;
    }

    return animationData;
}

void AnimationManager::SaveAnimation(const std::string& _directory, const std::string& _filename, const AnimationData& _animationData) {
    std::string filePath = _directory + "/" + _filename + ".anm";
    std::ofstream ofs(filePath, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Failed to open file for writing");
    }

    // duration を保存
    ofs.write(reinterpret_cast<const char*>(&_animationData.duration), sizeof(_animationData.duration));

    // nodeAnimations のサイズを保存
    size_t nodeAnimationsSize = _animationData.animationNodes_.size();
    ofs.write(reinterpret_cast<const char*>(&nodeAnimationsSize), sizeof(nodeAnimationsSize));

    // 各ノードのアニメーションデータを保存
    for (const auto& [nodeName, nodeAnimation] : _animationData.animationNodes_) {
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

int AnimationManager::addAnimationData(const std::string& _name, std::unique_ptr<AnimationData> _animationData) {
    auto animationIndex = animationDataLibrary_.find(_name);
    if (animationIndex != animationDataLibrary_.end()) {
        return animationIndex->second;
    }
    animationDataLibrary_[_name] = static_cast<int>(animationData_.size());
    animationData_.push_back(std::move(_animationData));
    return animationDataLibrary_[_name];
}

const AnimationData* AnimationManager::GetAnimationData(const std::string& _name) const {
    auto animationIndex = animationDataLibrary_.find(_name);
    if (animationIndex != animationDataLibrary_.end()) {
        return animationData_[animationIndex->second].get();
    }
    return nullptr;
}

void AnimationManager::AnimationLoadTask::Update() const {
    *animationData = AnimationManager::GetInstance()->LoadAnimationData(directory, filename);
}
