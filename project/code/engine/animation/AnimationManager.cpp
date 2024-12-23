#include "AnimationManager.h"

#include <fstream>

#pragma region Animation
Matrix4x4 Animation::CalculateNodeLocal(const std::string& nodeName) const {
    auto it = data->nodeAnimations.find(nodeName);
    if (it == data->nodeAnimations.end()) {
        // ノードに対応するアニメーションがない場合、単位行列を返す
        return MakeMatrix::Identity();
    }

    const NodeAnimation& nodeAnimation = it->second;
    Vector3 scale                      = CalculateValue(nodeAnimation.scale, currentAnimationTime);
    Quaternion rotate                  = Quaternion::Normalize(CalculateValue(nodeAnimation.rotate, currentAnimationTime));
    Vector3 translate                  = CalculateValue(nodeAnimation.translate, currentAnimationTime);
    return MakeMatrix::Affine(scale, rotate, translate);
}

Vector3 Animation::CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time) const {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        assert(!keyframes.empty());
        if (keyframes.size() == 1 || time <= keyframes[0].time) {
            return keyframes[0].value;
        }
    }

    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;

        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (keyframes[index].time <= time &&
            time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) /
                      (keyframes[nextIndex].time - keyframes[index].time);
            return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }

    // 登録されている時間より 後ろ
    // 最後の 値を
    return (*keyframes.rbegin()).value;
}

Quaternion Animation::CalculateValue(
    const std::vector<KeyframeQuaternion>& keyframes, float time) const {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        assert(!keyframes.empty());
        if (keyframes.size() == 1 || time <= keyframes[0].time) {
            return keyframes[0].value;
        }
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;

        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (keyframes[index].time <= time &&
            time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) /
                      (keyframes[nextIndex].time - keyframes[index].time);
            return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }

    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}

void ApplyAnimationToNodes(const ModelNode& node, const Matrix4x4& parentTransform, const Animation& animation, std::map<std::string, Matrix4x4>& outTransforms) {
    // ノードのローカル行列を計算
    Matrix4x4 localTransform  = animation.CalculateNodeLocal(node.name);
    Matrix4x4 globalTransform = parentTransform * localTransform;

    // 計算結果を保存
    outTransforms[node.name] = globalTransform;

    // 子ノードに再帰的に適用
    for (const auto& child : node.children) {
        ApplyAnimationToNodes(child, globalTransform, animation, outTransforms);
    }
}
#pragma endregion

AnimationManager::AnimationManager() {
    // Constructor implementation
}

AnimationManager::~AnimationManager() {
    // Destructor implementation
}

Animation AnimationManager::Load(const std::string& directory, const std::string& filename) {
    std::string filePath = directory + "/" + filename;
    auto animationIndex  = animationDataLibrary_.find(filePath);
    if (animationIndex != animationDataLibrary_.end()) {
        return Animation(animationIndex->second.get());
    } else {
        animationData_.push_back(std::make_unique<AnimationData>());
        animationDataLibrary_[filePath] = animationData_.size() - 1;
        return
    }
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

void SaveAnimationData(const AnimationData& animationData, const std::string& filename) {
    std::ofstream ofs(filename, std::ios::binary);
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
