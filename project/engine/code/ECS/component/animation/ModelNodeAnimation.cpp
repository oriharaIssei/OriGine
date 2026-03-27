#include "ModelNodeAnimation.h"

/// engine
#define RESOURCE_DIRECTORY
#include "editor/EditorController.h"
#include "EngineInclude.h"
// module
#include "AnimationManager.h"

// assets
#include "model/Model.h"

#include "myFileSystem/MyFileSystem.h"

/// externals
#ifdef _DEBUG
#include "myGui/MyGui.h"
#include "util/timeline/Timeline.h"
#include <imgui/imgui.h>

// Scene & Renderer (for Edit node tree)
#include "scene/Scene.h"
#include "component/renderer/ModelMeshRenderer.h"
#endif // _DEBUG

/// math
#include <cmath>

using namespace OriGine;

void ModelNodeAnimation::Initialize(Scene* /*_scene*/, EntityHandle /*_entity*/) {
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

void ModelNodeAnimation::Edit(Scene* _scene, EntityHandle _entity, [[maybe_unused]] [[maybe_unused]] const std::string& _parentLabel) {
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

    ImGui::SameLine();
    if (ImGui::Button(("Save File##" + _parentLabel).c_str())) {
        if (data_) {
            data_->duration = duration_;
            AnimationManager::GetInstance()->SaveAnimation(directory_, fileName_, *data_);
        }
    }

    ImGui::Text("File Name : %s", fileName_.c_str());

    label = "Duration##" + _parentLabel;
    DragGuiCommand(label, duration_, 0.01f, 0.0f);

    CheckBoxCommand("Is Loop##" + _parentLabel, animationState_.isLoop_);
    CheckBoxCommand("Is Play##" + _parentLabel, animationState_.isPlay_);

    // --- ModelMeshRenderer からノードツリーを取得・可視化 ---
    {
        ModelMeshData* modelData = nullptr;
        auto& renderers          = _scene->GetComponents<ModelMeshRenderer>(_entity);
        if (!renderers.empty()) {
            modelData = renderers[0].GetModelData();
        }

        if (modelData) {
            if (ImGui::TreeNode(("Node Tree##" + _parentLabel).c_str())) {
                // data_ がなければ作成
                if (!data_) {
                    data_ = std::make_shared<AnimationData>();
                }

                // 再帰的にノードツリーを表示
                std::function<void(const ModelNode&)> showNodeTree;
                showNodeTree = [&](const ModelNode& node) {
                    bool hasAnim = data_->animationNodes_.count(node.name) > 0;

                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
                    if (node.children.empty()) {
                        flags |= ImGuiTreeNodeFlags_Leaf;
                    }

                    // アニメーションノードがあるノードは緑色で表示
                    if (hasAnim) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
                    }

                    bool opened = ImGui::TreeNodeEx(
                        (node.name + "##NodeTree" + _parentLabel).c_str(), flags);

                    if (hasAnim) {
                        ImGui::PopStyleColor();
                    }

                    // アニメーションノードがなければ追加ボタン
                    if (!hasAnim) {
                        ImGui::SameLine();
                        if (ImGui::SmallButton(("+##Add" + node.name + _parentLabel).c_str())) {
                            data_->animationNodes_[node.name] = ModelAnimationNode();
                        }
                    } else {
                        // 削除ボタン
                        ImGui::SameLine();
                        if (ImGui::SmallButton(("-##Remove" + node.name + _parentLabel).c_str())) {
                            data_->animationNodes_.erase(node.name);
                        }
                    }

                    if (opened) {
                        for (const auto& child : node.children) {
                            showNodeTree(child);
                        }
                        ImGui::TreePop();
                    }
                };

                showNodeTree(modelData->rootNode);
                ImGui::TreePop();
            }
        }

        // 手動ノード追加
        {
            static char newNodeName[128] = "";
            ImGui::InputText(("New Node##Input" + _parentLabel).c_str(), newNodeName, sizeof(newNodeName));
            ImGui::SameLine();
            if (ImGui::Button(("Add##NewNode" + _parentLabel).c_str())) {
                if (strlen(newNodeName) > 0) {
                    if (!data_) {
                        data_ = std::make_shared<AnimationData>();
                    }
                    std::string name(newNodeName);
                    if (data_->animationNodes_.find(name) == data_->animationNodes_.end()) {
                        data_->animationNodes_[name] = ModelAnimationNode();
                        newNodeName[0]               = '\0';
                    }
                }
            }
        }
    }

    ImGui::Separator();

    // ノードごとのキーフレーム編集
    if (data_ && !data_->animationNodes_.empty()) {
        for (auto& [nodeName, nodeAnim] : data_->animationNodes_) {
            if (!ImGui::TreeNode((nodeName + "##" + _parentLabel).c_str())) {
                continue;
            }

            // InterpolationType
            std::string comboLabel = "InterpolationType##" + nodeName + _parentLabel;
            if (ImGui::BeginCombo(comboLabel.c_str(), InterpolationTypeName[int(nodeAnim.interpolationType)])) {
                for (int i = 0; i < (int)InterpolationType::COUNT; ++i) {
                    if (ImGui::Selectable(InterpolationTypeName[i], nodeAnim.interpolationType == InterpolationType(i))) {
                        OriGine::EditorController::GetInstance()->PushCommand(
                            std::make_unique<SetterCommand<InterpolationType>>(&nodeAnim.interpolationType, InterpolationType(i)));
                    }
                }
                ImGui::EndCombo();
            }

            ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollX;
            if (ImGui::BeginTable(("NodeKeyFrames##" + nodeName + _parentLabel).c_str(), 2, tableFlags)) {
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Edit");
                ImGui::TableHeadersRow();

                // Scale
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Scale");
                ImGui::TableSetColumnIndex(1);
                ImGui::EditKeyFrame("##Scale" + nodeName + _parentLabel, nodeAnim.scale, duration_);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Separator();
                ImGui::TableSetColumnIndex(1);
                ImGui::Separator();

                // Rotate
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Rotate");
                ImGui::TableSetColumnIndex(1);
                ImGui::EditKeyFrame("##Rotate" + nodeName + _parentLabel, nodeAnim.rotate, duration_);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Separator();
                ImGui::TableSetColumnIndex(1);
                ImGui::Separator();

                // Translate
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Translate");
                ImGui::TableSetColumnIndex(1);
                ImGui::EditKeyFrame("##Translate" + nodeName + _parentLabel, nodeAnim.translate, duration_);

                ImGui::EndTable();
            }

            ImGui::TreePop();
        }
    }

#endif // _DEBUG
}

