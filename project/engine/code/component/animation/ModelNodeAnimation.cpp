#include "ModelNodeAnimation.h"

/// engine
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
#include "module/editor/EditorGroup.h"
// module
#include "AnimationManager.h"

// assets
#include "model/Model.h"

/// lib
#include "myFileSystem/MyFileSystem.h"

/// externals
#ifdef _DEBUG
#include <imgui/imgui.h>
#include "myGui/MyGui.h"
#endif // _DEBUG

/// math
#include <cmath>

void ModelNodeAnimation::Initialize(GameEntity* /*_entity*/) {
    // 初期化
    currentAnimationTime_ = 0.0f;
    isEnd_                = false;

    if (data_->animationNodes_.empty()) {
        return;
    }

    if (!fileName_.empty()) {
        data_ = AnimationManager::getInstance()->Load(directory_, fileName_);
        while (true) {
            if (data_->loadState == LoadState::Loaded) {
                break;
            }
        }
    }
}

bool ModelNodeAnimation::Edit() {
#ifdef _DEBUG
    bool isChange = false;
    if (ImGui::Button("Load File")) {
        std::string directory, filename;
        if (MyFileSystem::selectFileDialog(
                kApplicationResourceDirectory,
                directory,
                filename,
                {"gltf", "anm"})) {
            // コマンドを作成
            auto commandCombo = std::make_unique<CommandCombo>();

            commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&directory_, kApplicationResourceDirectory + "/" + directory));
            commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&fileName_, filename));
            commandCombo->setFuncOnAfterCommand([this]() {
                data_ = AnimationManager::getInstance()->Load(directory_ ,fileName_);
                while (true) {
                    if (data_->loadState == LoadState::Loaded) {
                        break;
                    }
                }
                duration_ = data_->duration;
            });

            EditorGroup::getInstance()->pushCommand(std::move(commandCombo));

            isChange = true;
        }
    }

    ImGui::Text("File Name : %s", fileName_.c_str());

    isChange |= CheckBoxCommand("isPlay", isPlay_);

    isChange |= DragCommand("Duration", duration_, 0.01f, 0.0f);

    return isChange;
#else
    return false;
#endif // _DEBUG
}

void ModelNodeAnimation::Finalize() {
}

