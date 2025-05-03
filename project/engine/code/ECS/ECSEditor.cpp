#include "ECSEditor.h"

/// stl
#include <algorithm>

/// engine
// editor
#include "module/editor/EditorGroup.h"

#ifdef _DEBUG

/// externals
#include <imgui/imgui.h>

ECSEditor::ECSEditor() {}

ECSEditor::~ECSEditor() {}

void ECSEditor::Initialize() {
    ecsManager_ = ECSManager::getInstance();

    editEntity_ = nullptr;

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

                EditorGroup::getInstance()->pushCommand(std::move(command));
            }

            // 選ばれたエンティティが あれば 表示
            if (!selectedEntities_.empty()) {
                if (ImGui::Button("Group Join Work System")) {
                    popupJoinWorkSystem_.isOpen_ = true;
                }
                if (ImGui::Button("Group Add Component")) {
                    popupAddComponent_.isOpen_ = true;
                }
                if (ImGui::Button("Group Leave Work System")) {
                    popupLeaveWorkSystem_.isOpen_ = true;
                }
            }

            ImGui::EndPopup();
        }

        if (ImGui::Button("Add Entity")) {
            auto command = std::make_unique<CreateEntityCommand>(this);

            EditorGroup::getInstance()->pushCommand(std::move(command));
        }
        // 選ばれたエンティティが あれば表示
        if (!selectedEntities_.empty()) {

            if (ImGui::Button("Group Erase")) {
                auto command = std::make_unique<GroupEraseEntityCommand>(this);

                EditorGroup::getInstance()->pushCommand(std::move(command));
            }

            ImGui::Spacing();
            if (ImGui::Button("Group Add Component")) {
                popupAddComponent_.isOpen_ = true;
            }
            if (ImGui::Button("Group Join Work System")) {
                popupJoinWorkSystem_.isOpen_ = true;
            }
            if (ImGui::Button("Group Leave Work System")) {
                popupLeaveWorkSystem_.isOpen_ = true;
            }
        }

        ImGui::Separator();
        ImGui::Text("Entity List");

        static char searchBuffer[128] = ""; // 検索用のバッファ
        ImGui::InputText("Search", searchBuffer, sizeof(searchBuffer));

        // Active Entities
        bool isEntitySelected  = false;
        std::string checkBoxID = "";
        std::string entityName = "";

        for (auto& entity : ecsManager_->getEntities()) {
            if (entity.isAlive()) {
                entityName = entity.isUnique() ? entity.getDataType() : entity.getUniqueID();
                // 検索フィルタリング
                if (strlen(searchBuffer) > 0 && entityName.find(searchBuffer) == std::string::npos) {
                    continue; // 検索文字列に一致しない場合はスキップ
                }

                isEntitySelected = false;
                isEntitySelected = std::find(selectedEntities_.begin(), selectedEntities_.end(), &entity) != selectedEntities_.end();

                checkBoxID = "##entitySelect_" + entity.getUniqueID();
                if (ImGui::Checkbox(checkBoxID.c_str(), &isEntitySelected)) {
                    if (!isEntitySelected) {
                        selectedEntities_.remove(&entity);
                    } else {
                        selectedEntities_.push_back(&entity);
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button(entityName.c_str())) {
                    auto command = std::make_unique<SelectEntityCommand>(this, const_cast<GameEntity*>(&entity));
                    EditorGroup::getInstance()->pushCommand(std::move(command));
                }
            }
        }

        ImGui::End();

        PopupEntityJoinWorkSystem(editEntity_, true);
        PopupEntityAddComponent(editEntity_, true);
        PopupEntityLeaveWorkSystem(editEntity_, true);
    }
}

