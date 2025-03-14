#include "ECSEditor.h"

/// stl
#include <algorithm>

/// engine
// editor
#include "editor/EngineEditor.h"

/// externals
#include <imgui/imgui.h>

ECSEditor::ECSEditor() {}

ECSEditor::~ECSEditor() {}

void ECSEditor::Initialize() {
    ecsManager_ = ECSManager::getInstance();

    for (int32_t i = 0; i < int32_t(SystemType::Count); i++) {
        workSystemList_[i].clear();
    }

    SortPriorityOrderFromECSManager();
}

void ECSEditor::Update() {
    SelectEntity();
    EditEntity();

    WorkerSystemList();
}

void ECSEditor::Finalize() {
    ecsManager_ = nullptr;
    editEntity_ = nullptr;

    editEntityComponents_.clear();
    for (int32_t i = 0; i < int32_t(SystemType::Count); i++) {
        workSystemList_[i].clear();
    }
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
                editEntityComponents_.clear();
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

void ECSEditor::EditEntity() {
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

        for (auto& [compName, comp] : editEntityComponents_) {
            if (ImGui::CollapsingHeader(compName.c_str())) {
                comp->Edit();
            }
        }

        ImGui::Separator();

        ImGui::Text("Work Systems");

        for (int32_t systemTypeIndex = 0; systemTypeIndex < int32_t(SystemType::Count); ++systemTypeIndex) {
            if (ImGui::CollapsingHeader(SystemTypeString[systemTypeIndex].c_str())) {
                for (auto& [systemName, system] : editEntitySystems_[systemTypeIndex]) {
                    ImGui::Text("%s", systemName.c_str());
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
        std::string systemLabel = "";
        int32_t systemTypeIndex = 0;
        int systemPriority      = 0;

        for (auto& systemByType : workSystemList_) {
            // Typeごとで区切る
            if (ImGui::CollapsingHeader(SystemTypeString[systemTypeIndex].c_str())) {

                SortPriorityOrderFromECSManager(systemTypeIndex);

                for (auto& [systemName, system] : systemByType) {
                    systemLabel    = "##" + systemName + "_Priority";
                    systemPriority = system->getPriority();

                    // Drag & Drop Source
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        ImGui::SetDragDropPayload("SYSTEM_PRIORITY", &systemPriority, sizeof(int));
                        ImGui::Text("Dragging %s", systemName.c_str());
                        ImGui::EndDragDropSource();
                    }

                    // Drag & Drop Target
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SYSTEM_PRIORITY")) {
                            IM_ASSERT(payload->DataSize == sizeof(int) && "Payload data size mismatch.");
                            int droppedPriority = *(const int*)payload->Data;

                            // アイテムの中央を計算
                            ImVec2 itemRectMin    = ImGui::GetItemRectMin();
                            ImVec2 itemRectMax    = ImGui::GetItemRectMax();
                            ImVec2 itemRectCenter = ImVec2((itemRectMin.x + itemRectMax.x) * 0.5f, (itemRectMin.y + itemRectMax.y) * 0.5f);

                            // ドロップされた位置に基づいて優先度を調整
                            if (ImGui::IsMouseHoveringRect(itemRectMin, itemRectCenter)) {
                                // 上側にドロップされた場合、優先度を減少
                                system->setPriority(droppedPriority - 1);
                            } else if (ImGui::IsMouseHoveringRect(itemRectCenter, itemRectMax)) {
                                // 下側にドロップされた場合、優先度を増加
                                system->setPriority(droppedPriority + 1);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }

                    // Input Intで Priorityを変更 & 表示
                    if (ImGui::InputInt(systemLabel.c_str(), &systemPriority)) {
                        system->setPriority(systemPriority);
                    }
                    ImGui::SameLine();
                    ImGui::Text("%s", systemName.c_str());
                }

                if (systemByType.size() > 2) {
                    // Sort
                    std::sort(
                        systemByType.begin(),
                        systemByType.end(),
                        [](const std::pair<std::string, ISystem*>& a,
                            const std::pair<std::string, ISystem*>& b) {
                            return a.second->getPriority() < b.second->getPriority();
                        });

                    ecsManager_->SortPriorityOrderSystems(systemTypeIndex);
                }
            }
            systemTypeIndex++;
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
        if (ImGui::CollapsingHeader(SystemTypeString[systemTypeIndex].c_str())) {
            for (auto& [systemName, system] : systemByType) {
                if (ImGui::Button(systemName.c_str())) {
                    system->addEntity(_entity);

                    // エンティティに紐づくシステムを更新
                    editEntitySystems_[systemTypeIndex].push_back(std::make_pair(systemName, system.get()));

                    isOpenPopupJoinWorkSystem_ = false;
                }
            }
            ++systemTypeIndex;
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

            // エンティティに紐づくコンポーネントを更新

            // コンポーネントの配列の最後のインデックスを取得
            int32_t compBackIndex = 0;
            while (true) {
                auto comp = componentArray->getComponent(_entity, compBackIndex);
                if (!comp) {
                    break;
                }
                ++compBackIndex;
            }

            // エンティティに紐づくコンポーネントを更新
            editEntityComponents_.push_back(std::make_pair(componentTypeName, componentArray->getComponent(_entity, compBackIndex - 1)));

            isOpenPopUpAddComponent_ = false;
        }
    }

    ImGui::End();
}

void ECSEditor::SortPriorityOrderFromECSManager() {
    const auto& systemsArray = ecsManager_->getSystems();
    int32_t systemTypeIndex  = 0;
    for (const auto& sysMap : systemsArray) {
        workSystemList_[systemTypeIndex].clear();
        for (const auto& [sysName, sysPtr] : sysMap) {
            // システム名とそのシステムに登録されているエンティティのリストを追加
            workSystemList_[int32_t(sysPtr->getSystemType())].push_back(std::make_pair(sysName, sysPtr.get()));
        }

        std::sort(
            workSystemList_[systemTypeIndex].begin(),
            workSystemList_[systemTypeIndex].end(),
            [](const std::pair<std::string, ISystem*>& a,
                const std::pair<std::string, ISystem*>& b) {
                return a.second->getPriority() < b.second->getPriority();
            });

        systemTypeIndex++;
    }
}

void ECSEditor::SortPriorityOrderFromECSManager(int32_t systemTypeIndex) {
    const auto& systemsArray = ecsManager_->getSystems()[systemTypeIndex];
    workSystemList_[systemTypeIndex].clear();
    for (const auto& [sysName, sysPtr] : systemsArray) {
        // システム名とそのシステムに登録されているエンティティのリストを追加
        workSystemList_[systemTypeIndex].push_back(std::make_pair(sysName, sysPtr.get()));
    }

    std::sort(
        workSystemList_[systemTypeIndex].begin(),
        workSystemList_[systemTypeIndex].end(),
        [](const std::pair<std::string, ISystem*>& a,
            const std::pair<std::string, ISystem*>& b) {
            return a.second->getPriority() < b.second->getPriority();
        });
}

void SelectEntityCommand::Execute() {
    ecsEditor_->setEditEntity(nextEntity_);

    auto& editComponents = ecsEditor_->customEditComponents();
    editComponents.clear();

    ECSManager* ecsManager = ECSManager::getInstance();
    // エンティティに紐づくコンポーネントを取得
    for (auto& [componentTypeName, componentArray] : ecsManager->getComponentArrayMap()) {
        int32_t index = 0;
        while (true) {
            IComponent* component = componentArray->getComponent(ecsEditor_->getEditEntity(), index);
            if (component) {
                editComponents.push_back(std::make_pair(componentTypeName + "(" + std::to_string(index) + ")", component));
                index++;
            } else {
                break;
            }
        }
    }

    // エンティティに紐づくシステムを取得
    auto& editEntitySystems = ecsEditor_->customEditEntitySystems();
    for (int32_t systemTypeIndex = 0; systemTypeIndex < int32_t(SystemType::Count); ++systemTypeIndex) {
        // システムをクリア
        editEntitySystems[systemTypeIndex].clear();
        for (auto& [systemName, system] : ecsManager->getSystemsBy(SystemType(systemTypeIndex))) {
            // システムにエンティティが登録されている場合のみ追加
            if (system->hasEntity(nextEntity_)) {
                editEntitySystems[systemTypeIndex].push_back(std::make_pair(systemName, system.get()));
            }
        }
    }
}

void SelectEntityCommand::Undo() {
    ecsEditor_->setEditEntity(preEntity_);
}
