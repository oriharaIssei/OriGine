#include "AnimationEditor.h"

// stl
#include <algorithm>

// engine
#include "Engine.h"
#include "model/ModelManager.h"

// object
#include "animation/Animation.h"
#include "animation/AnimationManager.h"
#include "object3d/AnimationObject3d.h"

// lib
#include "myFileSystem/MyFileSystem.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "Timeline.h"
#endif // _DEBUG

static std::array<std::string, static_cast<int>(InterpolationType::COUNT)> interpolationTypeString = {
    "LINEAR",
    "STEP",
    "CUBICSPLINE"};

AnimationEditor::AnimationEditor()
    : IEditor() {}

AnimationEditor::~AnimationEditor() {}

void AnimationEditor::Initialize() {
    // アニメーションリストを取得
    // json
    animationSettingsFileList_ = myfs::SearchFile("resource/GlobalVariables/Animations", "json");

    // gltf
    std::list<std::pair<std::string, std::string>> addFileList = myfs::SearchFile("resource", "gltf", false);
    animationSettingsFileList_.insert(animationSettingsFileList_.end(), addFileList.begin(), addFileList.end());

    // editInstance を作成
    currentEditObject_ = std::make_unique<AnimationObject3d>();
}

void AnimationEditor::Update() {

#ifdef _DEBUG
    bool openCreateNewPopup = false;

    if (ImGui::Begin("AnimationEditor", nullptr, ImGuiWindowFlags_MenuBar)) {
        ///================================================================
        /// メニューバー
        ///================================================================
        if (ImGui::BeginMenuBar()) {
            /// ================================================================
            // ファイル
            if (ImGui::BeginMenu("File")) {
                // filelist reload
                if (ImGui::MenuItem("Reload")) {
                    //===================== アニメーションリストを取得 =====================//
                    // json
                    animationSettingsFileList_ = myfs::SearchFile("resource/GlobalVariables/Animations", "json");

                    // gltf
                    std::list<std::pair<std::string, std::string>> addFileList = myfs::SearchFile("resource", "gltf", false);
                    animationSettingsFileList_.insert(animationSettingsFileList_.end(), addFileList.begin(), addFileList.end());
                }
                // 開く
                if (ImGui::BeginMenu("Open")) {
                    //===================== アニメーション設定ファイルを開く =====================//
                    for (const auto& [directory, filename] : animationSettingsFileList_) {
                        if (ImGui::MenuItem(filename.c_str())) {
                            // ファイルのパスを保存
                            if (filename.find("gltf") != std::string::npos) {
                                currentEditAnimationSetting_.reset(new AnimationSetting(filename));
                                currentEditAnimationSetting_->targetModelDirection.setValue(directory);
                                currentEditAnimationSetting_->targetModelFileName.setValue(filename);
                                currentEditObject_->Initialize(directory, filename);
                            } else {
                                currentEditAnimationSetting_.reset(new AnimationSetting(filename));
                                currentEditObject_->Initialize(filename);
                            }
                        }
                    }
                    ImGui::EndMenu();
                }
                //===================== 保存 =====================//
                if (ImGui::MenuItem("Save")) {
                    if (currentEditAnimationSetting_) {
                        // アニメーション設定ファイルを保存
                        GlobalVariables::getInstance()->SaveFile(
                            "Animations",
                            currentEditAnimationSetting_->name);
                        // アニメーションを保存
                        if (currentEditObject_) {
                            AnimationManager::getInstance()->SaveAnimation(
                                currentEditAnimationSetting_->targetAnimationDirection,
                                currentEditAnimationSetting_->name,
                                *currentEditObject_->getAnimation()->getData());
                        }
                    }
                }

                //===================== 作成する =====================//
                if (ImGui::MenuItem("Create New")) {
                    openCreateNewPopup = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        //===================== メニューバーの外でポップアップを開く =====================//
        if (openCreateNewPopup) {
            ImGui::OpenPopup("Create_New_Animation");
            openCreateNewPopup = false;
        }

        //===================== ポップアップウィンドウ =====================//
        if (ImGui::BeginPopup("Create_New_Animation")) {
            // ファイル名の入力
            static char fileName[256] = "";
            ImGui::InputText("FileName", fileName, 256);
            // 作成ボタン
            if (ImGui::Button("Create")) {
                // ファイル名が空でない場合
                if (fileName[0] != '\0') {
                    // ファイルのパスを保存
                    currentEditAnimationSetting_.reset(new AnimationSetting(std::string(fileName)));
                    currentEditObject_ = std::make_unique<AnimationObject3d>();

                    // アニメーションデータを作成(Editor用のdemoData)
                    AnimationManager::getInstance()->addAnimationData(
                        fileName,
                        std::make_unique<AnimationData>());
                    std::unique_ptr<Animation> animation = std::make_unique<Animation>(
                        const_cast<AnimationData*>(AnimationManager::getInstance()->getAnimationData(fileName)));
                    currentEditObject_->setAnimation(animation);
                }
                ImGui::CloseCurrentPopup();
            }
            // キャンセルボタン
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ///================================================================
        /// アニメーションの設定
        ///================================================================
        if (currentEditAnimationSetting_) {
            //===================== アニメーション設定ファイルの編集 =====================//
            ImGui::Text("AnimationName :");
            {
                std::string preAnimationSettingName = currentEditAnimationSetting_->name;
                if (ImGui::InputText(
                        "##AnimationName",
                        currentEditAnimationSetting_->name.data(),
                        currentEditAnimationSetting_->name.capacity())) {
                    // アニメーション名の変更
                    preAnimationSettingName.erase(
                        std::remove_if(
                            preAnimationSettingName.begin(),
                            preAnimationSettingName.end(),
                            ::isspace),
                        preAnimationSettingName.end());
                    currentEditAnimationSetting_->name = currentEditAnimationSetting_->name.c_str();
                    GlobalVariables::getInstance()->ChangeGroupName(
                        "Animations",
                        preAnimationSettingName,
                        currentEditAnimationSetting_->name);
                }
            }

            //===================== アニメーション対象のモデルを選択 =====================//
            ImGui::Text("TargetModel :");
            if (ImGui::BeginCombo("##TargetModel", currentEditAnimationSetting_->targetModelFileName->c_str())) {
                // モデルリストを取得
                {
                    modelFileList_ = myfs::SearchFile("resource/Models", "gltf", false);
                    modelFileList_.splice(modelFileList_.begin(), myfs::SearchFile("resource/Models", "obj", false));
                }
                for (const auto& [directory, filename] : modelFileList_) {
                    bool isSelected = currentEditAnimationSetting_->targetModelFileName == filename;
                    if (ImGui::Selectable(filename.c_str(), isSelected)) {
                        // モデルのパスを保存
                        currentEditAnimationSetting_->targetModelDirection.setValue(directory);
                        currentEditAnimationSetting_->targetModelFileName.setValue(filename);
                        // モデルを読み込む
                        currentEditObject_->setModel(ModelManager::getInstance()->Create(directory, filename));

                        // モデルの読み込みが終わるまで待機
                        while (true) {
                            if (currentEditObject_->getModel()->meshData_->currentState_ == LoadState::Loaded) {
                                break;
                            }
                        }
                        // アニメーションを リセット
                        auto currentEditAnimationData = currentEditObject_->getAnimation()->getData();
                        currentEditAnimationData->nodeAnimations.clear();
                        for (const auto& [name, mesh] : currentEditObject_->getModel()->meshData_->meshGroup_) {
                            currentEditAnimationData->nodeAnimations[name].scale.push_back({0.0f, {1.0f, 1.0f, 1.0f}});
                            currentEditAnimationData->nodeAnimations[name].rotate.push_back({0.0f, {0.0f, 0.0f, 0.0f, 1.0f}});
                            currentEditAnimationData->nodeAnimations[name].translate.push_back({0.0f, {0.0f, 0.0f, 0.0f}});
                        }
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            //===================== アニメーションファイルの保存先を選択 =====================//
            ImGui::Text("AnimationDirectory : %s", currentEditAnimationSetting_->targetAnimationDirection->c_str());
            if (ImGui::Button("Select AnimationDirectory")) {
                std::string newDirection = "";
                myfs::SelectFolderDialog("resource", newDirection);
            }
        }
    }
    ImGui::End();

    ///================================================================
    /// アニメーションの編集
    ///================================================================
    if (currentEditObject_ && currentEditObject_->getModel() && currentEditObject_->getAnimation()->getData()) {
        ImGui::BeginGroup(); // グループ化
        std::string parentWindowName = currentEditAnimationSetting_->name + "TimeLines";
        if (ImGui::Begin(parentWindowName.c_str())) {

            // アニメーションの再生
            if (ImGui::Checkbox("Play", &isObjectPlaying_)) {
                currentEditObject_->getAnimation()->setCurrentAnimationTime(0.0f);
            }

            // アニメーションのタイムライン
            auto& nodeAnimations    = currentEditObject_->getAnimation()->getData()->nodeAnimations;
            float animationDuration = currentEditObject_->getAnimation()->getDuration();

            ImGui::Text("Duration :");
            if (ImGui::InputFloat(std::string("##" + currentEditAnimationSetting_->name + "Duration").c_str(), &animationDuration, 0.1f, 0.3f, "%.3f")) {
                float newDuration = animationDuration;
                float duration    = currentEditObject_->getAnimation()->getDuration();
                // 前の Duration に対する Scale の時間を変更
                for (auto& [nodeName, nodeAnimation] : nodeAnimations) {
                    for (auto& keyframe : nodeAnimation.scale) {
                        keyframe.time = (keyframe.time / duration) * newDuration;
                    }
                    // 前の Duration に対する Rotate の時間を変更
                    for (auto& keyframe : nodeAnimation.rotate) {
                        keyframe.time = (keyframe.time / duration) * newDuration;
                    }
                    // 前の Duration に対する Translate の時間を変更
                    for (auto& keyframe : nodeAnimation.translate) {
                        keyframe.time = (keyframe.time / duration) * newDuration;
                    }
                }
                // DurationUpdate
                currentEditObject_->getAnimation()->setDuration(newDuration);
                currentEditObject_->getAnimation()->getData()->duration = newDuration;
            }

            ImGui::Text("Local Time:");
            float currentTime = currentEditObject_->getAnimation()->getCurrentAnimationTime();
            if (ImGui::SliderFloat(std::string("##" + currentEditAnimationSetting_->name + "LocalTime").c_str(), &currentTime, 0.0f, animationDuration)) {
                currentEditObject_->getAnimation()->setCurrentAnimationTime(currentTime);
                // 現在の姿勢に更新
                currentEditObject_->Update(0.0f);
                // 更新を中断
                isObjectPlaying_ = false;
            }

            for (auto& [nodeName, nodeAnimation] : nodeAnimations) {
                if (ImGui::Begin(nodeName.c_str())) {
                    ImGui::BeginGroup(); // グループ化

                    // ============= 補間タイプ選択 ============= //
                    if (ImGui::BeginCombo("Interpolation", interpolationTypeString[(int)nodeAnimation.interpolationType].c_str())) {
                        for (int i = 0; i < static_cast<int>(InterpolationType::COUNT); i++) {
                            bool isSelected = (int)nodeAnimation.interpolationType == i;
                            if (ImGui::Selectable(interpolationTypeString[i].c_str(), isSelected)) {
                                nodeAnimation.interpolationType = static_cast<InterpolationType>(i);
                            }
                            if (isSelected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }

                    // ============= タイムラインの編集 ============= //
                    ImGui::Text("Scale");
                    ImGui::EditKeyFrame(
                        "Scale##" + nodeName,
                        nodeAnimation.scale,
                        animationDuration);

                    ImGui::Text("Rotate");
                    ImGui::EditKeyFrame(
                        "Rotate##" + nodeName,
                        nodeAnimation.rotate,
                        animationDuration);
                    ImGui::Text("Translate");
                    ImGui::EditKeyFrame(
                        "Translate##" + nodeName,
                        nodeAnimation.translate,
                        animationDuration);

                    ImGui::EndGroup(); // グループ化
                }
                ImGui::End();
            }
        }
        ImGui::End();
        ImGui::EndGroup();

        // アニメーションの再生
        if (isObjectPlaying_) {
            currentEditObject_->Update(Engine::getInstance()->getDeltaTime());
        } else {
            // 変更後の姿勢に更新
            currentEditObject_->Update(0.0f);
        }
    }

#endif // _DEBUG
}

void AnimationEditor::DrawEditObject() {
    // Object が　存在する場合 描画
    if (currentEditObject_ && currentEditObject_->getModel()) {
        currentEditObject_->Draw();
    }
}