void ECSEditor::EditEntity() {
    std::string label = "Entity Info";

    if (ImGui::Begin("Entity Info")) {
        // Entityが選択されていなければ Skip
        if (editEntity_ == nullptr || !editEntity_->isAlive()) {
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

            EditorGroup::getInstance()->pushCommand(std::move(command));

            ImGui::End();
            return;
        }

        ImGui::Text("Entity UniqueID   : %s", editEntity_->getUniqueID().c_str());
        ImGui::Text("Entity ID         : %d", editEntity_->getID());

        ImGui::Text("Entity Name       :");
        ImGui::SameLine();

        char buffer[64];
        strncpy_s(buffer, editEntity_->getDataType().c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0'; // 念のため終端文字を保証

        // ImGui::InputText を使用
        if (ImGui::InputText("##entityName", buffer, sizeof(buffer))) {
            // 入力が変更された場合のみ更新
            std::string newDataType(buffer);
            editEntity_->setDataType(newDataType);
        }

        ImGui::Text("Entity Is Unique  :");
        ImGui::SameLine();
        bool isUnique = editEntity_->isUnique();
        if (ImGui::Checkbox("##entityIsUnique", &isUnique)) {
            if (isUnique) {
                // Unique に設定 (登録できたら true,重複などで登録できなければ false)
                ecsManager_->registerUniqueEntity(editEntity_->getDataType(), editEntity_);
            } else {
                // Unique から解除
                ecsManager_->removeUniqueEntity(editEntity_->getDataType());
            }
        }

        ImGui::Separator();

        ImGui::Text("Components");
        ImGui::SameLine();
        if (ImGui::Button("AddComponent")) {
            popupAddComponent_.isOpen_ = true;
        }

        // 空のコンポーネントVectorを削除
        std::erase_if(editEntityComponents_, [](const std::pair<std::string, std::deque<IComponent*>>& compVec) { return compVec.second.empty(); });

        // コンポーネントの表示
        std::string componentLabel = "";
        int32_t componentIndex     = 0;

        for (auto& [compTypeName, compVec] : editEntityComponents_) {
            componentLabel = "";
            componentIndex = 0;
            for (auto& comp : compVec) {
                componentLabel = "X##" + compTypeName + "[" + std::to_string(componentIndex) + "]";
                if (ImGui::Button(componentLabel.c_str())) {
                    // 遅延実行用に削除コマンドをプッシュ
                    auto command = std::make_unique<RemoveComponentCommand>(this, editEntity_, compTypeName, componentIndex);
                    EditorGroup::getInstance()->pushCommand(std::move(command));
                }
                ImGui::SameLine();

                componentLabel = compTypeName + "[" + std::to_string(componentIndex) + "]";
                if (ImGui::CollapsingHeader(componentLabel.c_str())) {
                    ImGui::Indent();

                    comp->Edit();

                    ImGui::Unindent();
                }

                componentIndex++;
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
                    // Popupで 処理するために保持
                    if (ImGui::Button(systemName.c_str())) {
                        popupLeaveWorkSystem_.isOpen_ = true;
                        leaveSystemName_              = systemName;
                        leaveSystem_                  = system;
                    }
                }
            }
        }
    }

    ImGui::End();

    PopupEntityJoinWorkSystem(editEntity_, false);
    PopupEntityAddComponent(editEntity_, false);
    PopupEntityLeaveWorkSystem(editEntity_, false);
}