void ModelNodeAnimation::Finalize() {
}

void ModelNodeAnimation::UpdateModel(float _deltaTime, Model* _model, const Matrix4x4& _parentTransform) {
    {
        // isLoop_ が false の場合,一度終了したら return
        if (!animationState_.isLoop_) {
            if (animationState_.isEnd_) {
                return;
            }
        }

        animationState_.isEnd_ = false;
        // 時間更新
        currentAnimationTime_ += _deltaTime;

        // リピート
        if (currentAnimationTime_ > duration_) {
            animationState_.isEnd_  = true;
            animationState_.isPlay_ = false;
            currentAnimationTime_   = std::fmod(currentAnimationTime_, duration_);
        }
    }

    {
        ApplyAnimationToNodes(_model->meshData_->rootNode, _parentTransform, this);
    }
}

Matrix4x4 ModelNodeAnimation::CalculateNodeLocal(const std::string& _nodeName) const {
    auto it = data_->animationNodes_.find(_nodeName);
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
    ModelNode& _node,
    const Matrix4x4& _parentTransform,
    const ModelNodeAnimation* _animation) {
    _node.localMatrix         = _animation->CalculateNodeLocal(_node.name);
    Matrix4x4 globalTransform = _parentTransform * _node.localMatrix;

    // 子ノードに再帰的に適用
    for (auto& child : _node.children) {
        ApplyAnimationToNodes(child, globalTransform, _animation);
    }
}

Vec3f ModelNodeAnimation::GetCurrentScale(const std::string& _nodeName) const {
    auto itr = data_->animationNodes_.find(_nodeName);
    if (itr == data_->animationNodes_.end()) {
        return Vec3f(1.0f, 1.0f, 1.0f);
    }

    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.scale, currentAnimationTime_);
    }
    return CalculateValue::Linear(itr->second.scale, currentAnimationTime_);
}

Quaternion ModelNodeAnimation::GetCurrentRotate(const std::string& _nodeName) const {
    auto itr = data_->animationNodes_.find(_nodeName);
    if (itr == data_->animationNodes_.end()) {
        return Quaternion::Identity();
    }

    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.rotate, currentAnimationTime_);
    }
    return CalculateValue::Linear(itr->second.rotate, currentAnimationTime_);
}

Vec3f ModelNodeAnimation::GetCurrentTranslate(const std::string& _nodeName) const {
    auto itr = data_->animationNodes_.find(_nodeName);
    if (itr == data_->animationNodes_.end()) {
        return Vec3f(0.0f, 0.0f, 0.0f);
    }
    if (itr->second.interpolationType == InterpolationType::STEP) {
        return CalculateValue::Step(itr->second.translate, currentAnimationTime_);
    }
    return CalculateValue::Linear(itr->second.translate, currentAnimationTime_);
}

void OriGine::to_json(nlohmann::json& _j, const ModelNodeAnimation& _comp) {
    _j = nlohmann::json{
        {"directory", _comp.directory_},
        {"fileName", _comp.fileName_},
        {"isPlay", _comp.animationState_.isPlay_},
        {"isLoop", _comp.animationState_.isLoop_},
        {"duration", _comp.duration_},
        {"currentAnimationTime", _comp.currentAnimationTime_}};
}

void OriGine::from_json(const nlohmann::json& _j, ModelNodeAnimation& _comp) {
    _j.at("directory").get_to(_comp.directory_);
    _j.at("fileName").get_to(_comp.fileName_);
    _j.at("isPlay").get_to(_comp.animationState_.isPlay_);
    _j.at("isLoop").get_to(_comp.animationState_.isLoop_);
    _j.at("duration").get_to(_comp.duration_);
    _j.at("currentAnimationTime").get_to(_comp.currentAnimationTime_);
}
