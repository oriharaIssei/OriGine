#include "ModelNodeAnimation.h"

/// engine
#define RESOURCE_DIRECTORY
#include "editor/EditorController.h"
#include "EngineInclude.h"
// module
#include "AnimationManager.h"

// asSets
#include "model/Model.h"

#include "myFileSystem/MyFileSystem.h"

/// externals
#ifdef _DEBUG
#include "myGui/MyGui.h"
#include <imgui/imgui.h>
#endif // _DEBUG

/// math
#include <cmath>

using namespace OriGine;

void ModelNodeAnimation:: Initialize(Scene* /*_scene,*/, EntityHandle /*_owner*/) {
    // 初期化
    currentAnimationTime_  = 0.0f;
    animationState_.isEnd_ = false;

    if (!data_ || data_->animationNodes_.empty()) {
        return;
    }

    if (!fileName_.empty()) {
        data_ = AnimationManager::GetInstance()->Load(directory_, fileName_);
    }
}

void ModelNodeAnimation::Edit(Scene* /*_scene*/, EntityHandle /*_owner*/, [[maybe_unused]] [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    std::string label = "Load File##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        std::string directory, filename;
        if (MyFileSystem::SelectFileDialog(
                kApplicationResourceDirectory,
                directory,
                filename,
                {"gltf", "anm"})) {
            // コマンドを作成
            auto commandCombo = std::make_unique<CommandCombo>();

            commandCombo->AddCommand(std::make_shared<SetterCommand<std::string>>(&directory_, kApplicationResourceDirectory + "/" + directory));
            commandCombo->AddCommand(std::make_shared<SetterCommand<std::string>>(&fileName_, filename));
            commandCombo->SetFuncOnAfterCommand([this]() {
                data_ = AnimationManager::GetInstance()->Load(directory_, fileName_);

                duration_ = data_->duration;
            },
                true);

            OriGine::EditorController::GetInstance()->PushCommand(std::move(commandCombo));
        }
    }

    ImGui::Text("File Name : %s", fileName_.c_str());

    label = "isPlay##" + _parentLabel;
    CheckBoxCommand(label, animationState_.isPlay_);
    label = "Duration##" + _parentLabel;
    DragGuiCommand(label, duration_, 0.01f, 0.0f);

#endif // _DEBUG
}

void ModelNodeAnimation::Finalize() {
}

void ModelNodeAnimation::UpdateModel(float deltaTime, Model* model, const Matrix4x4& parentTransform) {
    {
        // isLoop_ が false の場合,一度終了したら return
        if (!animationState_.isLoop_) {
            if (animationState_.isEnd_) {
                return;
            }
        }

        animationState_.isEnd_ = false;
        // 時間更新
        currentAnimationTime_ += deltaTime;

        // リピート
        if (currentAnimationTime_ > duration_) {
            animationState_.isEnd_  = true;
            animationState_.isPlay_ = false;
            currentAnimationTime_   = std::fmod(currentAnimationTime_, duration_);
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
        return MakeMatrix4x4::Identity();
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

    return MakeMatrix4x4::Affine(scale, rotate, translate);
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

Vec3f ModelNodeAnimation::GetCurrentScale(const std::string& nodeName) const {
    auto itr = data_->animationNodes_.find(nodeName);
    if (itr == data_->animationNodes_.end()) {
        return Vec3f(1.0f, 1.0f, 1.0f);
    }

    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.scale, currentAnimationTime_);
    }
    return CalculateValue::Linear(itr->second.scale, currentAnimationTime_);
}

Quaternion ModelNodeAnimation::GetCurrentRotate(const std::string& nodeName) const {
    auto itr = data_->animationNodes_.find(nodeName);
    if (itr == data_->animationNodes_.end()) {
        return Quaternion::Identity();
    }

    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.rotate, currentAnimationTime_);
    }
    return CalculateValue::Linear(itr->second.rotate, currentAnimationTime_);
}

Vec3f ModelNodeAnimation::GetCurrentTranslate(const std::string& nodeName) const {
    auto itr = data_->animationNodes_.find(nodeName);
    if (itr == data_->animationNodes_.end()) {
        return Vec3f(0.0f, 0.0f, 0.0f);
    }
    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.translate, currentAnimationTime_);
    }
    return CalculateValue::Linear(itr->second.translate, currentAnimationTime_);
}

void OriGine::to_json(nlohmann::json& j, const ModelNodeAnimation& t) {
    j = nlohmann::json{
        {"directory", t.directory_},
        {"fileName", t.fileName_},
        {"isPlay", t.animationState_.isPlay_},
        {"isLoop", t.animationState_.isLoop_},
        {"duration", t.duration_},
        {"currentAnimationTime", t.currentAnimationTime_}};
}

void OriGine::from_json(const nlohmann::json& j, ModelNodeAnimation& t) {
    j.at("directory").get_to(t.directory_);
    j.at("fileName").get_to(t.fileName_);
    j.at("isPlay").get_to(t.animationState_.isPlay_);
    j.at("isLoop").get_to(t.animationState_.isLoop_);
    j.at("duration").get_to(t.duration_);
    j.at("currentAnimationTime").get_to(t.currentAnimationTime_);
}