void ECSEditor::WorkerSystemList() {
    if (ImGui::Begin("Worker System List")) {
        std::string systemLabel = "";
        int32_t systemTypeIndex = 0;
        int systemPriority      = 0;

        for (auto& systemByType : workSystemList_) {
            // Typeごとで区切る
            if (ImGui::CollapsingHeader(SystemTypeString[systemTypeIndex].c_str())) {
                ImGui::Indent();

                SortPriorityOrderFromECSManager(systemTypeIndex);

                for (auto& [systemName, system] : systemByType) {

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

                    systemLabel   = "##" + systemName + "isActive";
                    bool isActive = system->isActive();
                    ImGui::Checkbox(systemLabel.c_str(), &isActive);
                    system->setIsActive(isActive);
                    ImGui::SameLine();

                    // Input Intで Priorityを変更 & 表示
                    ImGui::PushItemWidth(78);
                    systemLabel    = "##" + systemName + "_Priority";
                    systemPriority = system->getPriority();
                    if (ImGui::InputInt(systemLabel.c_str(), &systemPriority)) {
                        system->setPriority(systemPriority);
                    }
                    ImGui::PopItemWidth();
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
                ImGui::Unindent();
            }
            systemTypeIndex++;
        }
    }
    ImGui::End();
}

void ECSEditor::PopupEntityJoinWorkSystem(GameEntity* _entity, bool _isGroup) {
    if (!popupJoinWorkSystem_.isOpen_) {
        return;
    }

    if (_isGroup == selectedEntities_.empty()) {
        return;
    }

    static char searchBuffer[128] = ""; // 検索用のバッファ
    ImGui::Begin("Join Work System", &popupJoinWorkSystem_.isOpen_);
    if (popupJoinWorkSystem_.isOpen_ != false) {
        popupJoinWorkSystem_.isOpen_ = ImGui::IsWindowFocused();
    }

    ImGui::Text("Work Systems");

    ImGui::Spacing();

    // 検索欄
    ImGui::InputText("Search##WorkSysterm", searchBuffer, sizeof(searchBuffer));
    ImGui::Separator();
    int systemTypeIndex = 0;
    for (auto& systemByType : ecsManager_->getSystems()) {

        // 検索に一致するシステムがあるかを確認
        bool hasMatchingSystem = false;
        for (auto& [systemName, system] : systemByType) {
            if (strlen(searchBuffer) == 0 || systemName.find(searchBuffer) != std::string::npos) {
                hasMatchingSystem = true;
                break;
            }
        }

        // 一致する場合は CollapsingHeader を開く
        if (hasMatchingSystem) {
            ImGui::SetNextItemOpen(true);
        } else if (strlen(searchBuffer) != 0) {
            ImGui::SetNextItemOpen(false);
        }

        if (ImGui::CollapsingHeader(SystemTypeString[systemTypeIndex].c_str())) {
            for (auto& [systemName, system] : systemByType) {
                // 検索フィルタリング
                if (strlen(searchBuffer) > 0 && systemName.find(searchBuffer) == std::string::npos) {
                    continue; // 検索文字列に一致しない場合はスキップ
                }
                if (ImGui::Button(systemName.c_str())) {
                    if (_isGroup) {
                        // GroupCommand経由に変更
                        auto command = std::make_unique<GroupJoinWorkSystemCommand>(this, systemName, system.get());

                        EditorGroup::getInstance()->pushCommand(std::move(command));

                    } else {
                        // commandから 実行
                        auto command = std::make_unique<JoinWorkSystemCommand>(this, _entity, systemName, system.get());

                        EditorGroup::getInstance()->pushCommand(std::move(command));
                    }
                    // 実行したら閉じる
                    popupJoinWorkSystem_.isOpen_ = false;
                }
            }
        }
        ++systemTypeIndex;
    }

    ImGui::End();
}

void ECSEditor::PopupEntityAddComponent(GameEntity* _entity, bool _isGroup) {
    if (!popupAddComponent_.isOpen_) {
        return;
    }

    if (_isGroup == selectedEntities_.empty()) {
        return;
    }

    ImGui::Begin("AddComponent", &popupAddComponent_.isOpen_);
    if (popupAddComponent_.isOpen_ != false) {
        popupAddComponent_.isOpen_ = ImGui::IsWindowFocused();
    }

    ImGui::Spacing();

    static char searchBuffer[128] = ""; // 検索用のバッファ
    // 検索欄
    ImGui::InputText("Search##AddComponent", searchBuffer, sizeof(searchBuffer));
    ImGui::Separator();

    // コンポーネントの追加 → Command経由に変更
    for (auto& [componentTypeName, componentArray] : ecsManager_->getComponentArrayMap()) {
        // 検索フィルタリング
        if (strlen(searchBuffer) > 0 && componentTypeName.find(searchBuffer) == std::string::npos) {
            continue; // 検索文字列に一致しない場合はスキップ
        }

        if (ImGui::Button(componentTypeName.c_str())) {
            if (_isGroup) {
                // GroupCommand経由に変更
                auto command = std::make_unique<GroupAddComponentCommand>(this, componentTypeName);

                EditorGroup::getInstance()->pushCommand(std::move(command));
            } else {
                // commandから 実行
                auto command = std::make_unique<AddComponentCommand>(this, _entity, componentTypeName);

                EditorGroup::getInstance()->pushCommand(std::move(command));
            }
            // 実行したら閉じる
            popupAddComponent_.isOpen_ = false;
        }
    }
    ImGui::End();
}

void ECSEditor::PopupEntityLeaveWorkSystem(GameEntity* _entity, bool _isGroup) {
    if (!popupLeaveWorkSystem_.isOpen_) {
        return;
    };

    if (_isGroup == selectedEntities_.empty()) {
        return;
    }

    // IsGroup の場合, leaveSystemを決められるように
    ImGui::Begin("Leave Work System", &popupLeaveWorkSystem_.isOpen_);
    if (popupLeaveWorkSystem_.isOpen_ != false) {
        popupLeaveWorkSystem_.isOpen_ = ImGui::IsWindowFocused();
    }

    if (_isGroup) {
        if (!leaveSystem_) {
            ImGui::Text("Work Systems");
            ImGui::Spacing();
            static char searchBuffer[128] = ""; // 検索用のバッファ
            // 検索欄
            ImGui::InputText("Search##leaveSysterm", searchBuffer, sizeof(searchBuffer));

            ImGui::Separator();
            int systemTypeIndex = 0;
            for (auto& systemByType : ecsManager_->getSystems()) {
                if (ImGui::CollapsingHeader(SystemTypeString[systemTypeIndex].c_str())) {
                    // 検索に一致するシステムがあるかを確認
                    bool hasMatchingSystem = false;
                    for (auto& [systemName, system] : systemByType) {
                        if (strlen(searchBuffer) == 0 || systemName.find(searchBuffer) != std::string::npos) {
                            hasMatchingSystem = true;
                            break;
                        }
                    }

                    // 一致する場合は CollapsingHeader を開く
                    if (hasMatchingSystem) {
                        ImGui::SetNextItemOpen(true);
                    } else if (strlen(searchBuffer) != 0) {
                        ImGui::SetNextItemOpen(false);
                    }

                    for (auto& [systemName, system] : systemByType) {
                        if (strlen(searchBuffer) == 0 || systemName.find(searchBuffer) != std::string::npos) {
                            continue;
                        }
                        if (ImGui::Button(systemName.c_str())) {
                            leaveSystemName_ = systemName;
                            leaveSystem_     = system.get();
                        }
                    }
                }
                ++systemTypeIndex;
            }
        }
        ImGui::Separator();
    }

    if (leaveSystem_) {
        ImGui::Text("Leave %s ?", leaveSystemName_.c_str());

        if (ImGui::Button("Yes")) {
            if (_isGroup) {
                // GroupCommand経由に変更
                auto command = std::make_unique<GroupLeaveWorkSystemCommand>(this, leaveSystemName_, leaveSystem_);

                EditorGroup::getInstance()->pushCommand(std::move(command));
            } else {
                // commandから 実行
                auto command = std::make_unique<LeaveWorkSystemCommand>(this, _entity, leaveSystemName_, leaveSystem_);

                EditorGroup::getInstance()->pushCommand(std::move(command));
            }

            // 実行したら閉じる
            popupLeaveWorkSystem_.isOpen_ = false;
            leaveSystem_                  = nullptr;
            leaveSystemName_.clear();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            popupLeaveWorkSystem_.isOpen_ = false;

            leaveSystem_ = nullptr;
            leaveSystemName_.clear();
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

        // 追加したコンポーネントのインデックスを記録
        addedComponentIndex_ = addedComponentArray_->getComponentSize(entity_);
        if (addedComponentArray_->entityCapacity(entity_) <= 0) {
            int32_t newSize = (std::max)(10, addedComponentIndex_ * 2);
            addedComponentArray_->reserveEntity(entity_, newSize);
        }
        // コンポーネントの追加
        addedComponentArray_->addComponent(entity_);

        // 編集用 Component に追加
        if (entityIsEditEntity_) {
            ecsEditor_->customEditComponents()[componentTypeName_].clear();

            for (int32_t compIndex = 0; compIndex < addedComponentIndex_ + 1; ++compIndex) {
                IComponent* component = addedComponentArray_->getComponent(entity_, compIndex);
                if (component) {
                    ecsEditor_->customEditComponents()[componentTypeName_].push_back(component);
                }
            }
        }
    }
}

void AddComponentCommand::Undo() {
    addedComponentArray_->removeBackComponent(entity_);
    if (entityIsEditEntity_) {
        ecsEditor_->customEditComponents()[componentTypeName_].pop_back();
    }
}

void JoinWorkSystemCommand::Execute() {
    // エンティティをシステムに参加させる処理
    system_->addEntity(entity_);
    typeIdx_ = int32_t(system_->getSystemType());
    if (entityIsEditEntity_) {
        ecsEditor_->customEditEntitySystems()[typeIdx_][systemName_] = system_;
    }
}

void JoinWorkSystemCommand::Undo() {
    system_->removeEntity(entity_);
    if (entityIsEditEntity_) {
        ecsEditor_->customEditEntitySystems()[typeIdx_].erase(systemName_);
    }
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
                editComponents[componentTypeName].push_back(component);
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
                editEntitySystems[systemTypeIndex][systemName] = system.get();
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
                editComponents[componentTypeName].push_back(component);
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
                editEntitySystems[systemTypeIndex][systemName] = system.get();
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
        if (entityIsEditEntity_) {
            auto& editCOmponents = ecsEditor_->customEditComponents();
            editCOmponents[componentTypeName_].erase(editCOmponents[componentTypeName_].begin() + componentIndex_);
        }
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

        if (entityIsEditEntity_) {
            auto& editCOmponents = ecsEditor_->customEditComponents();
            editCOmponents[componentTypeName_].insert(editCOmponents[componentTypeName_].begin() + componentIndex_, removedComponentArray_->getComponent(entity_, componentIndex_));
        }
    }
}
#pragma endregion // Commands

void LeaveWorkSystemCommand::Execute() {
    // エンティティをシステムから離脱させる処理
    system_->removeEntity(entity_);
    typeIdx_ = int32_t(system_->getSystemType());
    if (entityIsEditEntity_) {
        ecsEditor_->customEditEntitySystems()[typeIdx_].erase(systemName_);
    }
}

void LeaveWorkSystemCommand::Undo() {
    // エンティティをシステムに参加させる処理
    system_->addEntity(entity_);
    if (entityIsEditEntity_) {
        ecsEditor_->customEditEntitySystems()[typeIdx_][systemName_] = system_;
    }
}

void ChangeEntityDataTypeCommand::Execute() {
    // エンティティのデータタイプを変更
    oldDataType_ = entity_->getDataType();
    entity_->setDataType(newDataType_);
}

void ChangeEntityDataTypeCommand::Undo() {
    // エンティティのデータタイプを戻す
    entity_->setDataType(oldDataType_);
}

#endif // _DEBUG
