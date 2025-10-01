#include "EntityReferenceList.h"

/// engine
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"

/// editor
#include "editor/EditorController.h"
#include "myGui/MyGui.h"
/// util
#include "myFileSystem/MyFileSystem.h"

EntityReferenceList::EntityReferenceList() {
}

EntityReferenceList::~EntityReferenceList() {
}

void EntityReferenceList::Initialize(GameEntity* /*_entity*/) {
}

void EntityReferenceList::Edit(Scene* _scene, GameEntity* _entity, const std::string& _parentLabel) {
    std::string label = _parentLabel + "##EntityReferenceList";

    for (size_t i = 0; i < entityFileList_.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        std::string filePath = entityFileList_[i].first + "/" + entityFileList_[i].second;
        ImGui::Text("%s", filePath.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
            auto command = std::make_unique<EraseElementCommand<std::vector<std::pair<std::string, std::string>>>>(&entityFileList_, i);
            EditorController::getInstance()->pushCommand(std::move(command));

            ImGui::PopID();
            break;
        }
        ImGui::PopID();
    }

    if (ImGui::Button("Add Entity Reference")) {
        std::string directory, filename;
        if (MyFileSystem::selectFileDialog(kApplicationResourceDirectory, directory, filename, {".ent"}, true)) {
            auto command = std::make_unique<AddElementCommand<std::vector<std::pair<std::string, std::string>>>>(&entityFileList_, std::make_pair(directory, filename));
            EditorController::getInstance()->pushCommand(std::move(command));
        }
    }
}

void EntityReferenceList::Finalize() {}
