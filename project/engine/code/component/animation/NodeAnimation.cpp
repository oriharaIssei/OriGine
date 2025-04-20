#include "NodeAnimation.h"

/// engine
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
// module
#include "AnimationManager.h"

// assets
#include "model/Model.h"

/// lib
#include "myFileSystem/MyFileSystem.h"

/// externals
#ifdef _DEBUG
#include <imgui/imgui.h>
#endif // _DEBUG

/// math
#include <cmath>

void NodeAnimation::Initialize(GameEntity* /*_entity*/) {
    // 初期化
    currentAnimationTime_ = 0.0f;
    isEnd_                = false;

    if (data_->animationNodes_.empty()) {
        return;
    }
}

bool NodeAnimation::Edit() {
#ifdef _DEBUG
    bool isChange = false;
    if (ImGui::Button("Load File")) {
        std::string directory, filename;
        if (MyFileSystem::selectFileDialog(
                kApplicationResourceDirectory,
                directory,
                filename,
                {"gltf", "anm"})) {

            data_ = AnimationManager::getInstance()->Load(kApplicationResourceDirectory + "/" + directory, filename);

            directory_ = kApplicationResourceDirectory + "/" + directory;
            fileName_  = filename;

            duration_ = data_->duration;

            isChange = true;
        }
    }

    ImGui::Text("File Name : %s", fileName_.c_str());

    isChange |= ImGui::Checkbox("isPlay", &isPlay_);

    isChange |= ImGui::DragFloat("Duration", &duration_, 0.01f, 0.0f);

    return isChange;
#else
    return false;
#endif // _DEBUG
}

void NodeAnimation::Save(BinaryWriter& _writer) {
    _writer.Write("isPlay", isPlay_);
    _writer.Write("isLoop", isLoop_);
    _writer.Write("duration", duration_);

    _writer.Write("directory", directory_);
    _writer.Write("fileName", fileName_);
}

void NodeAnimation::Load(BinaryReader& _reader) {
    _reader.Read("isPlay", isPlay_);
    _reader.Read("isLoop", isLoop_);
    _reader.Read("duration", duration_);

    _reader.Read("directory", directory_);
    _reader.Read("fileName", fileName_);

    if (!fileName_.empty()) {
        data_ = AnimationManager::getInstance()->Load(directory_, fileName_);
        while (true) {
            if (data_->loadState == LoadState::Loaded) {
                break;
            }
        }
    }
}

void NodeAnimation::Finalize() {
}

void NodeAnimation::UpdateModel(float deltaTime, Model* model, const Matrix4x4& parentTransform) {
    {
        // isLoop_ が false の場合,一度終了したら return
        if (!isLoop_) {
            if (isEnd_) {
                return;
            }
        }

        isEnd_ = false;
        // 時間更新
        currentAnimationTime_ += deltaTime;

        // リピート
        if (currentAnimationTime_ > duration_) {
            isEnd_                = true;
            isPlay_               = false;
            currentAnimationTime_ = std::fmod(currentAnimationTime_, duration_);
        }
    }

    {
        ApplyAnimationToNodes(model->meshData_->rootNode, parentTransform, this);
    }
}

Matrix4x4 NodeAnimation::CalculateNodeLocal(const std::string& nodeName) const {
    auto it = data_->animationNodes_.find(nodeName);
    if (it == data_->animationNodes_.end()) {
        // ノードに対応するアニメーションがない場合、単位行列を返す
        return MakeMatrix::Identity();
    }

    const AnimationNode& nodeAnimation = it->second;

    Vec3f scale;
    Quaternion rotate;
    Vec3f translate;

    switch (nodeAnimation.interpolationType) {
    case InterpolationType::LINEAR:
        scale     = CalculateValue::LINEAR(nodeAnimation.scale, currentAnimationTime_);
        rotate    = Quaternion::Normalize(CalculateValue::LINEAR(nodeAnimation.rotate, currentAnimationTime_));
        translate = CalculateValue::LINEAR(nodeAnimation.translate, currentAnimationTime_);
        break;
    case InterpolationType::STEP:
        scale     = CalculateValue::Step(nodeAnimation.scale, currentAnimationTime_);
        rotate    = Quaternion::Normalize(CalculateValue::Step(nodeAnimation.rotate, currentAnimationTime_));
        translate = CalculateValue::Step(nodeAnimation.translate, currentAnimationTime_);
        break;
    }

    return MakeMatrix::Affine(scale, rotate, translate);
}

