#include "AnimationEditor.h"

// stl
#include <algorithm>

// engine
#include "Engine.h"

// object
#include "object3d/AnimationObject3d.h"

// lib
#include "myFileSystem/MyFileSystem.h"

#ifdef _DEBUG
#include "Timeline.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#endif // _DEBUG

AnimationEditor::AnimationEditor() {}

AnimationEditor::~AnimationEditor() {}

void AnimationEditor::Init() {
    // アニメーションリストを取得
    animationSettingsFileList_ = myfs::SearchFile("resource/GlobalVariables/Animations", "json", false);
}

void AnimationEditor::Update() {
    static bool openCreateNewPopup = false;

    if (ImGui::Begin("AnimationEditor", nullptr, ImGuiWindowFlags_MenuBar)) {
        // メニューバー
        if (ImGui::BeginMenuBar()) {
            // ファイル
            if (ImGui::BeginMenu("File")) {
                // filelist reload
                if (ImGui::MenuItem("Reload")) {
                    animationSettingsFileList_ = myfs::SearchFile("resource/GlobalVariables/Animations", "json", false);
                }
                // 開く
                if (ImGui::BeginMenu("Open")) {
                    // アニメーション設定ファイルを開く
                    for (const auto& [directory, filename] : animationSettingsFileList_) {
                        if (ImGui::MenuItem(filename.c_str())) {
                            // ファイルのパスを保存
                            currentEditAnimationSetting_.reset(new AnimationSetting(filename));
                            // アニメーションを読み込む
                            currentEditObject_ = AnimationObject3d::Create(
                                currentEditAnimationSetting_->targetModelDirection_,
                                currentEditAnimationSetting_->targetModelFileName_);
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
                }
                ImGui::CloseCurrentPopup();
            }
            // キャンセルボタン
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (currentEditObject_) {
            // アニメーションの再生
            ImGui::Checkbox("Play", &isObjectPlaying_);

            ImGui::Text("[AnimationName]", currentEditAnimationSetting_->name.c_str());
            {
                std::string preAnimationSettingName = currentEditAnimationSetting_->name;
                if (ImGui::InputText("##AnimationName", currentEditAnimationSetting_->name.data(), currentEditAnimationSetting_->name.size())) {
                    currentEditAnimationSetting_->name = currentEditAnimationSetting_->name.c_str();
                    GlobalVariables::getInstance()->ChangeGroupName(
                        "Animations",
                        preAnimationSettingName,
                        currentEditAnimationSetting_->name);
                }
            }

            // アニメーション対象のモデル
            ImGui::InputText("##TargetModel", reinterpret_cast<char*>("TargetModel"), sizeof("TargetModel"), ImGuiInputTextFlags_ReadOnly);
            if (ImGui::BeginCombo("##TargetModel", currentEditAnimationSetting_->targetModelFileName_->c_str())) {
                // モデルリストを取得
                {
                    modelFileList_ = myfs::SearchFile("resource/Models", "gltf", false);
                    modelFileList_.splice(modelFileList_.begin(), myfs::SearchFile("resource/Models", "obj", false));
                }
                for (const auto& [directory, filename] : modelFileList_) {
                    bool isSelected = currentEditAnimationSetting_->targetModelFileName_ == filename;
                    if (ImGui::Selectable(filename.c_str(), isSelected)) {
                        currentEditAnimationSetting_->targetModelFileName_.setValue(filename);
                        currentEditObject_ = AnimationObject3d::Create(
                            currentEditAnimationSetting_->targetModelDirection_,
                            currentEditAnimationSetting_->targetModelFileName_);
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }
    }
    ImGui::End();

    if (currentEditObject_) {
        if (ImGui::Begin("Timelines")) {
            // アニメーションの再生
            ImGui::Checkbox("Play", &isObjectPlaying_);

            // アニメーションのタイムライン
            auto& nodeAnimations =
                currentEditObject_->getAnimation()->nodeAnimations;

            for (auto& [nodeName, nodeAnimation] : nodeAnimations) {
                if (ImGui::TreeNode(nodeName.c_str())) {
                    // scaleのキーフレーム時刻を取得
                    std::vector<float*> scaleTimes;
                    for (auto& keyframe : nodeAnimation.scale) {
                        scaleTimes.push_back(&keyframe.time);
                    }
                    // rotateのキーフレーム時刻を取得
                    std::vector<float*> rotateTimes;
                    for (auto& keyframe : nodeAnimation.rotate) {
                        rotateTimes.push_back(&keyframe.time);
                    }
                    // translateのキーフレーム時刻を取得
                    std::vector<float*> translateTimes;
                    for (auto& keyframe : nodeAnimation.translate) {
                        translateTimes.push_back(&keyframe.time);
                    }

                    // TimeLineButtonsを呼び出す
                    ImGui::TimeLineButtons(
                        std::string("Scale##" + nodeName).c_str(),
                        scaleTimes,
                        currentEditObject_->getAnimation()->duration,
                        [&](float newNodeTime) {
                            // キーフレームによる ノードの順番を変更
                            std::sort(
                                nodeAnimation.scale.begin(),
                                nodeAnimation.scale.end(),
                                [](const auto& a, const auto& b) {
                                    return a.time < b.time;
                                });
                        },
                        [&]() {
                            // SliderPopup
                            // Node の追加
                            if (ImGui::Button("Add Node")) {
                                nodeAnimation.scale.push_back(
                                    {0.0f, {1.0f, 1.0f, 1.0f}});
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

                            ImGui::Text("X:");
                            ImGui::DragFloat(
                                std::string("##X" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.scale[nodeIndex].value.x);
                            ImGui::Text("Y:");
                            ImGui::DragFloat(
                                std::string("##Y" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.scale[nodeIndex].value.y);
                            ImGui::Text("Z:");
                            ImGui::DragFloat(
                                std::string("##Z" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.scale[nodeIndex].value.z);
                        },
                        [&](float newDuration) {
                            // 前の Duration に対する Scale の時間を変更
                            for (auto& keyframe : nodeAnimation.scale) {
                                keyframe.time = (keyframe.time / currentEditObject_->getAnimation()->duration) * newDuration;
                            }

                            // DurationUpdate
                            currentEditObject_->getAnimation()->duration = newDuration;
                        });
                    ImGui::TimeLineButtons(
                        std::string("Rotate##" + nodeName).c_str(),
                        rotateTimes,
                        currentEditObject_->getAnimation()->duration,
                        [&](float newNodeTime) {
                            // キーフレームによる ノードの順番を変更
                            std::sort(
                                nodeAnimation.rotate.begin(),
                                nodeAnimation.rotate.end(),
                                [](const auto& a, const auto& b) {
                                    return a.time < b.time;
                                });
                        },
                        [&]() {
                            // SliderPopup
                            // Node の追加
                            if (ImGui::Button("Add Node")) {
                                nodeAnimation.rotate.push_back(
                                    {0.0f, {0.0f, 0.0f, 0.0f, 1.0f}});
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

                            ImGui::Text("X:");
                            ImGui::DragFloat(
                                std::string("##X" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.rotate[nodeIndex].value.x);
                            ImGui::Text("Y:");
                            ImGui::DragFloat(
                                std::string("##Y" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.rotate[nodeIndex].value.y);
                            ImGui::Text("Z:");
                            ImGui::DragFloat(
                                std::string("##Z" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.rotate[nodeIndex].value.z);
                            ImGui::Text("W:");
                            ImGui::DragFloat(
                                std::string("##W" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.rotate[nodeIndex].value.w);
                        },
                        [&](float newDuration) {
                            // 前の Duration に対する Rotate の時間を変更
                            for (auto& keyframe : nodeAnimation.rotate) {
                                keyframe.time = (keyframe.time / currentEditObject_->getAnimation()->duration) * newDuration;
                            }

                            // DurationUpdate
                            currentEditObject_->getAnimation()->duration = newDuration;
                        });
                    ImGui::TimeLineButtons(
                        std::string("Translate##" + nodeName).c_str(),
                        translateTimes,
                        currentEditObject_->getAnimation()->duration,
                        [&](float newNodeTime) {
                            // キーフレームによる ノードの順番を変更
                            std::sort(
                                nodeAnimation.translate.begin(),
                                nodeAnimation.translate.end(),
                                [](const auto& a, const auto& b) {
                                    return a.time < b.time;
                                });
                        },
                        [&]() {
                            // SliderPopup
                            // Node の追加
                            if (ImGui::Button("Add Node")) {
                                nodeAnimation.translate.push_back(
                                    {0.0f, {0.0f, 0.0f, 0.0f}});
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

                            ImGui::Text("X:");
                            ImGui::DragFloat(
                                std::string("##X" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.translate[nodeIndex].value.x);
                            ImGui::Text("Y:");
                            ImGui::DragFloat(
                                std::string("##Y" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.translate[nodeIndex].value.y);
                            ImGui::Text("Z:");
                            ImGui::DragFloat(
                                std::string("##Z" + nodeName + std::to_string(nodeIndex)).c_str(),
                                &nodeAnimation.translate[nodeIndex].value.z);
                        },
                        [&](float newDuration) {
                            // 前の Duration に対する Translate の時間を変更
                            for (auto& keyframe : nodeAnimation.translate) {
                                keyframe.time = (keyframe.time / currentEditObject_->getAnimation()->duration) * newDuration;
                            }

                            // DurationUpdate
                            currentEditObject_->getAnimation()->duration = newDuration;
                        });
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
    if (currentEditObject_) {
        currentEditObject_->Draw();
    }
}
