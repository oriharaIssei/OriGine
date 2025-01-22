#include "Effect.h"

///stl
//pointer
#include <functional>

//algorithm
#include <algorithm>
//stream
#include <fstream>

///engine
//Object
#include "object3d/AnimationObject3d.h"
#include "particle/emitter/Emitter.h"
//lib
#include "animationEditor/Timeline.h"
#include "myFileSystem/MyFileSystem.h"
//external
#include "imgui/imgui.h"

static std::list<std::pair<std::string, std::string>> LoadEmitters() {
    return myfs::SearchFile("resource/GlobalVariables/Emitters", "json");
}

static std::list<std::pair<std::string, std::string>> emitterLists_ = LoadEmitters();

Effect::Effect(std::shared_ptr<DxSrvArray> _srvArray, const std::string& _name)
    : dataName_(_name), srvArray_(_srvArray) {}

Effect::~Effect() {}

void Effect::Init() {
    isActive_ = true;

    isLoop_   = SerializedField<bool>("Effects", dataName_, "IsLoop");
    duration_ = SerializedField<float>("Effects", dataName_, "Duration");

    LoadCurve();
}

void Effect::Update(float _deltaTime) {
    if (!isActive_) {
        return;
    }

    preTime_ = currentTime_;
    currentTime_ += _deltaTime;

    if (currentTime_ > duration_) {
        if (isLoop_) {
            currentTime_ = 0.0f;
            preTime_     = 0.0f;
        } else {
            isActive_ = false;
            return;
        }
    }

    for (auto& emitter : particleSchedule_) {
        if (emitter.time < preTime_) {
            break;
        } else if (emitter.time > currentTime_) {
            break;
        }
        activeEmitters_.push_back(emitters_[emitter.value].get());
    }

    for (auto& emitter : activeEmitters_) {
        emitter->Update(_deltaTime);
    }
}

void Effect::Draw() {
    for (auto& emitter : activeEmitters_) {
        emitter->Draw();
    }
}

void Effect::Finalize() {
    if (emitters_.empty()) {
        return;
    }

    for (auto& emitter : emitters_) {
        emitter->Finalize();
    }
}

void Effect::Debug() {
    ImGui::Text("Name");
    std::string preName = dataName_;
    if (ImGui::InputText("##Name", &dataName_[0], sizeof(char) * 64)) {
        // 変更 ＆ 削除
        GlobalVariables::getInstance()->ChangeGroupName("Effects", preName, dataName_);
        MyFileSystem::deleteFile("resource/GlobalVariables/Effects/" + preName + ".bin");

        Save();
    }

    ImGui::Spacing();

    std::string windowLabel = dataName_ + "Emitters";
    float height            = 200.0f;
    ImVec2 childWindowSize  = ImVec2(ImGui::CalcItemWidth() * 0.5f, height);

    // 追加できるEmitterのリストを表示
    if (ImGui::BeginChild((windowLabel + "Addable").c_str(), childWindowSize, ImGuiChildFlags_Border)) {
        for (const auto& emitter : emitterLists_) {
            if (ImGui::Button(emitter.second.c_str())) {
                auto newEmitter = std::make_unique<Emitter>(srvArray_.get(), emitter.second, static_cast<int>(emitters_.size()));
                newEmitter->Init();
                emitters_.push_back(std::move(newEmitter));
            }
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                ImGui::SetDragDropPayload("DND_EFFECT", &emitter, sizeof(emitter));
                ImGui::Text("Dragging %s", emitter.second.c_str());
                ImGui::EndDragDropSource();
            }
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    int clickedEmitterIndex_ = -1;
    std::string emitterName;

    // 現在存在しているEmitterのリストを表示
    if (ImGui::BeginChild((windowLabel + "Existing").c_str(), childWindowSize, ImGuiChildFlags_Border)) {
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_EFFECT")) {
                auto payload_n  = static_cast<const std::pair<std::string, std::string>*>(payload->Data);
                auto newEmitter = std::make_unique<Emitter>(srvArray_.get(), payload_n->second, static_cast<int>(emitters_.size()));
                emitters_.push_back(std::move(newEmitter));
            }
            ImGui::EndDragDropTarget();
        }

        for (const auto& emitter : emitters_) {
            emitterName = std::to_string(emitter->getId()) + emitter->getDataName();
            if (ImGui::Button(emitterName.c_str())) {
                clickedEmitterIndex_ = emitter->getId();
            }
        }
    }
    if (clickedEmitterIndex_ != -1) {
        ImGui::OpenPopup("Delete Emitter");
    }
    if (ImGui::BeginPopup("Delete Emitter")) {
        ImGui::Text("Delete Emitter?");
        if (ImGui::Button("Yes")) {
            activeEmitters_.erase(
                std::remove_if(
                    activeEmitters_.begin(),
                    activeEmitters_.end(),
                    [clickedEmitterIndex_](const auto& emitter) {
                        return emitter->getId() == clickedEmitterIndex_;
                    }),
                activeEmitters_.end());
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("No")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::EndChild();

    ImGui::Checkbox("Active", &isActive_);
    ImGui::Checkbox("Loop", isLoop_);

    std::string timelineWindowLabel_ = dataName_ + "Timeline";
    if (ImGui::Begin(timelineWindowLabel_.c_str())) {
        ImGui::DragFloat("Duration", duration_, 0.001f);
        ImGui::SliderFloat("Time", &currentTime_, 0.0f, duration_);

        {
            // タイムラインで使用する関数たち
            std::function<void(float newNodeTime)> updateOnNodeDragged = [this](float newNodeTime) {
                std::sort(
                    particleSchedule_.begin(),
                    particleSchedule_.end(),
                    [](const auto& a, const auto& b) {
                        return a.time < b.time;
                    });
            };
            std::function<void(float _currentTime)> sliderPopupUpdate = [this](float _currentTime) {
                if (ImGui::Button("Add Node")) {
                    // 1つのエミッターに対して 1つのスケジュールしか設定できない
                    if (particleSchedule_.size() > emitters_.size()) {
                        return;
                    }
                    particleSchedule_.push_back({_currentTime, 0});
                    ImGui::CloseCurrentPopup();
                    return;
                }
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                    return;
                }
            };
            std::function<bool(int)> nodePopUpUpdate = [this](int _index) {
                if (ImGui::Button("Delete")) {
                    particleSchedule_.erase(particleSchedule_.begin() + _index);

                    if (particleSchedule_.empty()) {
                        particleSchedule_.emplace_back(0.0f, 0);
                    }

                    return false;
                }

                int emitterIndex        = particleSchedule_[_index].value;
                std::string emitterName = std::to_string(emitters_[emitterIndex]->getId()) + emitters_[emitterIndex]->getDataName();
                if (ImGui::BeginCombo("Emitter", emitterName.c_str())) {
                    for (int32_t i = 0; i < emitters_.size(); i++) {
                        bool isSelected = (emitterIndex == i); // 現在選択中かどうか
                        if (ImGui::Selectable(emitterName.c_str(), isSelected)) {
                            particleSchedule_[_index].value = i;
                            break;
                        }
                    }
                    ImGui::EndCombo();
                }
                return true;
            };

            ImGui::TimeLineButtons(
                dataName_ + "ParticleSchedule",
                particleSchedule_,
                duration_,
                updateOnNodeDragged,
                sliderPopupUpdate,
                nodePopUpUpdate);
        }
    }

    ImGui::End();

    for (auto& emitter : emitters_) {
        emitter->Debug();
    }
}

