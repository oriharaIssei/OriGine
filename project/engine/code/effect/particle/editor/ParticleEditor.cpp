#include "ParticleEditor.h"

#include "../emitter/Emitter.h"

/// engine
#include "Engine.h"
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
// dx12Object
#include "directX12/DxSrvArrayManager.h"
// lib
#include "camera/CameraManager.h"
#include "myFileSystem/MyFileSystem.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

const std::string emittersDirectory = kApplicationResourceDirectory + "/GlobalVariables/Emitters";

static std::list<std::pair<std::string, std::string>> emitterFiles = myfs::SearchFile(emittersDirectory, "json");

void ParticleEditor::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    dxSrvArray_ = DxSrvArrayManager::getInstance()->Create(srvNum_);

    for (auto& [directory, filename] : emitterFiles) {
        emitters_[filename] = std::make_unique<Emitter>(dxSrvArray_.get(), filename, 0);
        emitters_[filename]->Initialize();
    }
}

void ParticleEditor::Update() {
#ifdef _DEBUG

    // main window
    if (ImGui::Begin("ParticleEditor", nullptr, ImGuiWindowFlags_MenuBar)) {
        MenuBarUpdate();

        if (ImGui::BeginCombo("Emitter", currentEditEmitter_ ? currentEditEmitter_->emitterDataName_.c_str() : "NULL")) {
            for (auto& [emitterName, emitter] : emitters_) {
                bool isSelected = (currentEditEmitter_ == emitter.get());
                if (ImGui::Selectable(emitterName.c_str(), isSelected)) {
                    currentEditEmitter_ = emitter.get();
                }
            }
            ImGui::EndCombo();
        }

        if (currentEditEmitter_) {
            currentEditEmitter_->Debug();
            currentEditEmitter_->Update(Engine::getInstance()->getDeltaTime());
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
            if (emitters_.find(newInstanceName_) == emitters_.end()) {
                emitters_[newInstanceName_] = std::make_unique<Emitter>(dxSrvArray_.get(), newInstanceName_, 0);
                emitters_[newInstanceName_]->Initialize();
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

#endif // _DEBUG
}

void ParticleEditor::Draw() {
    if (currentEditEmitter_) {
        currentEditEmitter_->Draw();
    }
}

void ParticleEditor::Finalize() {
    if (dxCommand_) {
        dxCommand_->Finalize();
    }
    if (dxSrvArray_) {
        dxSrvArray_->Finalize();
    }
    emitters_.clear();
}

ParticleEditor::ParticleEditor() {}

ParticleEditor::~ParticleEditor() {}

void ParticleEditor::MenuBarUpdate() {
#ifdef _DEBUG

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::BeginMenu("Save")) {
                if (ImGui::MenuItem("ALL")) {
                    for (auto& [emitterName, emitter] : emitters_) {
                        emitter->Save();
                    }
                }
                if (ImGui::MenuItem("This")) {
                    if (currentEditEmitter_) {
                        currentEditEmitter_->Save();
                    }
                }
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Load")) {
                emitterFiles = myfs::SearchFile("resource/GlobalVariables/Effects", "json");
                for (auto& [directory, filename] : emitterFiles) {
                    if (ImGui::MenuItem(filename.c_str())) {
                        if (!emitters_[filename]) {
                            emitters_[filename] = std::make_unique<Emitter>(dxSrvArray_.get(), filename, 0);
                            emitters_[filename]->Initialize();
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

#endif // _DEBUG
}
