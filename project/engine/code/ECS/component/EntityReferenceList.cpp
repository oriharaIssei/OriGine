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

void EntityReferenceList::Initialize(Entity* /*_entity*/) {}

void EntityReferenceList::Edit(Scene* /*_scene*/, Entity* /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    std::string label = "##" + _parentLabel;

    for (size_t i = 0; i < entityFileList_.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        std::string filePath = entityFileList_[i].first + "/" + entityFileList_[i].second;
        ImGui::Text("%s", filePath.c_str());

        ImGui::SameLine();

        label = "Remove##" + filePath + _parentLabel;
        if (ImGui::Button(label.c_str())) {
            auto command = std::make_unique<EraseElementCommand<std::vector<std::pair<std::string, std::string>>>>(&entityFileList_, entityFileList_.begin() + i);
            EditorController::GetInstance()->PushCommand(std::move(command));

            ImGui::PopID();
            break;
        }

        ImGui::PopID();
    }

    label = "Add Entity Reference##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        std::string directory, filename;
        if (MyFileSystem::SelectFileDialog(kApplicationResourceDirectory, directory, filename, {"ent"}, true)) {
            auto command = std::make_unique<AddElementCommand<std::vector<std::pair<std::string, std::string>>>>(&entityFileList_, std::make_pair(kApplicationResourceDirectory + "/" + directory, filename));
            EditorController::GetInstance()->PushCommand(std::move(command));
        }
    }

#endif // _DEBUG
}

void EntityReferenceList::Finalize() {}

void to_json(nlohmann::json& j, const EntityReferenceList& c) {
    j = nlohmann::json{{"entityFileList", c.entityFileList_}};
}

void from_json(const nlohmann::json& j, EntityReferenceList& c) {
    j.at("entityFileList").get_to(c.entityFileList_);
}
