#include "EntityInspector.h"

#ifdef _DEBUG

/// engine
// scene
#include "scene/SceneManager.h"

/// editor
#include "editor/EditorController.h"
#include "editor/sceneEditor/SceneEditor.h"

/// externals
#include "myGui/MyGui.h"

#pragma region "EntityInspector"

EntityInspectorArea::EntityInspectorArea(SceneEditorWindow* _window)
    : parentWindow_(_window), Editor::Area(nameof<EntityInspectorArea>()) {}
EntityInspectorArea::~EntityInspectorArea() {}
void EntityInspectorArea::Initialize() {
    addRegion(std::make_shared<EntityInfomationRegion>(this));
    addRegion(std::make_shared<EntityComponentRegion>(this));
    addRegion(std::make_shared<EntitySystemRegion>(this));
}
void EntityInspectorArea::Finalize() {
    for (auto& [name, region] : regions_) {
        region->Finalize();
        region.reset();
    }
    regions_.clear();

    parentWindow_ = nullptr; // 親ウィンドウへのポインタをクリア
}

EntityInfomationRegion::EntityInfomationRegion(EntityInspectorArea* _parent)
    : Editor::Region(nameof(this)), parentArea_(_parent) {}
EntityInfomationRegion::~EntityInfomationRegion() {}
void EntityInfomationRegion::Initialize() {}
void EntityInfomationRegion::DrawGui() {
    auto currentScene    = parentArea_->getParentWindow()->getCurrentScene();
    int32_t editEntityId = parentArea_->getEditEntityId();
    auto editEntity      = currentScene->getEntityRepositoryRef()->getEntity(editEntityId);

    ImGui::Text("Entity Information");
    if (!editEntity) {
        ImGui::Text("No entity selected.");
        return;
    }

    if (ImGui::Button("Delete")) {
        auto deleteCommand = std::make_unique<DeleteEntityCommand>(parentArea_, editEntityId);
        EditorController::getInstance()->pushCommand(std::move(deleteCommand));

        auto changeEditEntityCommand = std::make_unique<EntityInspectorArea::ChangeEditEntityCommand>(parentArea_, -1, editEntityId);
        EditorController::getInstance()->pushCommand(std::move(changeEditEntityCommand));
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Entity ID  : %d", editEntityId);
    {
        std::string label = "EntityName##" + std::to_string(editEntityId);
        ImGui::InputText(label.c_str(), &parentArea_->getEditEntityNameRef()[0], sizeof(char) * 256);

        if (ImGui::IsItemDeactivatedAfterEdit()) {
            auto command = std::make_unique<ChangeEntityName>(parentArea_, editEntityId, parentArea_->getEditEntityName());
            EditorController::getInstance()->pushCommand(std::move(command));
        }
    }

    bool isUnique = editEntity->isUnique();
    if (ImGui::Checkbox("Entity is Unique", &isUnique)) {
        auto command = std::make_unique<ChangeEntityUniqueness>(parentArea_, editEntityId, editEntity->isUnique());
        EditorController::getInstance()->pushCommand(std::move(command));
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Unique entities cannot be duplicated.");
    }

    ImGui::Separator();
}
void EntityInfomationRegion::Finalize() {}

EntityInfomationRegion::ChangeEntityName::ChangeEntityName(EntityInspectorArea* _inspectorArea, int32_t _entityID, const std::string& _newName)
    : inspectorArea_(_inspectorArea), entityId_(_entityID), newName_(_newName) {
    auto currentScene = inspectorArea_->getParentWindow()->getCurrentScene();
    oldName_          = currentScene->getEntityRepositoryRef()->getEntity(entityId_)->getDataType();
}

EntityComponentRegion::EntityComponentRegion(EntityInspectorArea* _parent)
    : Editor::Region(nameof<EntityComponentRegion>()), parentArea_(_parent) {}
EntityComponentRegion::~EntityComponentRegion() {}
void EntityComponentRegion::Initialize() {}
void EntityComponentRegion::DrawGui() {
    auto currentScene    = parentArea_->getParentWindow()->getCurrentScene();
    int32_t editEntityId = parentArea_->getEditEntityId();
    auto editEntity      = currentScene->getEntityRepositoryRef()->getEntity(editEntityId);

    if (!ImGui::CollapsingHeader("Entity Components", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    if (!editEntity) {
        return;
    }
    ImGui::Indent();

    if (ImGui::Button("+ Component")) {
        auto selectAddComponentArea = parentArea_->getParentWindow()->getAreas().find("SelectAddComponentArea")->second.get();
        if (selectAddComponentArea) {
            auto selectArea = dynamic_cast<SelectAddComponentArea*>(selectAddComponentArea);
            if (selectArea) {
                selectArea->setTargets({editEntityId});
                selectArea->setOpen(true);
            } else {
                LOG_ERROR("Failed to cast SelectAddComponentArea.");
            }
        } else {
            LOG_ERROR("SelectAddComponentArea not found in parent window.");
        }
    }

    auto& entityComponentMap = parentArea_->getEntityComponentMap();
    for (const auto& [componentTypeName, components] : entityComponentMap) {
        if (ImGui::CollapsingHeader(componentTypeName.c_str())) {
            ImGui::Indent();
            if (components.size() > 1) {
                int32_t componentIndex = 0;
                std::string label      = "";
                for (const auto& component : components) {
                    label = componentTypeName + std::to_string(componentIndex);

                    if (ImGui::TreeNode(label.c_str())) {
                        component->Edit();
                        ImGui::TreePop();
                    }

                    ++componentIndex;
                }
            } else {
                components[0]->Edit();
            }
            ImGui::Unindent();
        }
    }

    ImGui::Unindent();
}
void EntityComponentRegion::Finalize() {}

EntitySystemRegion::EntitySystemRegion(EntityInspectorArea* _parent)
    : Editor::Region(nameof<EntitySystemRegion>()), parentArea_(_parent) {}
EntitySystemRegion::~EntitySystemRegion() {}
void EntitySystemRegion::Initialize() {}
void EntitySystemRegion::DrawGui() {
    auto currentScene    = parentArea_->getParentWindow()->getCurrentScene();
    int32_t editEntityId = parentArea_->getEditEntityId();
    auto editEntity      = currentScene->getEntityRepositoryRef()->getEntity(editEntityId);

    if (!ImGui::CollapsingHeader("Entity Systems", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    if (!editEntity) {
        return;
    }

    ImGui::Indent();

    if (ImGui::Button("+ System")) {
        auto selectAddSystemArea = parentArea_->getParentWindow()->getAreas().find("SelectAddSystemArea")->second.get();
        if (selectAddSystemArea) {
            auto selectArea = dynamic_cast<SelectAddSystemArea*>(selectAddSystemArea);
            if (selectArea) {
                std::list<int32_t> targets = {editEntityId};
                auto setTargetsCommand     = std::make_unique<SelectAddSystemArea::SetTargeEntities>(selectArea, targets);
                EditorController::getInstance()->pushCommand(std::move(setTargetsCommand));
                selectArea->setOpen(true);
                selectArea->setFocused(true);
            } else {
                LOG_ERROR("Failed to cast SelectAddSystemArea.");
            }
        } else {
            LOG_ERROR("SelectAddSystemArea not found in parent window.");
        }
    }

    auto& systemMap = parentArea_->getSystemMap();
    for (size_t i = 0; i < systemMap.size(); ++i) {
        auto& systems = systemMap[i];
        if (systems.empty()) {
            continue;
        }
        if (ImGui::CollapsingHeader(SystemCategoryString[i].c_str())) {
            ImGui::Indent();
            for (auto& [systemName, system] : systems) {
                if (ImGui::TreeNode(systemName.c_str())) {
                    system->Edit();
                    ImGui::TreePop();
                }
            }
            ImGui::Unindent();
        }
    }

    ImGui::Unindent();
}
void EntitySystemRegion::Finalize() {}

SelectAddComponentArea::SelectAddComponentArea(SceneEditorWindow* _parentWindow)
    : Editor::Area(nameof(this)), parentWindow_(_parentWindow) {
    isOpen_.set(false);
    isFocused_.set(false);
}
SelectAddComponentArea::~SelectAddComponentArea() {}

void SelectAddComponentArea::Initialize() {
    addRegion(std::make_shared<ComponentListRegion>(this));
    // 初期化時にコンポーネントのタイプ名をクリア
    componentTypeNames_.clear();
    targetEntityIds_.clear();
}

void SelectAddComponentArea::DrawGui() {
    bool isOpen = isOpen_.current();
    if (!isOpen) {
        return;
    }

    ImGui::Begin(name_.c_str(), &isOpen);
    if (!ImGui::IsWindowFocused()) {
        ImGui::SetWindowFocus(); // このウィンドウを最前面に
    }

    isOpen_.set(isOpen);
    isFocused_.set(ImGui::IsWindowFocused());

    areaSize_ = ImGui::GetWindowSize();

    for (auto& [name, region] : regions_) {
        if (region) {
            ImGui::BeginGroup();
            region->DrawGui();
            ImGui::EndGroup();
        }
    }

    UpdateFocusAndOpenState();

    ImGui::End();
}

void SelectAddComponentArea::Finalize() {
    Editor::Area::Finalize();
    componentTypeNames_.clear(); // コンポーネントのタイプ名をクリア
    targetEntityIds_.clear(); // 対象のエンティティIDリストをクリア
}

void SelectAddComponentArea::setTargets(const std::list<int32_t>& _targets) {
    targetEntityIds_ = _targets;
}

SelectAddComponentArea::ComponentListRegion::ComponentListRegion(SelectAddComponentArea* _parentArea) : Editor::Region(nameof(this)), parentArea_(_parentArea) {}
SelectAddComponentArea::ComponentListRegion::~ComponentListRegion() {}

void SelectAddComponentArea::ComponentListRegion::Initialize() {
}

void SelectAddComponentArea::ComponentListRegion::DrawGui() {
    std::string label = "Search##SelectAddComponent";

    ImGui::InputText(label.c_str(), &searchBuff_[0], sizeof(char) * 256);
    searchBuff_ = std::string(searchBuff_.c_str());

    float parentHeight = ImGui::GetWindowHeight();
    float itemHeight   = ImGui::GetItemRectSize().y * 2.f; // 直前のアイテムの高さ
    float padding      = ImGui::GetStyle().WindowPadding.y * 6.f;

    // 必要に応じてパディングを2倍（上下分）にする場合も
    float childHeight = parentHeight - (itemHeight + padding);

    ImGui::BeginChild(
        "ComponentList",
        ImVec2(0, childHeight),
        ImGuiChildFlags_Border);

    auto& componentRegistryMap = ComponentRegistry::getInstance()->getComponentArrayMap();

    // ImGuiのスタイルで選択色を設定（必要に応じてアプリ全体で設定してもOK）
    ImVec4 winSelectColor = ImVec4(0.26f, 0.59f, 0.98f, 1.0f); // Windows風の青
    ImGui::PushStyleColor(ImGuiCol_Header, winSelectColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, winSelectColor);
    for (auto& [name, array] : componentRegistryMap) {

        if (searchBuff_.size() > 0) {
            if (name.find(searchBuff_) == std::string::npos) {
                continue; // 検索文字列に一致しない場合はスキップ
            }
        }

        // 選択状態か判定
        bool isSelected = std::find(parentArea_->componentTypeNames_.begin(), parentArea_->componentTypeNames_.end(), name) != parentArea_->componentTypeNames_.end();

        // Selectableで表示
        if (ImGui::Selectable(name.c_str(), isSelected)) {
            if (!isSelected) {
                if (!ImGui::GetIO().KeyShift) {
                    auto clearCommand = std::make_unique<ClearComponentTypeNames>(parentArea_);
                    EditorController::getInstance()->pushCommand(std::move(clearCommand));
                }
                // まだ選択されていなければ追加
                auto command = std::make_unique<AddComponentTypeNames>(parentArea_, name);
                EditorController::getInstance()->pushCommand(std::move(command));
            } else {
                // すでに選択されていれば解除
                auto command = std::make_unique<RemoveComponentTypeNames>(parentArea_, name);
                EditorController::getInstance()->pushCommand(std::move(command));
            }
        }
    }

    ImGui::PopStyleColor(3); // スタイルのポップ

    ImGui::EndChild();

    ImGui::Spacing();

    if (parentArea_->componentTypeNames_.empty()) {
        bool selected = false;
        ImGui::Selectable("OK", &selected, ImGuiSelectableFlags_Disabled);
    } else {
        if (ImGui::Button("OK")) {
            // OKボタンが押された場合、選択されたコンポーネントを追加
            for (auto& compTypeName : parentArea_->componentTypeNames_) {
                auto addCompCommand = std::make_unique<AddComponentCommand>(parentArea_->targetEntityIds_, compTypeName);
                EditorController::getInstance()->pushCommand(std::move(addCompCommand));
            }

            auto clearEntitiesCommand = std::make_unique<ClearTargetEntities>(parentArea_);
            EditorController::getInstance()->pushCommand(std::move(clearEntitiesCommand));

            auto clearComponentNamesCommand = std::make_unique<ClearComponentTypeNames>(parentArea_);
            EditorController::getInstance()->pushCommand(std::move(clearComponentNamesCommand));

            parentArea_->isOpen_.set(false); // エリアを閉じる
            parentArea_->isFocused_.set(false); // フォーカスを外す
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("CANCEL")) {
        // キャンセルボタンが押された場合、選択をクリア
        auto clearEntitiesCommand = std::make_unique<ClearTargetEntities>(parentArea_);
        EditorController::getInstance()->pushCommand(std::move(clearEntitiesCommand));

        auto clearComponentNamesCommand = std::make_unique<ClearComponentTypeNames>(parentArea_);
        EditorController::getInstance()->pushCommand(std::move(clearComponentNamesCommand));

        parentArea_->isOpen_.set(false); // エリアを閉じる
        parentArea_->isFocused_.set(false); // フォーカスを外す
    }
}

void SelectAddComponentArea::ComponentListRegion::Finalize() {}

SelectAddSystemArea::SelectAddSystemArea(SceneEditorWindow* _parentWindow)
    : Editor::Area(nameof<SelectAddSystemArea>()), parentWindow_(_parentWindow) {}
SelectAddSystemArea::~SelectAddSystemArea() {}

void SelectAddSystemArea::Initialize() {
    addRegion(std::make_shared<SystemListRegion>(this));

    isOpen_.set(false); // 初期状態では閉じている
    isFocused_.set(false); // 初期状態ではフォーカスされていない
}
void SelectAddSystemArea::DrawGui() {
    bool isOpen = isOpen_.current();
    if (!isOpen) {
        return;
    }

    ImGui::Begin(name_.c_str(), &isOpen);
    if (!ImGui::IsWindowFocused()) {
        ImGui::SetWindowFocus(); // このウィンドウを最前面に
    }

    isOpen_.set(isOpen);
    isFocused_.set(ImGui::IsWindowFocused());

    areaSize_ = ImGui::GetWindowSize();

    for (auto& [name, region] : regions_) {
        if (region) {
            ImGui::BeginGroup();
            region->DrawGui();
            ImGui::EndGroup();
        }
    }

    UpdateFocusAndOpenState();

    ImGui::End();
}
void SelectAddSystemArea::Finalize() {
    Editor::Area::Finalize();
    systemTypeNames_.clear(); // システムのタイプ名をクリア
    targetEntityIds_.clear(); // 対象のエンティティIDリストをクリア
}
void SelectAddSystemArea::setTargets(const std::list<int32_t>& _targets) {
    targetEntityIds_ = _targets;
}

SelectAddSystemArea::SystemListRegion::SystemListRegion(SelectAddSystemArea* _parentArea) : Editor::Region(nameof(this)), parentArea_(_parentArea) {}
SelectAddSystemArea::SystemListRegion::~SystemListRegion() {}
void SelectAddSystemArea::SystemListRegion::Initialize() {
}
void SelectAddSystemArea::SystemListRegion::DrawGui() {
    std::string label = "Search##SelectAddSystem";
    ImGui::InputText(label.c_str(), &searchBuff_[0], sizeof(char) * 256);
    searchBuff_ = std::string(searchBuff_.c_str());

    float parentHeight = ImGui::GetWindowHeight();
    float itemHeight   = ImGui::GetItemRectSize().y * 2.f; // 直前のアイテムの高さ
    float padding      = ImGui::GetStyle().WindowPadding.y * 6.f;

    // 必要に応じてパディングを2倍（上下分）にする場合も
    float childHeight = parentHeight - (itemHeight + padding);

    ImGui::BeginChild(
        "ComponentList",
        ImVec2(0, childHeight),
        ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysUseWindowPadding);

    auto currentScene = EditorController::getInstance()->getWindow<SceneEditorWindow>()->getCurrentScene();
    auto& systemsMap  = currentScene->getSystemRunnerRef()->getSystemsRef();

    ImVec4 winSelectColor = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Header, winSelectColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, winSelectColor);

    if (searchBuff_.empty()) {
        // カテゴリごとに見出しをつけて表示
        for (size_t i = 0; i < systemsMap.size(); ++i) {
            auto& systemsByCategory = systemsMap[i];
            if (systemsByCategory.empty()) {
                continue;
            }
            const std::string& categoryName = SystemCategoryString[i];
            if (ImGui::CollapsingHeader(categoryName.c_str())) {
                ImGui::Indent();
                for (auto& [name, system] : systemsByCategory) {
                    bool isSelected = std::find(parentArea_->systemTypeNames_.begin(), parentArea_->systemTypeNames_.end(), name) != parentArea_->systemTypeNames_.end();
                    if (ImGui::Selectable(name.c_str(), isSelected)) {
                        if (!isSelected) {
                            if (!ImGui::GetIO().KeyShift) {
                                auto clearCommand = std::make_unique<ClearSystemCategoryNames>(parentArea_);
                                EditorController::getInstance()->pushCommand(std::move(clearCommand));
                            }
                            auto command = std::make_unique<AddSystemCategoryNames>(parentArea_, name);
                            EditorController::getInstance()->pushCommand(std::move(command));
                        } else {
                            auto command = std::make_unique<RemoveSystemCategoryNames>(parentArea_, name);
                            EditorController::getInstance()->pushCommand(std::move(command));
                        }
                    }
                }
                ImGui::Unindent();
            }
        }
    } else {
        // 検索結果のみカテゴリ見出しなしで表示
        for (size_t i = 0; i < systemsMap.size(); ++i) {
            auto& systemsByCategory = systemsMap[i];
            for (auto& [name, system] : systemsByCategory) {
                if (name.find(searchBuff_) == std::string::npos)
                    continue;
                bool isSelected = std::find(parentArea_->systemTypeNames_.begin(), parentArea_->systemTypeNames_.end(), name) != parentArea_->systemTypeNames_.end();
                if (ImGui::Selectable(name.c_str(), isSelected)) {
                    if (!isSelected) {
                        if (!ImGui::GetIO().KeyShift) {
                            auto clearCommand = std::make_unique<ClearSystemCategoryNames>(parentArea_);
                            EditorController::getInstance()->pushCommand(std::move(clearCommand));
                        }
                        auto command = std::make_unique<AddSystemCategoryNames>(parentArea_, name);
                        EditorController::getInstance()->pushCommand(std::move(command));
                    } else {
                        auto command = std::make_unique<RemoveSystemCategoryNames>(parentArea_, name);
                        EditorController::getInstance()->pushCommand(std::move(command));
                    }
                }
            }
        }
    }
    ImGui::PopStyleColor(3);

    ImGui::EndChild();

    ImGui::Spacing();

    if (parentArea_->systemTypeNames_.empty()) {
        bool selected = false;
        ImGui::Selectable("OK", &selected, ImGuiSelectableFlags_Disabled);
    } else {
        if (ImGui::Button("OK")) {
            auto addSystemCommand = std::make_unique<AddSystemsForTargetEntities>(parentArea_, parentArea_->targetEntityIds_, parentArea_->systemTypeNames_);
            EditorController::getInstance()->pushCommand(std::move(addSystemCommand));
            auto clearEntitiesCommand = std::make_unique<ClearTargetEntities>(parentArea_);
            EditorController::getInstance()->pushCommand(std::move(clearEntitiesCommand));
            auto clearSystemNamesCommand = std::make_unique<ClearSystemCategoryNames>(parentArea_);
            EditorController::getInstance()->pushCommand(std::move(clearSystemNamesCommand));
            parentArea_->isOpen_.set(false);
            parentArea_->isFocused_.set(false);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("CANCEL")) {
        auto clearEntitiesCommand = std::make_unique<ClearTargetEntities>(parentArea_);
        EditorController::getInstance()->pushCommand(std::move(clearEntitiesCommand));
        auto clearSystemNamesCommand = std::make_unique<ClearSystemCategoryNames>(parentArea_);
        EditorController::getInstance()->pushCommand(std::move(clearSystemNamesCommand));
        parentArea_->isOpen_.set(false);
        parentArea_->isFocused_.set(false);
    }
}
void SelectAddSystemArea::SystemListRegion::Finalize() {}

#pragma endregion

EntityInspectorArea::ChangeEditEntityCommand::ChangeEditEntityCommand(EntityInspectorArea* _inspectorArea, int32_t _to, int32_t _from)
    : inspectorArea_(_inspectorArea), toId_(_to), fromId_(_from) {
    SceneSerializer serializer(inspectorArea_->getParentWindow()->getCurrentScene());
    if (toId_ >= 0) {
        serializer.EntityToJson(toId_, toEntityData_);
    }
    if (fromId_ >= 0) {
        serializer.EntityToJson(fromId_, fromEntityData_);
    }
}

void EntityInspectorArea::ChangeEditEntityCommand::Execute() {
    /// 適応
    inspectorArea_->editEntityId_ = toId_;
    inspectorArea_->entityComponentMap_.clear();
    inspectorArea_->systemMap_.fill({});

    if (toId_ < 0) {
        return;
    }
    Scene* currentScene             = inspectorArea_->getParentWindow()->getCurrentScene();
    GameEntity* toEntity            = currentScene->getEntityRepositoryRef()->getEntity(toId_);
    inspectorArea_->editEntityName_ = toEntity->getDataType();

    if (toEntityData_.empty()) {
        LOG_DEBUG("ChangeEditEntityCommand::Execute: toEntityData is empty, skipping entity change.");
        return;
    }

    /// 必要なら 読み込み (InspectorArea の entityComponentMap と systemMapに対して)
    auto& componentMap = currentScene->getComponentRepositoryRef()->getComponentArrayMap();

    /// Components の読み込み
    auto& entityCompData = toEntityData_.at("Components");
    for (auto& compData : entityCompData.items()) {
        const std::string& compTypeName = compData.key();
        auto compArray                  = componentMap.find(compTypeName);
        if (compArray == componentMap.end()) {
            LOG_ERROR("ChangeEditEntityCommand::Execute: Component type '{}' not found in component map.", compTypeName);
            continue;
        }
        IComponentArray* compArrayPtr = compArray->second.get();
        for (uint32_t i = 0; i < static_cast<uint32_t>((compArrayPtr->getComponentSize(toEntity))); ++i) {
            auto component = compArrayPtr->getComponent(toEntity, i);
            if (!component) {
                LOG_ERROR("ChangeEditEntityCommand::Execute: Component '{}' not found for entity ID '{}'.", compTypeName, toId_);
                continue;
            }
            inspectorArea_->entityComponentMap_[compTypeName].emplace_back(component);
        }
    }

    /// Systems の読み込み
    auto& entitySystemData = toEntityData_.at("Systems");
    auto& systemMap        = currentScene->getSystemRunnerRef()->getSystems();
    for (auto& systemData : entitySystemData.items()) {
        const std::string& systemName = systemData.value().at("SystemName");
        int32_t category              = systemData.value().at("SystemCategory").get<int32_t>();
        auto systemItr                = systemMap[category].find(systemName);
        if (systemItr == systemMap[category].end()) {
            LOG_ERROR("ChangeEditEntityCommand::Execute: System '{}' not found in category '{}'.", systemName, SystemCategoryString[category]);
            continue;
        }
        ISystem* system = systemItr->second;
        if (!system) {
            LOG_ERROR("ChangeEditEntityCommand::Execute: System '{}' not found for entity ID '{}'.", systemName, toId_);
            continue;
        }
        inspectorArea_->systemMap_[category][systemName] = system;
    }
}

void EntityInspectorArea::ChangeEditEntityCommand::Undo() {
    inspectorArea_->editEntityId_ = fromId_;
    inspectorArea_->entityComponentMap_.clear();
    inspectorArea_->systemMap_.fill({});

    if (fromId_ < 0) {
        return;
    }

    Scene* currentScene             = inspectorArea_->getParentWindow()->getCurrentScene();
    GameEntity* frontEntity         = currentScene->getEntityRepositoryRef()->getEntity(fromId_);
    inspectorArea_->editEntityName_ = frontEntity->getDataType();

    if (fromEntityData_.empty()) {
        LOG_DEBUG("ChangeEditEntityCommand::Undo: fromEntityData is empty, skipping entity change.");
        return;
    }

    /// 必要なら 読み込み (InspectorArea の entityComponentMap と systemMapに対して)
    auto& componentMap = currentScene->getComponentRepositoryRef()->getComponentArrayMap();

    /// Components の読み込み
    auto& entityCompData = fromEntityData_.at("Components");
    for (auto& compData : entityCompData.items()) {
        const std::string& compTypeName = compData.key();
        auto compArray                  = componentMap.find(compTypeName);
        if (compArray == componentMap.end()) {
            LOG_ERROR("ChangeEditEntityCommand::Execute: Component type '{}' not found in component map.", compTypeName);
            continue;
        }
        IComponentArray* compArrayPtr = compArray->second.get();
        for (uint32_t i = 0; i < static_cast<uint32_t>(compArrayPtr->getComponentSize(frontEntity)); ++i) {
            auto component = compArrayPtr->getComponent(frontEntity, i);
            if (!component) {
                LOG_ERROR("ChangeEditEntityCommand::Execute: Component '{}' not found for entity ID '{}'.", compTypeName, toId_);
                continue;
            }
            inspectorArea_->entityComponentMap_[compTypeName].emplace_back(component);
        }
    }

    /// Systems の読み込み
    auto& entitySystemData = fromEntityData_.at("Systems");
    auto& systemMap        = currentScene->getSystemRunnerRef()->getSystems();
    for (auto& systemData : entitySystemData.items()) {
        const std::string& systemName = systemData.value().at("SystemName");
        int32_t category              = systemData.value().at("SystemCategory").get<int32_t>();
        auto systemItr                = systemMap[category].find(systemName);
        if (systemItr == systemMap[category].end()) {
            LOG_ERROR("ChangeEditEntityCommand::Execute: System '{}' not found in category '{}'.", systemName, SystemCategoryString[category]);
            continue;
        }
        ISystem* system = systemItr->second;
        if (!system) {
            LOG_ERROR("ChangeEditEntityCommand::Execute: System '{}' not found for entity ID '{}'.", systemName, toId_);
            continue;
        }
        inspectorArea_->systemMap_[category][systemName] = system;
    }
}

void EntityInfomationRegion::ChangeEntityUniqueness::Execute() {
    auto currentScene  = inspectorArea_->getParentWindow()->getCurrentScene();
    GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(entityId_);

    if (!entity) {
        LOG_ERROR("ChangeEntityUniqueness::Execute: Entity with ID '{}' not found.", entityId_);
        return;
    }

    if (newValue_) {
        currentScene->getEntityRepositoryRef()->registerUniqueEntity(entity);
    } else {
        currentScene->getEntityRepositoryRef()->unregisterUniqueEntity(entity->getDataType());
    }
}

void EntityInfomationRegion::ChangeEntityUniqueness::Undo() {
    auto currentScene  = inspectorArea_->getParentWindow()->getCurrentScene();
    GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(entityId_);

    if (!entity) {
        LOG_ERROR("ChangeEntityUniqueness::Execute: Entity with ID '{}' not found.", entityId_);
        return;
    }

    if (oldValue_) {
        currentScene->getEntityRepositoryRef()->registerUniqueEntity(entity);
    } else {
        currentScene->getEntityRepositoryRef()->unregisterUniqueEntity(entity->getDataType());
    }
}

void EntityInfomationRegion::ChangeEntityName::Execute() {
    auto currentScene  = inspectorArea_->getParentWindow()->getCurrentScene();
    GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(entityId_);
    if (!entity) {
        LOG_ERROR("ChangeEntityName::Execute: Entity with ID '{}' not found.", entityId_);
        return;
    }
    entity->setDataType(newName_);
    // UniqueEntity の場合は、名前変更後に再登録
    if (entity->isUnique()) {
        currentScene->getEntityRepositoryRef()->unregisterUniqueEntity(oldName_);
        currentScene->getEntityRepositoryRef()->registerUniqueEntity(entity);
    }
}

void EntityInfomationRegion::ChangeEntityName::Undo() {
    auto currentScene  = inspectorArea_->getParentWindow()->getCurrentScene();
    GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(entityId_);
    if (!entity) {
        LOG_ERROR("ChangeEntityName::Undo: Entity with ID '{}' not found.", entityId_);
        return;
    }
    entity->setDataType(oldName_);
    // UniqueEntity の場合は、名前変更後に再登録
    if (entity->isUnique()) {
        currentScene->getEntityRepositoryRef()->unregisterUniqueEntity(newName_);
        currentScene->getEntityRepositoryRef()->registerUniqueEntity(entity);
    }
}

void SelectAddComponentArea::AddComponentTypeNames::Execute() {
    parentArea_->componentTypeNames_.push_back(componentTypeName_);
}

void SelectAddComponentArea::AddComponentTypeNames::Undo() {
    auto& componentTypeNames = parentArea_->componentTypeNames_;
    auto it                  = std::remove(componentTypeNames.begin(), componentTypeNames.end(), componentTypeName_);
    if (it != componentTypeNames.end()) {
        componentTypeNames.erase(it, componentTypeNames.end());
    }
}

void SelectAddComponentArea::SetTargeEntities::Execute() {
    parentArea_->targetEntityIds_ = targetEntityIds_; // 新しいターゲットエンティティIDを設定
}

void SelectAddComponentArea::SetTargeEntities::Undo() {
    parentArea_->targetEntityIds_ = previousTargetEntityIds_; // 保存したターゲットエンティティIDを復元
}

void SelectAddComponentArea::RemoveComponentTypeNames::Execute() {
    auto& componentTypeNames = parentArea_->componentTypeNames_;
    auto it                  = std::remove(componentTypeNames.begin(), componentTypeNames.end(), componentTypeName_);
    if (it != componentTypeNames.end()) {
        componentTypeNames.erase(it, componentTypeNames.end());
    }
}

void SelectAddComponentArea::RemoveComponentTypeNames::Undo() {
    parentArea_->componentTypeNames_.push_back(componentTypeName_);
}

void SelectAddComponentArea::ClearTargetEntities::Execute() {
    parentArea_->targetEntityIds_.clear(); // ターゲットエンティティIDをクリア
}

void SelectAddComponentArea::ClearTargetEntities::Undo() {
    parentArea_->targetEntityIds_ = previousTargetEntityIds_; // 保存したターゲットエンティティIDを復元
}

void SelectAddComponentArea::ClearComponentTypeNames::Execute() {
    componentTypeNames_ = parentArea_->componentTypeNames_; // 現在のコンポーネントタイプ名を保存
    parentArea_->componentTypeNames_.clear(); // コンポーネントタイプ名をクリア

    LOG_DEBUG("SelectAddComponentArea::ClearComponentTypeNames::Execute: Cleared component type names.");
}

void SelectAddComponentArea::ClearComponentTypeNames::Undo() {
    parentArea_->componentTypeNames_ = componentTypeNames_; // 保存したコンポーネントタイプ名を復元
    LOG_DEBUG("SelectAddComponentArea::ClearComponentTypeNames::Undo: Restored component type names.");
}

void SelectAddSystemArea::AddSystemCategoryNames::Execute() {
    parentArea_->systemTypeNames_.push_back(systemTypeName_);
    LOG_DEBUG("SelectAddSystemArea::AddSystemCategoryNames::Execute: Added system type name '{}'.", systemTypeName_);
}

void SelectAddSystemArea::AddSystemCategoryNames::Undo() {
    std::string popName = parentArea_->systemTypeNames_.back();
    parentArea_->systemTypeNames_.pop_back();
    LOG_DEBUG("SelectAddSystemArea::AddSystemCategoryNames::Undo: Removed system type name '{}'.", popName);
}

void SelectAddSystemArea::RemoveSystemCategoryNames::Execute() {
    auto& systemTypeNames = parentArea_->systemTypeNames_;
    auto it               = std::remove(systemTypeNames.begin(), systemTypeNames.end(), systemTypeName_);
    if (it != systemTypeNames.end()) {
        systemTypeNames.erase(it, systemTypeNames.end());
        LOG_DEBUG("SelectAddSystemArea::RemoveSystemCategoryNames::Execute: Removed system type name '{}'.", systemTypeName_);
    } else {
        LOG_DEBUG("SelectAddSystemArea::RemoveSystemCategoryNames::Execute: System type name '{}' not found.", systemTypeName_);
    }
}

void SelectAddSystemArea::RemoveSystemCategoryNames::Undo() {
    parentArea_->systemTypeNames_.push_back(systemTypeName_);
    LOG_DEBUG("SelectAddSystemArea::RemoveSystemCategoryNames::Undo: Added system type name '{}' back.", systemTypeName_);
}

void SelectAddSystemArea::ClearSystemCategoryNames::Execute() {
    systemTypeNames_ = parentArea_->systemTypeNames_; // 現在のシステムタイプ名を保存
    parentArea_->systemTypeNames_.clear(); // システムタイプ名をクリア
    LOG_DEBUG("SelectAddSystemArea::ClearSystemCategoryNames::Execute: Cleared system type names.");
}

void SelectAddSystemArea::ClearSystemCategoryNames::Undo() {
    parentArea_->systemTypeNames_ = systemTypeNames_; // 保存したシステムタイプ名を復元
    LOG_DEBUG("SelectAddSystemArea::ClearSystemCategoryNames::Undo: Restored system type names.");
}

void SelectAddSystemArea::SetTargeEntities::Execute() {
    parentArea_->targetEntityIds_ = targetEntityIds_; // 新しいターゲットエンティティIDを設定
}

void SelectAddSystemArea::SetTargeEntities::Undo() {
    parentArea_->targetEntityIds_ = previousTargetEntityIds_; // 保存したターゲットエンティティIDを復元
}

void SelectAddSystemArea::ClearTargetEntities::Execute() {
    parentArea_->targetEntityIds_.clear(); // ターゲットエンティティIDをクリア
}

void SelectAddSystemArea::ClearTargetEntities::Undo() {
    parentArea_->targetEntityIds_ = previousTargetEntityIds_; // 保存したターゲットエンティティIDを復元
}

void SelectAddSystemArea::AddSystemsForTargetEntities::Execute() {
    auto window = parentArea_->parentWindow_;
    if (!window) {
        LOG_ERROR("AddSystemsForTargetEntities::Undo: No SceneEditorWindow found.");
        return;
    }
    auto currentScene = window->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("AddSystemsForTargetEntities::Execute: No current scene found.");
        return;
    }
    auto* entityInspectorArea = dynamic_cast<EntityInspectorArea*>(window->getArea(nameof<EntityInspectorArea>()).get()); // 選択状態をクリア
    if (!entityInspectorArea) {
        LOG_ERROR("entityInspectorArea not found in SceneEditorWindow.");
        return;
    }
    int32_t editEntityId = entityInspectorArea->getEditEntityId();
    for (const auto& entityId : parentArea_->targetEntityIds_) {
        GameEntity* entity = currentScene->getEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with ID '{}' not found.", entityId);
            continue;
        }

        if (editEntityId == entityId) {
            for (const auto& systemTypeName : parentArea_->systemTypeNames_) {
                currentScene->getSystemRunnerRef()->registerEntity(systemTypeName, entity);

                ISystem* system                                                                     = currentScene->getSystemRunnerRef()->getSystem(systemTypeName);
                entityInspectorArea->getSystemMap()[int32_t(system->getCategory())][systemTypeName] = system;
            }
        } else {
            for (const auto& systemTypeName : parentArea_->systemTypeNames_) {
                currentScene->getSystemRunnerRef()->registerEntity(systemTypeName, entity);
            }
        }
    }
}

void SelectAddSystemArea::AddSystemsForTargetEntities::Undo() {
    auto window = parentArea_->parentWindow_;
    if (!window) {
        LOG_ERROR("AddSystemsForTargetEntities::Undo: No SceneEditorWindow found.");
        return;
    }
    auto currentScene = window->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("AddSystemsForTargetEntities::Execute: No current scene found.");
        return;
    }
    auto* entityInspectorArea = dynamic_cast<EntityInspectorArea*>(window->getArea(nameof<EntityInspectorArea>()).get()); // 選択状態をクリア
    if (!entityInspectorArea) {
        LOG_ERROR("entityInspectorArea not found in SceneEditorWindow.");
        return;
    }
    int32_t editEntityId = entityInspectorArea->getEditEntityId();
    for (const auto& entityId : parentArea_->targetEntityIds_) {
        GameEntity* entity = currentScene->getEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with ID '{}' not found.", entityId);
            continue;
        }

        if (editEntityId == entityId) {
            for (const auto& systemTypeName : parentArea_->systemTypeNames_) {
                currentScene->getSystemRunnerRef()->removeEntity(systemTypeName, entity);
                ISystem* system = currentScene->getSystemRunnerRef()->getSystem(systemTypeName);
                auto& systems   = entityInspectorArea->getSystemMap()[int32_t(system->getCategory())];
                auto itr        = systems.find(systemTypeName);
                if (itr != systems.end()) {
                    systems.erase(itr);
                } else {
                    LOG_ERROR("System '{}' not found in EntityInspectorArea's system map.", systemTypeName);
                }
            }
        } else {
            for (const auto& systemTypeName : parentArea_->systemTypeNames_) {
                currentScene->getSystemRunnerRef()->removeEntity(systemTypeName, entity);
            }
        }
    }
}

EntityInfomationRegion::DeleteEntityCommand::DeleteEntityCommand(EntityInspectorArea* _parentArea, int32_t _entityId)
    : parentArea_(_parentArea), entityId_(_entityId) {}

void EntityInfomationRegion::DeleteEntityCommand::Execute() {
    auto currentScene = parentArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("DeleteEntityCommand::Execute: No current scene found.");
        return;
    }
    GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(entityId_);
    if (!entity) {
        LOG_ERROR("DeleteEntityCommand::Execute: Entity with ID '{}' not found.", entityId_);
        return;
    }
    SceneSerializer serializer(currentScene);
    serializer.EntityToJson(entityId_, entityData_);
    currentScene->getEntityRepositoryRef()->removeEntity(entityId_);
    LOG_DEBUG("DeleteEntityCommand::Execute: Deleted entity with ID '{}'.", entityId_);
}

void EntityInfomationRegion::DeleteEntityCommand::Undo() {
    auto currentScene = parentArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("DeleteEntityCommand::Undo: No current scene found.");
        return;
    }

    SceneSerializer serializer(currentScene);
    GameEntity* entity = serializer.EntityFromJson(entityId_, entityData_);
    if (!entity) {
        LOG_ERROR("DeleteEntityCommand::Undo: Failed to restore entity with ID '{}'.", entityId_);
        return;
    }
    LOG_DEBUG("DeleteEntityCommand::Undo: Restored entity with ID '{}'.", entityId_);
}

#endif // _DEBUG