float CalculateValue::LINEAR(const std::vector<Keyframe<float>>& keyframes, float time) {
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
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return std::lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}

Vec2f CalculateValue::LINEAR(const std::vector<Keyframe<Vec2f>>& keyframes, float time) {
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
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}

Vec3f CalculateValue::LINEAR(const std::vector<KeyframeVector3>& keyframes, float time) {
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
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }

    // 登録されている時間より 後ろ
    // 最後の 値を
    return (*keyframes.rbegin()).value;
}
Vec4f CalculateValue::LINEAR(const std::vector<Keyframe<Vec4f>>& keyframes, float time) {
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
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}
Quaternion CalculateValue::LINEAR(const std::vector<KeyframeQuaternion>& keyframes, float time) {
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
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Slerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }

    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}

float CalculateValue::Step(const std::vector<Keyframe<float>>& keyframes, float time) {
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
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            // 範囲内 で 保管
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return std::lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}
Vec2f CalculateValue::Step(const std::vector<Keyframe<Vec2f>>& keyframes, float time) {
    assert(!keyframes.empty());
    if (keyframes.size() == 1 || time <= keyframes[0].time) {
        return keyframes[0].value;
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            return keyframes[index].value;
        }
    }
    return (*keyframes.rbegin()).value;
}
Vec3f CalculateValue::Step(const std::vector<KeyframeVector3>& keyframes, float time) {
    assert(!keyframes.empty());
    if (keyframes.size() == 1 || time <= keyframes[0].time) {
        return keyframes[0].value;
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            return keyframes[index].value;
        }
    }
    return (*keyframes.rbegin()).value;
}
Vec4f CalculateValue::Step(const std::vector<Keyframe<Vec4f>>& keyframes, float time) {
    assert(!keyframes.empty());
    if (keyframes.size() == 1 || time <= keyframes[0].time) {
        return keyframes[0].value;
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            return keyframes[index].value;
        }
    }
    return (*keyframes.rbegin()).value;
}
Quaternion CalculateValue::Step(const std::vector<KeyframeQuaternion>& keyframes, float time) {
    assert(!keyframes.empty());
    if (keyframes.size() == 1 || time <= keyframes[0].time) {
        return keyframes[0].value;
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            return keyframes[index].value;
        }
    }
    return (*keyframes.rbegin()).value;
}

void NodeAnimation::ApplyAnimationToNodes(
    ModelNode& node,
    const Matrix4x4& parentTransform,
    const NodeAnimation* animation) {
    node.localMatrix          = animation->CalculateNodeLocal(node.name);
    Matrix4x4 globalTransform = parentTransform * node.localMatrix;

    // 子ノードに再帰的に適用
    for (auto& child : node.children) {
        ApplyAnimationToNodes(child, globalTransform, animation);
    }
}

Vec3f NodeAnimation::getCurrentScale(const std::string& nodeName) const {
    auto itr = data_->animationNodes_.find(nodeName);
    if (itr == data_->animationNodes_.end()) {
        return Vec3f(1.0f, 1.0f, 1.0f);
    }

    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.scale, currentAnimationTime_);
    }
    return CalculateValue::LINEAR(itr->second.scale, currentAnimationTime_);
}

Quaternion NodeAnimation::getCurrentRotate(const std::string& nodeName) const {
    auto itr = data_->animationNodes_.find(nodeName);
    if (itr == data_->animationNodes_.end()) {
        return Quaternion::Identity();
    }

    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.rotate, currentAnimationTime_);
    }
    return CalculateValue::LINEAR(itr->second.rotate, currentAnimationTime_);
}

Vec3f NodeAnimation::getCurrentTranslate(const std::string& nodeName) const {
    auto itr = data_->animationNodes_.find(nodeName);
    if (itr == data_->animationNodes_.end()) {
        return Vec3f(0.0f, 0.0f, 0.0f);
    }
    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.translate, currentAnimationTime_);
    }
    return CalculateValue::LINEAR(itr->second.translate, currentAnimationTime_);
}