void ModelNodeAnimation::UpdateModel(float deltaTime, Model* model, const Matrix4x4& parentTransform) {
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

Matrix4x4 ModelNodeAnimation::CalculateNodeLocal(const std::string& nodeName) const {
    auto it = data_->animationNodes_.find(nodeName);
    if (it == data_->animationNodes_.end()) {
        // ノードに対応するアニメーションがない場合、単位行列を返す
        return MakeMatrix::Identity();
    }

    const ModelAnimationNode& nodeAnimation = it->second;

    Vec3f scale;
    Quaternion rotate;
    Vec3f translate;

    switch (nodeAnimation.interpolationType) {
    case InterpolationType::LINEAR:
        scale     = CalculateValue::Linear(nodeAnimation.scale, currentAnimationTime_);
        rotate    = Quaternion::Normalize(CalculateValue::Linear(nodeAnimation.rotate, currentAnimationTime_));
        translate = CalculateValue::Linear(nodeAnimation.translate, currentAnimationTime_);
        break;
    case InterpolationType::STEP:
        scale     = CalculateValue::Step(nodeAnimation.scale, currentAnimationTime_);
        rotate    = Quaternion::Normalize(CalculateValue::Step(nodeAnimation.rotate, currentAnimationTime_));
        translate = CalculateValue::Step(nodeAnimation.translate, currentAnimationTime_);
        break;
    }

    return MakeMatrix::Affine(scale, rotate, translate);
}

float CalculateValue::Linear(const std::vector<Keyframe<float>>& keyframes, float time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (keyframes.empty()) {
            return 0.f;
        }
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

Vec2f CalculateValue::Linear(const std::vector<Keyframe<Vec2f>>& keyframes, float time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (keyframes.empty()) {
            return Vec2f();
        }

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
Vec3f CalculateValue::Linear(const std::vector<KeyframeVector3>& keyframes, float time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (keyframes.empty()) {
            return Vec3f();
        }
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
Vec4f CalculateValue::Linear(const std::vector<Keyframe<Vec4f>>& keyframes, float time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (keyframes.empty()) {
            return Vec4f();
        }
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
Quaternion CalculateValue::Linear(const std::vector<KeyframeQuaternion>& keyframes, float time) {
    ///===========================================
    /// 例外処理
    ///===========================================
    {
        if (keyframes.empty()) {
            return Quaternion();
        }
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
        if (keyframes.empty()) {
            return 0.f;
        }
        if (keyframes.size() == 1 || time <= keyframes[0].time) {
            return keyframes[0].value;
        }
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // index と nextIndex の 2つを
        // 取得して 現時刻が 範囲内か
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            return keyframes[index].value;
        }
    }
    // 登録されている時間より 後ろ
    // 最後の 値を返す
    return (*keyframes.rbegin()).value;
}
Vec2f CalculateValue::Step(const std::vector<Keyframe<Vec2f>>& keyframes, float time) {
    if (keyframes.empty()) {
        return Vec2f();
    }

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
    if (keyframes.empty()) {
        return Vec3f();
    }
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
    if (keyframes.empty()) {
        return Vec4f();
    }
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
    if (keyframes.empty()) {
        return Quaternion();
    }
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

void ModelNodeAnimation::ApplyAnimationToNodes(
    ModelNode& node,
    const Matrix4x4& parentTransform,
    const ModelNodeAnimation* animation) {
    node.localMatrix          = animation->CalculateNodeLocal(node.name);
    Matrix4x4 globalTransform = parentTransform * node.localMatrix;

    // 子ノードに再帰的に適用
    for (auto& child : node.children) {
        ApplyAnimationToNodes(child, globalTransform, animation);
    }
}

Vec3f ModelNodeAnimation::getCurrentScale(const std::string& nodeName) const {
    auto itr = data_->animationNodes_.find(nodeName);
    if (itr == data_->animationNodes_.end()) {
        return Vec3f(1.0f, 1.0f, 1.0f);
    }

    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.scale, currentAnimationTime_);
    }
    return CalculateValue::Linear(itr->second.scale, currentAnimationTime_);
}

Quaternion ModelNodeAnimation::getCurrentRotate(const std::string& nodeName) const {
    auto itr = data_->animationNodes_.find(nodeName);
    if (itr == data_->animationNodes_.end()) {
        return Quaternion::Identity();
    }

    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.rotate, currentAnimationTime_);
    }
    return CalculateValue::Linear(itr->second.rotate, currentAnimationTime_);
}

Vec3f ModelNodeAnimation::getCurrentTranslate(const std::string& nodeName) const {
    auto itr = data_->animationNodes_.find(nodeName);
    if (itr == data_->animationNodes_.end()) {
        return Vec3f(0.0f, 0.0f, 0.0f);
    }
    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.translate, currentAnimationTime_);
    }
    return CalculateValue::Linear(itr->second.translate, currentAnimationTime_);
}

void to_json(nlohmann::json& j, const ModelNodeAnimation& t) {
    j = nlohmann::json{
        {"directory", t.directory_},
        {"fileName", t.fileName_},
        {"isPlay", t.isPlay_},
        {"isLoop", t.isLoop_},
        {"duration", t.duration_},
        {"currentAnimationTime", t.currentAnimationTime_}};
}

void from_json(const nlohmann::json& j, ModelNodeAnimation& t) {
    j.at("directory").get_to(t.directory_);
    j.at("fileName").get_to(t.fileName_);
    j.at("isPlay").get_to(t.isPlay_);
    j.at("isLoop").get_to(t.isLoop_);
    j.at("duration").get_to(t.duration_);
    j.at("currentAnimationTime").get_to(t.currentAnimationTime_);
}
