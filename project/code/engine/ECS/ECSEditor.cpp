#include "ECSEditor.h"

/// engine
// editor
#include "editor/EngineEditor.h"

/// externals
#include <imgui/imgui.h>

ECSEditor::ECSEditor() {
    ecsManager_ = ECSManager::getInstance();
}

ECSEditor::~ECSEditor() {
    // Destructor implementation
}

void ECSEditor::Update() {
    SelectEntity();
    EditComponent();

    WorkerSystemList();
}

void ECSEditor::SelectEntity() {
    if (ImGui::Begin("Entities")) {

        // エンティティを登録する
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_None) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("EntityOption");
        }
        if (ImGui::BeginPopup("EntityOption")) {
            if (ImGui::Button("AddEntity")) {
                uint32_t addedEntity = ecsManager_->registerEntity("Entity");
                editEntity_          = ecsManager_->getEntity(addedEntity);
            }
            ImGui::EndPopup();
        }

        // Active Entities
        if (ImGui::TreeNode("Active Entities")) {
            for (auto& entity : ecsManager_->getEntities()) {
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
            for (auto& entity : ecsManager_->getEntities()) {
                // "Free" としてマークされたものをInactiveとする
                if (entity.getID() == -1) {
                    ImGui::Text("Inactive Entity");
                }
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ECSEditor::EditComponent() {
    std::string label = "Entity Info";

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
            if (ImGui::Button("Join Work System")) {
                isOpenPopupJoinWorkSystem_ = true;
            }
            ImGui::EndPopup();
        }

        // ID と Name の表示
        ImGui::Text("Entity ID : %s", editEntity_->getUniqueID().c_str());
        ImGui::Text("Entity Name : ");
        ImGui::SameLine();
        ImGui::InputText("##entityName", const_cast<char*>(editEntity_->getDataType().c_str()), 256);

        ImGui::Separator();

        for (auto& [componentTypeName, componentArray] : ecsManager_->getComponentArrayMap()) {
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

    PopupEntityJoinWorkSystem(editEntity_);
    PopupEntityAddComponent(editEntity_);
}

void ECSEditor::WorkerSystemList() {
    if (ImGui::Begin("Worker System List")) {
        int32_t systemTypeIndex = 0;
        for (auto& systemMap : ecsManager_->getSystems()) {

            if (ImGui::CollapsingHeader(SystemTypeString[systemTypeIndex++].c_str())) {
                for (auto& [systemName, system] : systemMap) {
                    if (ImGui::TreeNode(systemName.c_str())) {

                        for (auto& entity : system->getEntities()) {
                            if (ImGui::Button(entity->getUniqueID().c_str())) {
                                system->removeEntity(entity);
                            }
                        }

                        ImGui::TreePop();
                    }
                }
            }
        }
    }
    ImGui::End();
}

void ECSEditor::PopupEntityJoinWorkSystem(GameEntity* _entity) {
    if (!isOpenPopupJoinWorkSystem_) {
        return;
    }

    ImGui::Begin("Join Work System", &isOpenPopupJoinWorkSystem_);

    ImGui::Text("Work Systems");
    int systemTypeIndex = 0;
    for (auto& systemByType : ecsManager_->getSystems()) {
        if (ImGui::CollapsingHeader(SystemTypeString[systemTypeIndex++].c_str())) {
            for (auto& [systemName, system] : systemByType) {
                if (ImGui::Button(systemName.c_str())) {
                    system->addEntity(_entity);
                    isOpenPopupJoinWorkSystem_ = false;
                }
            }
        }
    }

    ImGui::End();
}

void ECSEditor::PopupEntityAddComponent(GameEntity* _entity) {
    if (!isOpenPopUpAddComponent_) {
        return;
    }
    ImGui::Begin("AddComponent", &isOpenPopUpAddComponent_);

    // コンポーネントの追加
    for (auto& [componentTypeName, componentArray] : ecsManager_->getComponentArrayMap()) {
        if (ImGui::Button(componentTypeName.c_str())) {
            componentArray->addComponent(_entity);
            isOpenPopUpAddComponent_ = false;
        }
    }

    ImGui::End();
}
