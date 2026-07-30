#include "AnimationLoader.h"

/// stl
#include <fstream>

/// util
#include "logger/Logger.h"

/// externals
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

using namespace OriGine;

/// <summary>
/// assimp でモデルファイルを開き、先頭のアニメーションを AnimationData に変換する.
/// assimp は右手座標系・ティック単位で値を返すため、
/// エンジン側の左手座標系・秒単位へ変換しながら取り込む.
/// </summary>
/// <param name="_assetPath">読み込むファイルのパス</param>
/// <returns>読み込まれたアニメーションアセット（失敗時は空のアニメーション）</returns>
AnimationAsset GltfAnimationLoader::LoadAsset(const std::string& _assetPath) {
    AnimationAsset asset;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(_assetPath.c_str(), 0);
    if (scene == nullptr) {
        LOG_ERROR("Failed to load animation file: {} ({})", _assetPath, importer.GetErrorString());
        return asset;
    }
    if (scene->mNumAnimations == 0) {
        LOG_ERROR("Animation not found in file: {}", _assetPath);
        return asset;
    }

    AnimationData& result = asset.data;

    aiAnimation* animationAssimp = scene->mAnimations[0];
    /// 時間の単位を 秒 に 合わせる
    // mTicksPerSecond ： 周波数
    // mDuration      : mTicksPerSecond で 指定された 周波数 における長さ
    const double ticksPerSecond = animationAssimp->mTicksPerSecond != 0.0
                                      ? animationAssimp->mTicksPerSecond
                                      : 1.0;
    result.duration             = float(animationAssimp->mDuration / ticksPerSecond);

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
            keyframe.time = float(keyAssimp.mTime / ticksPerSecond);
            // スケール値をそのまま使用
            keyframe.value = {keyAssimp.mValue[X], keyAssimp.mValue[Y], keyAssimp.mValue[Z]};
            nodeAnimation.scale.push_back(keyframe);
        }

        // =============================== Rotate =============================== //
        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
            aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
            KeyframeQuaternion keyframe;
            // 時間単位を 秒 に変換
            keyframe.time = float(keyAssimp.mTime / ticksPerSecond);
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
            keyframe.time = float(keyAssimp.mTime / ticksPerSecond);
            // 元が 右手座標系 なので 左手座標系 に 変換する
            keyframe.value = {-keyAssimp.mValue[X], keyAssimp.mValue[Y], keyAssimp.mValue[Z]};
            nodeAnimation.translate.push_back(keyframe);
        }
    }

    return asset;
}

/// <summary>
/// 独自バイナリ形式 (.anm) を読み込む.
/// レイアウトは AnimationSerializer::Save の書き出し順と対になっている.
/// </summary>
/// <param name="_assetPath">読み込むファイルのパス</param>
/// <returns>読み込まれたアニメーションアセット（失敗時は空のアニメーション）</returns>
AnimationAsset MyAnimationLoader::LoadAsset(const std::string& _assetPath) {
    AnimationAsset asset;

    std::ifstream ifs(_assetPath, std::ios::binary);
    if (!ifs) {
        LOG_ERROR("Failed to open animation file for reading: {}", _assetPath);
        return asset;
    }

    AnimationData& animationData = asset.data;

    // duration を読み込み
    ifs.read(reinterpret_cast<char*>(&animationData.duration), sizeof(animationData.duration));

    // nodeAnimations のサイズを読み込み
    size_t nodeAnimationsSize = 0;
    ifs.read(reinterpret_cast<char*>(&nodeAnimationsSize), sizeof(nodeAnimationsSize));

    // 各ノードのアニメーションデータを読み込み
    for (size_t i = 0; i < nodeAnimationsSize; ++i) {
        // ノード名の長さとノード名を読み込み
        size_t nodeNameLength = 0;
        ifs.read(reinterpret_cast<char*>(&nodeNameLength), sizeof(nodeNameLength));
        std::string nodeName(nodeNameLength, '\0');
        ifs.read(nodeName.data(), nodeNameLength);

        ModelAnimationNode nodeAnimation;

        // scale, rotate, translate の各アニメーションカーブを読み込み
        auto readCurve = [&ifs](auto& curve) {
            size_t size = 0;
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

    return asset;
}
