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

        // Command経由に変更
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_None) && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("EntityOption");
        }
        if (ImGui::BeginPopup("EntityOption")) {
            if (ImGui::Button("AddEntity")) {
                auto command = std::make_unique<CreateEntityCommand>(this);
                command->Execute();
                EngineEditor::getInstance()->addCommand(std::move(command));
            }
            ImGui::EndPopup();
        }

        // Active Entities
        if (ImGui::TreeNode("Active Entities")) {
            for (auto& entity : ecsManager_->getEntities()) {
                if (entity.isAlive()) {
                    if (ImGui::Button(entity.getUniqueID().c_str())) {
                        auto command = std::make_unique<SelectEntityCommand>(this, const_cast<GameEntity*>(&entity));
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
                if (!entity.isAlive()) {
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

        // PopUp で Entity に対する操作を表示
        if (ImGui::BeginPopup("EntityOption")) {
            if (ImGui::Button("AddComponent")) {
                popupAddComponent_.isOpen_ = true;
            }
            if (ImGui::Button("Join Work System")) {
                popupJoinWorkSystem_.isOpen_ = true;
            }
            ImGui::EndPopup();
        }

        // ID と Name の表示
        if (ImGui::Button("Erase This")) {
            auto command = std::make_unique<EraseEntityCommand>(this, editEntity_);
            command->Execute();
            EngineEditor::getInstance()->addCommand(std::move(command));
        }

        ImGui::Text("Entity ID   : %s", editEntity_->getUniqueID().c_str());
        ImGui::Text("Entity Name : ");
        ImGui::SameLine();
        ImGui::InputText("##entityName", const_cast<char*>(editEntity_->getDataType().c_str()), 256);

        ImGui::Separator();

        ImGui::Text("Components");
        ImGui::SameLine();
        if (ImGui::Button("AddComponent")) {
            popupAddComponent_.isOpen_ = true;
        }

        // コンポーネントの表示
        for (auto& [compName, comp] : editEntityComponents_) {
            if (ImGui::CollapsingHeader(compName.c_str())) {
                comp->Edit();
            }
        }

        ImGui::Separator();

        ImGui::Text("Work Systems");
        ImGui::SameLine();
        if (ImGui::Button("Join Work System")) {
            popupJoinWorkSystem_.isOpen_ = true;
        }

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
    if (!popupJoinWorkSystem_.isOpen_) {
        return;
    }

    ImGui::Begin("Join Work System", &popupJoinWorkSystem_.isOpen_);
    popupJoinWorkSystem_.isOpen_ = ImGui::IsWindowFocused();

    ImGui::Text("Work Systems");
    int systemTypeIndex = 0;
    for (auto& systemByType : ecsManager_->getSystems()) {
        if (ImGui::CollapsingHeader(SystemTypeString[systemTypeIndex].c_str())) {
            for (auto& [systemName, system] : systemByType) {
                if (ImGui::Button(systemName.c_str())) {
                    // commandから 実行
                    auto command = std::make_unique<JoinWorkSystemCommand>(this, _entity, systemName, system.get());
                    command->Execute();
                    EngineEditor::getInstance()->addCommand(std::move(command));
                }
            }
        }
        ++systemTypeIndex;
    }

    ImGui::End();
}

void ECSEditor::PopupEntityAddComponent(GameEntity* _entity) {
    if (!popupAddComponent_.isOpen_) {
        return;
    }
    ImGui::Begin("AddComponent", &popupAddComponent_.isOpen_);
    popupAddComponent_.isOpen_ = ImGui::IsWindowFocused();

    // コンポーネントの追加 → Command経由に変更
    for (auto& [componentTypeName, componentArray] : ecsManager_->getComponentArrayMap()) {
        if (ImGui::Button(componentTypeName.c_str())) {
            // commandから 実行
            auto command = std::make_unique<AddComponentCommand>(this, _entity, componentTypeName);
            command->Execute();
            EngineEditor::getInstance()->addCommand(std::move(command));
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
    const auto& systemsArray = ecsManager_->getSystems();

    workSystemList_[systemTypeIndex].clear();
    for (const auto& [sysName, sysPtr] : systemsArray[systemTypeIndex]) {
        // システム名とそのシステムに登録されているエンティティのリストを追加
        workSystemList_[systemTypeIndex].push_back(std::make_pair(sysName, sysPtr.get()));
    }

    std::sort(workSystemList_[systemTypeIndex].begin(),
        workSystemList_[systemTypeIndex].end(),
        [](const std::pair<std::string, ISystem*>& a,
            const std::pair<std::string, ISystem*>& b) {
            return a.second->getPriority() < b.second->getPriority();
        });
}

#pragma region "Commands"

void CreateEntityCommand::Execute() {
    ECSManager* ecsManager = ECSManager::getInstance();
    // エンティティ作成処理の抽出
    uint32_t addedEntity = ecsManager->registerEntity("Entity");
    ecsEditor_->setEditEntity(ecsManager->getEntity(addedEntity));
    ecsEditor_->customEditComponents().clear();
    for (int32_t i = 0; i < int32_t(SystemType::Count); i++) {
        ecsEditor_->customEditEntitySystems()[i].clear();
    }
}

void CreateEntityCommand::Undo() {
    DestroyEntity(ecsEditor_->getEditEntity());

    // 編集中のエンティティをクリア
    ecsEditor_->setEditEntity(nullptr);
    // 編集中のコンポーネントをクリア
    ecsEditor_->customEditComponents().clear();
    // 編集中のシステムをクリア
    for (int32_t i = 0; i < int32_t(SystemType::Count); i++) {
        ecsEditor_->customEditEntitySystems()[i].clear();
    }
}

void AddComponentCommand::Execute() {
    ECSManager* ecsManager = ECSManager::getInstance();
    // ECSManagerから対象の ComponentArray を取得してコンポーネント追加
    auto& compMap     = ecsManager->getComponentArrayMap();
    auto compArrayItr = compMap.find(componentTypeName_);

    if (compArrayItr != compMap.end()) {
        addedComponentArray_ = compArrayItr->second.get();
        // コンポーネントの追加
        addedComponentArray_->addComponent(entity_);
        // 追加したコンポーネントのインデックスを記録
        addedComponentIndex_ = addedComponentArray_->getComponentSize(entity_) - 1;

        // 編集用 Component に追加
        auto* addedComponent_ = addedComponentArray_->getComponent(entity_, addedComponentIndex_);
        ecsEditor_->customEditComponents().push_back({componentTypeName_ + "(" + std::to_string(addedComponentIndex_) + ")", addedComponent_});
    }
}

void AddComponentCommand::Undo() {
    addedComponentArray_->removeBackComponent(entity_);
    ecsEditor_->customEditComponents().pop_back();
}

void JoinWorkSystemCommand::Execute() {
    // エンティティをシステムに参加させる処理
    system_->addEntity(entity_);
    typeIdx_ = int32_t(system_->getSystemType());
    ecsEditor_->customEditEntitySystems()[typeIdx_].push_back({systemName_, system_});
}

void JoinWorkSystemCommand::Undo() {
    system_->removeEntity(entity_);
    ecsEditor_->customEditEntitySystems()[typeIdx_].pop_back();
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

void EraseEntityCommand::Execute() {
    ECSManager* ecsManager = ECSManager::getInstance();

    // BackUp 用のデータを取得
    erasedEntityBackup_ = *erasedEntity_;

    // erasedEntity に紐づくコンポーネントを取得
    for (auto& [componentTypeName, componentArray] : ecsManager->getComponentArrayMap()) {
        int32_t index = 0;
        while (true) {
            IComponent* component = componentArray->getComponent(erasedEntity_, index);
            if (component) {
                erasedEntityComponents_.push_back(std::make_pair(componentTypeName + "(" + std::to_string(index) + ")", component));
                index++;
            } else {
                break;
            }
        }
    }

    // erasedEntity に紐づくシステムを取得
    for (int32_t systemTypeIndex = 0; systemTypeIndex < int32_t(SystemType::Count); ++systemTypeIndex) {
        for (auto& [systemName, system] : ecsManager->getSystemsBy(SystemType(systemTypeIndex))) {
            if (system->hasEntity(erasedEntity_)) {
                erasedEntitySystems_[systemTypeIndex].push_back(std::make_pair(systemName, system.get()));
            }
        }
    }

    // エンティティを削除
    DestroyEntity(erasedEntity_);

    // 編集中のエンティティをクリア
    auto command = std::make_unique<SelectEntityCommand>(ecsEditor_, nullptr);
    command->Execute();
}
void EraseEntityCommand::Undo() {
    ECSManager* ecsManager = ECSManager::getInstance();

    // エンティティを再生成
    uint32_t addedEntity       = ecsManager->registerEntity(erasedEntityBackup_.getDataType());
    GameEntity* addedEntityPtr = ecsManager->getEntity(addedEntity);

    // コンポーネントを追加
    for (auto& [componentTypeName, component] : erasedEntityComponents_) {

        auto& compMap     = ecsManager->getComponentArrayMap();
        auto compArrayItr = compMap.find(componentTypeName);

        if (compArrayItr != compMap.end()) {
            IComponentArray* componentArray = compArrayItr->second.get();
            componentArray->addComponent(addedEntityPtr);
        }
    }

    // システムに参加
    for (int32_t systemTypeIndex = 0; systemTypeIndex < int32_t(SystemType::Count); ++systemTypeIndex) {
        for (auto& [systemName, system] : erasedEntitySystems_[systemTypeIndex]) {
            system->addEntity(addedEntityPtr);
        }
    }

    // 編集中のエンティティをセット
    auto command = std::make_unique<SelectEntityCommand>(ecsEditor_, addedEntityPtr);
    command->Execute();
}

void RemoveComponentCommand::Execute() {
    ECSManager* ecsManager = ECSManager::getInstance();
    // ECSManagerから対象の ComponentArray を取得してコンポーネント削除
    auto& compMap     = ecsManager->getComponentArrayMap();
    auto compArrayItr = compMap.find(componentTypeName_);

    if (compArrayItr != compMap.end()) {
        IComponentArray* removedComponentArray_ = compArrayItr->second.get();

        // remove
        removedComponentArray_->removeComponent(entity_, componentIndex_);
        ecsEditor_->customEditComponents().pop_back();
    }
}

void RemoveComponentCommand::Undo() {
    ECSManager* ecsManager = ECSManager::getInstance();
    // ECSManagerから対象の ComponentArray を取得してコンポーネント追加
    auto& compMap     = ecsManager->getComponentArrayMap();
    auto compArrayItr = compMap.find(componentTypeName_);

    if (compArrayItr != compMap.end()) {
        IComponentArray* removedComponentArray_ = compArrayItr->second.get();
        // 追加
        removedComponentArray_->addComponent(entity_);
        ecsEditor_->customEditComponents().push_back({componentTypeName_, removedComponentArray_->getBackComponent(entity_)});
    }
}
#pragma endregion // Commands
