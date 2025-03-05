#include "ECSEditor.h"

/// engine
// editor
#include "editor/EngineEditor.h"

/// externals
#include <imgui/imgui.h>

ECSEditor::ECSEditor() {
    // Constructor implementation
}

ECSEditor::~ECSEditor() {
    // Destructor implementation
}

void ECSEditor::Update() {
    SelectEntity();
    EditComponent();
}

void ECSEditor::SelectEntity() {
    ECSManager* ecsManager = ECSManager::getInstance();
    if (ImGui::Begin("Entities")) {

        // エンティティを登録する
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_None) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("AddEntity");
        }
        if (ImGui::BeginPopup("AddEntity")) {
            if (ImGui::Button("AddEntity")) {
                uint32_t addedEntity = ecsManager->registerEntity("Entity");
                editEntity_          = ecsManager->getEntity(addedEntity);
            }
            ImGui::EndPopup();
        }

        // Active Entities
        if (ImGui::TreeNode("Active Entities")) {
            for (auto& entity : ecsManager->getEntities()) {
                // "Free" としてマークされていないものをActiveとする
                if (entity.getID() != -1) {
                    if (ImGui::Button(entity.getUniqueID().c_str())) {
                        // 選択中のエンティティを設定
                        GameEntity* selectedEntity            = const_cast<GameEntity*>(&entity);
                        std::unique_ptr<IEditCommand> command = std::make_unique<SelectEntityCommand>(this, selectedEntity);
                        command->Execute();
                        EngineEditor::getInstance()->addCommand(std::move(command));
                    }
                }
            }
            ImGui::TreePop();
        }

        // Inactive Entities
        if (ImGui::TreeNode("Inactive Entities")) {
            for (auto& entity : ecsManager->getEntities()) {
                // "Free" としてマークされたものをInactiveとする
                if (entity.getID() == -1) {
                    if (ImGui::Button("Inactive Entity")) { // 識別用ラベルは必要に応じて変更してください
                        GameEntity* selectedEntity            = const_cast<GameEntity*>(&entity);
                        std::unique_ptr<IEditCommand> command = std::make_unique<SelectEntityCommand>(this, selectedEntity);
                        command->Execute();
                        EngineEditor::getInstance()->addCommand(std::move(command));
                    }
                }
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ECSEditor::EditComponent() {
    ECSManager* ecsManager = ECSManager::getInstance();
    std::string label      = "Entity Info";

    std::string componentName = "UNKNOWN";
    if (ImGui::Begin("Entity Info")) {
        // Entityが選択されていなければ Skip
        if (editEntity_ == nullptr) {
            ImGui::Text("Entity is not selected.");
            ImGui::End();
            return;
        }

        // 右クリック + ホバーでポップアップを表示
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_None) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("EntityOption");
        }

        if (ImGui::BeginPopup("EntityOption")) {
            if (ImGui::Button("AddComponent")) {
                isOpenPopUpAddComponent_ = true;
            }
            ImGui::EndPopup();
        }

        if (isOpenPopUpAddComponent_) {
            isOpenPopUpAddComponent_ = ImGui::Begin("AddComponent");

            // コンポーネントの追加
            for (auto& [componentTypeName, componentArray] : ecsManager->getComponentArrayMap()) {
                if (ImGui::Button(componentTypeName.c_str())) {
                    componentArray->addComponent(editEntity_);
                    isOpenPopUpAddComponent_ = false;
                }
            }
            ImGui::End();
        }

        // ID と Name の表示
        ImGui::Text("Entity ID : %s", editEntity_->getUniqueID().c_str());
        ImGui::Text("Entity Name : ");
        ImGui::SameLine();
        ImGui::InputText("##entityName", const_cast<char*>(editEntity_->getDataType().c_str()), 256);

        ImGui::Separator();

        for (auto& [componentTypeName, componentArray] : ecsManager->getComponentArrayMap()) {
            int32_t index = 0;
            while (true) {
                IComponent* component = componentArray->getComponent(editEntity_, index);
                if (component) {
                    componentName = componentTypeName + std::to_string(index);
                    if (ImGui::CollapsingHeader(componentName.c_str())) {
                        ImGui::Text("Component Type : %s", componentTypeName.c_str());
                        ImGui::Separator();
                        component->Edit();
                    }
                    index++;
                } else {
                    break;
                }
            }
        }
    }
    ImGui::End();
}
