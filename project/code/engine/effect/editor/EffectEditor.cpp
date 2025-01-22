#include "EffectEditor.h"

///engine
#include "Engine.h"
//dx12Object
#include "directX12/DxSrvArrayManager.h"
// object
#include "effect/Effect.h"
//lib
#include "camera/CameraManager.h"
#include "myFileSystem/MyFileSystem.h"
//manager
#include "effect/manager/EffectManager.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

static std::list<std::pair<std::string, std::string>> emitterFiles = myfs::SearchFile("resource/GlobalVariables/Effects", "json");

EffectEditor::EffectEditor() {}

EffectEditor::~EffectEditor() {}

void EffectEditor::Init() {
    EffectManager* effectMaanger = EffectManager::getInstance();
    for (auto& [directory, filename] : emitterFiles) {
        effects_[filename] = effectMaanger->CreateEffect("filename");
    }
}

void EffectEditor::Update() {
    // main window
    if (ImGui::Begin("EffectEditor", nullptr, ImGuiWindowFlags_MenuBar)) {
        MenuBarUpdate();

        if (ImGui::BeginCombo("Effect", currentEditEffect_ ? currentEditEffect_->getDataName().c_str() : "NULL")) {
            for (auto& [emitterName, emitter] : effects_) {
                bool isSelected = (currentEditEffect_ == emitter.get());
                if (ImGui::Selectable(emitterName.c_str(), isSelected)) {
                    currentEditEffect_ = emitter.get();
                }
            }
            ImGui::EndCombo();
        }

        if (currentEditEffect_) {
            currentEditEffect_->Debug();
        }
    }
    ImGui::End();

    if (isOpenedCrateWindow_) {
        ImGui::Begin("Create New", &isOpenedCrateWindow_);
        ImGui::InputText("name", &newInstanceName_[0], sizeof(char) * 64, ImGuiInputTextFlags_CharsNoBlank);

        // 終端文字で切り詰める
        newInstanceName_ = std::string(newInstanceName_.c_str());

        if (ImGui::Button("Create")) {
            // 名前が既に存在している場合は登録しない
            if (effects_.find(newInstanceName_) == effects_.end()) {
                effects_[newInstanceName_] = EffectManager::getInstance()->CreateEffect(newInstanceName_);
            }
            isOpenedCrateWindow_ = false;
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            // window を閉じる
            isOpenedCrateWindow_ = false;
        }
        ImGui::End();
    } else {
        // 作成用文字列の初期化
        newInstanceName_ = "NULL";
    }
}

void EffectEditor::Draw() {
    if (currentEditEffect_) {
        currentEditEffect_->Draw();
    }
}

void EffectEditor::Finalize() {
    effects_.clear();
}

void EffectEditor::MenuBarUpdate() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::BeginMenu("Save")) {
                if (ImGui::MenuItem("ALL")) {
                    for (auto& [emitterName, emitter] : effects_) {
                        emitter->Save();
                    }
                }
                if (ImGui::MenuItem("This")) {
                    if (currentEditEffect_) {
                        currentEditEffect_->Save();
                    }
                }
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Load")) {
                emitterFiles = myfs::SearchFile("resource/GlobalVariables/Effects", "json");
                for (auto& [directory, filename] : emitterFiles) {
                    if (ImGui::MenuItem(filename.c_str())) {
                        if (!effects_[filename]) {
                            effects_[filename] = EffectManager::getInstance()->CreateEffect(filename);
                        }
                    }
                }
            }

            if (ImGui::MenuItem("Create")) {
                isOpenedCrateWindow_ = true;
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}
