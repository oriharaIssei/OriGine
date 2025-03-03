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
    ImGui::Begin("EntityList");
    for (auto& entity : ecsManager->getEntities()) {
        if (ImGui::Button(entity.getUniqueID().c_str())) {
            // 選択中のエンティティを設定
            GameEntity* selectedEntity            = const_cast<GameEntity*>(&entity);
            std::unique_ptr<IEditCommand> command = std::make_unique<SelectEntityCommand>(this, selectedEntity);
            command->Execute();
            EngineEditor::getInstance()->addCommand(std::move(command));
        }
    }
    ImGui::End();
}

void ECSEditor::EditComponent() {
    if (!editEntity_) {
        return;
    }

    ECSManager* ecsManager = ECSManager::getInstance();
    std::string label      = "Entity :";
    label += editEntity_->getUniqueID();

    std::string componentName = "UNKNOWN";
    if (ImGui::Begin(label.c_str())) {
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
