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
#include "Timeline.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#endif // _DEBUG

AnimationEditor::AnimationEditor()
    : IEditor() {}

AnimationEditor::~AnimationEditor() {}

void AnimationEditor::Init() {
    // アニメーションリストを取得
    //json
    animationSettingsFileList_ = myfs::SearchFile("resource/GlobalVariables/Animations", "json");

    // gltf
    std::list<std::pair<std::string, std::string>> addFileList = myfs::SearchFile("resource", "gltf", false);
    animationSettingsFileList_.insert(animationSettingsFileList_.end(), addFileList.begin(), addFileList.end());

    // editInstance を作成
    currentEditObject_ = std::make_unique<AnimationObject3d>();
}

void AnimationEditor::Update() {
    bool openCreateNewPopup = false;

    if (ImGui::Begin("AnimationEditor", nullptr, ImGuiWindowFlags_MenuBar)) {
        // メニューバー
        if (ImGui::BeginMenuBar()) {
            // ファイル
            if (ImGui::BeginMenu("File")) {
                // filelist reload
                if (ImGui::MenuItem("Reload")) {
                    // アニメーションリストを取得
                    //json
                    animationSettingsFileList_ = myfs::SearchFile("resource/GlobalVariables/Animations", "json");

                    // gltf
                    std::list<std::pair<std::string, std::string>> addFileList = myfs::SearchFile("resource", "gltf", false);
                    animationSettingsFileList_.insert(animationSettingsFileList_.end(), addFileList.begin(), addFileList.end());
                }
                // 開く
                if (ImGui::BeginMenu("Open")) {
                    // アニメーション設定ファイルを開く
                    for (const auto& [directory, filename] : animationSettingsFileList_) {
                        if (ImGui::MenuItem(filename.c_str())) {
                            // ファイルのパスを保存
                            if (filename.find("gltf") != std::string::npos) {
                                currentEditAnimationSetting_.reset(new AnimationSetting(filename));
                                currentEditAnimationSetting_->targetModelDirection.setValue(directory);
                                currentEditAnimationSetting_->targetModelFileName.setValue(filename);
                                currentEditObject_->Init(directory, filename);
                            } else {
                                currentEditAnimationSetting_.reset(new AnimationSetting(filename));
                                currentEditObject_->Init(filename);
                            }
                        }
                    }
                    ImGui::EndMenu();
                }
                // 保存
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

                // 作成する
                if (ImGui::MenuItem("Create New")) {
                    openCreateNewPopup = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // メニューバーの外でポップアップを開く
        if (openCreateNewPopup) {
            ImGui::OpenPopup("Create_New_Animation");
            openCreateNewPopup = false;
        }

        // ポップアップウィンドウ
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

        if (currentEditAnimationSetting_) {
            ImGui::Text("AnimationName :");
            {
                std::string preAnimationSettingName = currentEditAnimationSetting_->name;
                if (ImGui::InputText(
                        "##AnimationName",
                        currentEditAnimationSetting_->name.data(),
                        currentEditAnimationSetting_->name.capacity())) {
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

            // アニメーション対象のモデル
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
                        //モデルを読み込む
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
                        for (const auto& mesh : currentEditObject_->getModel()->meshData_->mesh_) {
                            currentEditAnimationData->nodeAnimations[mesh.nodeName].scale.push_back({0.0f, {1.0f, 1.0f, 1.0f}});
                            currentEditAnimationData->nodeAnimations[mesh.nodeName].rotate.push_back({0.0f, {0.0f, 0.0f, 0.0f, 1.0f}});
                            currentEditAnimationData->nodeAnimations[mesh.nodeName].translate.push_back({0.0f, {0.0f, 0.0f, 0.0f}});
                        }
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // アニメーション の directory
            ImGui::Text("AnimationDirectory : %s", currentEditAnimationSetting_->targetAnimationDirection->c_str());
            if (ImGui::Button("Select AnimationDirectory")) {
                std::string newDirection = "";
                myfs::SelectFolderDialog("resource", newDirection);
            }
        }
    }
    ImGui::End();

    if (currentEditObject_ &&
        currentEditObject_->getModel() &&
        currentEditObject_->getAnimation()) {

        if (ImGui::Begin("Timelines")) {
            // アニメーションの再生
            if (ImGui::Checkbox("Play", &isObjectPlaying_)) {
                currentEditObject_->getAnimation()->setCurrentAnimationTime(0.0f);
            }

            // アニメーションのタイムライン
            auto& nodeAnimations    = currentEditObject_->getAnimation()->getData()->nodeAnimations;
            float animationDuration = currentEditObject_->getAnimation()->getDuration();

            for (auto& [nodeName, nodeAnimation] : nodeAnimations) {
                if (ImGui::TreeNode(nodeName.c_str())) {
                    ImGui::BeginGroup(); // グループ化
                    ImGui::Text("Duration :");
                    if (ImGui::InputFloat(std::string("##" + nodeName + "Duration").c_str(), &animationDuration, 0.1f, 0.3f, "%.3f")) {
                        float newDuration = animationDuration;
                        float duration    = currentEditObject_->getAnimation()->getDuration();
                        // 前の Duration に対する Scale の時間を変更
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

                        // DurationUpdate
                        currentEditObject_->getAnimation()->setDuration(newDuration);
                        currentEditObject_->getAnimation()->getData()->duration = newDuration;
                    }

                    // scaleのキーフレーム時刻を取得
                    std::vector<float*> scaleTimes;
                    // rotateのキーフレーム時刻を取得
                    std::vector<float*> rotateTimes;
                    // translateのキーフレーム時刻を取得
                    std::vector<float*> translateTimes;

                    {
                        for (auto& keyframe : nodeAnimation.scale) {
                            scaleTimes.push_back(&keyframe.time);
                        }
                        for (auto& keyframe : nodeAnimation.rotate) {
                            rotateTimes.push_back(&keyframe.time);
                        }
                        for (auto& keyframe : nodeAnimation.translate) {
                            translateTimes.push_back(&keyframe.time);
                        }
                    }

                    // TimeLineButtonsを呼び出す
                    ImGui::TimeLineButtons(
                        std::string("Scale##" + nodeName).c_str(),
                        scaleTimes,
                        animationDuration,
                        [&](float newNodeTime) {
                            // キーフレームによる ノードの順番を変更
                            std::sort(
                                nodeAnimation.scale.begin(),
                                nodeAnimation.scale.end(),
                                [](const auto& a, const auto& b) {
                                    return a.time < b.time;
                                });
                        },
                        [&](float currentTime) {
                            // SliderPopup
                            // Node の追加
                            if (ImGui::Button("Add Node")) {
                                nodeAnimation.scale.push_back(
                                    {currentTime, {1.0f, 1.0f, 1.0f}});
                                ImGui::CloseCurrentPopup();
                                return;
                            }
                            if (ImGui::Button("Cancel")) {
                                ImGui::CloseCurrentPopup();
                                return;
                            }
                        },
                        [&](int nodeIndex) {
                            // NodeUpdate
                            ImGui::Text("NodeIndex : %d", nodeIndex);

                            if (ImGui::Button("Delete")) {
                                nodeAnimation.scale.erase(nodeAnimation.scale.begin() + nodeIndex);
                                return;
                            }
                            ImGui::Text("Time");
                            ImGui::DragFloat(
                                std::string("##Time" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.scale[nodeIndex].time,
                                0.1f);

                            ImGui::Spacing();

                            ImGui::Text("X:");
                            ImGui::DragFloat(
                                std::string("##X" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.scale[nodeIndex].value.x,
                                0.1f);
                            ImGui::Text("Y:");

                            ImGui::DragFloat(
                                std::string("##Y" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.scale[nodeIndex].value.y,
                                0.1f);
                            ImGui::Text("Z:");
                            ImGui::DragFloat(
                                std::string("##Z" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.scale[nodeIndex].value.z,
                                0.1f);
                        });
                    ImGui::TimeLineButtons(
                        std::string("Rotate##" + nodeName).c_str(),
                        rotateTimes,
                        animationDuration,
                        [&](float newNodeTime) {
                            // キーフレームによる ノードの順番を変更
                            std::sort(
                                nodeAnimation.rotate.begin(),
                                nodeAnimation.rotate.end(),
                                [](const auto& a, const auto& b) {
                                    return a.time < b.time;
                                });
                        },
                        [&](float currentTime) {
                            // SliderPopup
                            // Node の追加
                            if (ImGui::Button("Add Node")) {
                                nodeAnimation.rotate.push_back(
                                    {currentTime, {0.0f, 0.0f, 0.0f, 1.0f}});
                                ImGui::CloseCurrentPopup();
                                return;
                            }
                            if (ImGui::Button("Cancel")) {
                                ImGui::CloseCurrentPopup();
                                return;
                            }
                        },
                        [&](int nodeIndex) {
                            // NodeUpdate
                            ImGui::Text("NodeIndex : %d", nodeIndex);

                            if (ImGui::Button("Delete")) {
                                nodeAnimation.rotate.erase(nodeAnimation.rotate.begin() + nodeIndex);
                                return;
                            }
                            ImGui::Text("Time");
                            ImGui::DragFloat(
                                std::string("##Time" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.rotate[nodeIndex].time,
                                0.1f);

                            ImGui::Spacing();

                            ImGui::Text("X:");
                            ImGui::DragFloat(
                                std::string("##X" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.rotate[nodeIndex].value.x,
                                0.1f);
                            ImGui::Text("Y:");

                            ImGui::DragFloat(
                                std::string("##Y" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.rotate[nodeIndex].value.y,
                                0.1f);
                            ImGui::Text("Z:");
                            ImGui::DragFloat(
                                std::string("##Z" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.rotate[nodeIndex].value.z,
                                0.1f);
                            ImGui::Text("W:");
                            ImGui::DragFloat(
                                std::string("##W" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.rotate[nodeIndex].value.w,
                                0.1f);
                        });
                    ImGui::TimeLineButtons(
                        std::string("Translate##" + nodeName).c_str(),
                        translateTimes,
                        animationDuration,
                        [&](float newNodeTime) {
                            // キーフレームによる ノードの順番を変更
                            std::sort(
                                nodeAnimation.translate.begin(),
                                nodeAnimation.translate.end(),
                                [](const auto& a, const auto& b) {
                                    return a.time < b.time;
                                });
                        },
                        [&](float currentTime) {
                            // SliderPopup
                            // Node の追加
                            if (ImGui::Button("Add Node")) {
                                nodeAnimation.translate.push_back(
                                    {currentTime, {0.0f, 0.0f, 0.0f}});
                                ImGui::CloseCurrentPopup();
                                return;
                            }
                            if (ImGui::Button("Cancel")) {
                                ImGui::CloseCurrentPopup();
                                return;
                            }
                        },
                        [&](int nodeIndex) {
                            // NodeUpdate
                            ImGui::Text("NodeIndex : %d", nodeIndex);

                            if (ImGui::Button("Delete")) {
                                nodeAnimation.translate.erase(nodeAnimation.translate.begin() + nodeIndex);
                                return;
                            }
                            ImGui::Text("Time");
                            ImGui::DragFloat(
                                std::string("##Time" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.translate[nodeIndex].time,
                                0.1f);

                            ImGui::Spacing();

                            ImGui::Text("X:");
                            ImGui::DragFloat(
                                std::string("##X" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.translate[nodeIndex].value.x,
                                0.1f);
                            ImGui::Text("Y:");

                            ImGui::DragFloat(
                                std::string("##Y" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.translate[nodeIndex].value.y,
                                0.1f);
                            ImGui::Text("Z:");
                            ImGui::DragFloat(
                                std::string("##Z" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.translate[nodeIndex].value.z,
                                0.1f);
                        });

                    ImGui::EndGroup(); // グループ化

                    ImGui::TreePop();
                }
            }
        }
        ImGui::End();

        // アニメーションの再生
        if (isObjectPlaying_) {
            currentEditObject_->Update(Engine::getInstance()->getDeltaTime());
        }
    }
}

void AnimationEditor::DrawEditObject() {
    // Object が　存在する場合 描画
    if (currentEditObject_ && currentEditObject_->getModel()) {
        currentEditObject_->Draw();
    }
}