#pragma region "IO"
void Effect::LoadCurve() {
    std::string filePath = "resource/GlobalVariables/Effect/" + dataName_ + ".bin";

    // 1. fileを開く
    std::ofstream ofs(filePath, std::ios::binary);
    if (!ofs) {
        return;
    }

    auto writeCurve = [&ofs](const auto& curve) {
        size_t size = curve.size();
        ofs.write(reinterpret_cast<const char*>(&size), sizeof(size));
        for (const auto& keyframe : curve) {
            ofs.write(reinterpret_cast<const char*>(&keyframe.time), sizeof(keyframe.time));
            ofs.write(reinterpret_cast<const char*>(&keyframe.value), sizeof(keyframe.value));
        }
    };
    // 2. 書き込み
    writeCurve(particleSchedule_);

    //3. fileを閉じる
    ofs.close();
}

void Effect::SaveCurve() {
    std::string filePath = "resource/GlobalVariables/Effect/" + dataName_ + ".bin";

    // 1. fileを開く
    std::ifstream ifs(filePath, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("Failed to open file for reading");
    }
    auto readCurve = [&ifs](auto& curve) {
        size_t size;
        ifs.read(reinterpret_cast<char*>(&size), sizeof(size));
        curve.resize(size);
        for (auto& keyframe : curve) {
            ifs.read(reinterpret_cast<char*>(&keyframe.time), sizeof(keyframe.time));
            ifs.read(reinterpret_cast<char*>(&keyframe.value), sizeof(keyframe.value));
        }
    };
    // 2 書き込み
    readCurve(particleSchedule_);
    //3. fileを閉じる
    ifs.close();
}
void Effect::Save() {
    // 保存
    GlobalVariables::getInstance()->SaveFile("Effects", dataName_);
    SaveCurve();
}
void Effect::StartEmitter() {
    if (particleSchedule_.empty()) {
        return;
    }

    for (int i = currentEmitterIndex_; i < static_cast<int>(emitters_.size()); ++i) {
        if (particleSchedule_[i].time <= currentTime_) {
            activeEmitters_.push_back(emitters_[particleSchedule_[i].value].get());
            currentEmitterIndex_ = i;
        }
    }
}
#pragma endregion
