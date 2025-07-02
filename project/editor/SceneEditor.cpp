#include "SceneEditor.h"

/// engine
#include "Engine.h"
#include "scene/SceneManager.h"
#include "winApp/WinApp.h"

// ecs
#include "component/ComponentArray.h"
#include "component/IComponent.h"
#include "ECS/Entity.h"
#include "system/ISystem.h"

// directX12
#include "directX12/RenderTexture.h"
// camera
#include "camera/CameraManager.h"
#include "camera/debugCamera/DebugCamera.h"

/// lib
#include "myFileSystem/MyFileSystem.h"
#include <myGui/MyGui.h>

void SceneEditorWindow::Initialize() {
    currentScene_ = std::make_unique<Scene>(editSceneName_);
    currentScene_->Initialize();

    // メニューの初期化
    auto fileMenu = std::make_unique<FileMenu>(this);
    addMenu(std::move(fileMenu));

    // エリアの初期化
    addArea(std::make_unique<SceneViewArea>(this));
    addArea(std::make_unique<HierarchyArea>(this));
    addArea(std::make_unique<EntityInspectorArea>(this));
    addArea(std::make_unique<SystemInspectorArea>(this));
    addArea(std::make_unique<SelectAddComponentArea>(this));
}

void SceneEditorWindow::Finalize() {
    // エリアの終了処理
    for (auto& [name, area] : areas_) {
        area->Finalize();
    }
    areas_.clear();
    // メニューの終了処理
    for (auto& [name, menu] : menus_) {
        menu->Finalize();
    }
    menus_.clear();
}

#pragma region "Menus"
FileMenu::FileMenu(SceneEditorWindow* _parentWindow)
    : parentWindow_(_parentWindow), Editor::Menu("File") {}
FileMenu::~FileMenu() {}
void FileMenu::Initialize() {
    addMenuItem(std::make_unique<SaveMenuItem>(this));
    addMenuItem(std::make_unique<LoadMenuItem>(this));
}
void FileMenu::Finalize() {
    for (auto& [name, item] : menuItems_) {
        item->Finalize();
    }
    menuItems_.clear();
}

SaveMenuItem::SaveMenuItem(FileMenu* _parent)
    : Editor::MenuItem("Save"), parentMenu_(_parent) {}
SaveMenuItem::~SaveMenuItem() {}

void SaveMenuItem::Initialize() {
    saveScene_ = parentMenu_->getParentWindow()->getCurrentScene();
    if (!saveScene_) {
        LOG_ERROR("SaveMenuItem: No current scene to save.");
        return;
    }
}

void SaveMenuItem::DrawGui() {
    bool isSelect = isSelected_.current();
    bool isEnable = isEnable_.current();

    if (ImGui::MenuItem(name_.c_str(), "ctl + s", &isSelect, &isEnable)) {
        SceneSerializer serializer = SceneSerializer(saveScene_);
        LOG_DEBUG("SaveMenuItem : Saving scene '{}'.", saveScene_->getName());
        serializer.Serialize();
    }

    isSelected_.set(isSelect);
    isEnable_.set(isEnable);
}

void SaveMenuItem::Finalize() {
    saveScene_ = nullptr; // 保存するシーンへのポインタをクリア
}

LoadMenuItem::LoadMenuItem(FileMenu* _parent)
    : Editor::MenuItem("Load"), parentMenu_(_parent) {}

LoadMenuItem::~LoadMenuItem() {}

void LoadMenuItem::Initialize() {
    loadScene_ = parentMenu_->getParentWindow()->getCurrentScene();
    if (!loadScene_) {
        LOG_ERROR("LoadMenuItem: No current scene to load.");
        return;
    }
}

void LoadMenuItem::DrawGui() {
    bool isSelect = isSelected_.current();
    bool isEnable = isEnable_.current();
    if (ImGui::MenuItem(name_.c_str(), "ctl + o", &isSelect, &isEnable)) {
        // シーンのロード処理

        loadScene_->Finalize();

        SceneSerializer serializer = SceneSerializer(loadScene_);
        serializer.Deserialize();

        LOG_DEBUG("LoadMenuItem : Loading scene '{}'.", loadScene_->getName());
    }
    isSelected_.set(isSelect);
    isEnable_.set(isEnable);
}

void LoadMenuItem::Finalize() {
    loadScene_ = nullptr; // ロードするシーンへのポインタをクリア
}

CreateMenuItem::CreateMenuItem(FileMenu* _parent)
    : Editor::MenuItem(nameof<CreateMenuItem>()), parentMenu_(_parent) {}

CreateMenuItem::~CreateMenuItem() {}

void CreateMenuItem::Initialize() {}

void CreateMenuItem::DrawGui() {
    bool isSelect = isSelected_.current();
    bool isEnable = isEnable_.current();
    if (ImGui::MenuItem(name_.c_str(), nullptr, &isSelect, &isEnable)) {
        ImGui::InputText("New Scene Name", &newSceneName_[0], sizeof(char) * 256);
        if (ImGui::Button("Create")) {
            auto scene = parentMenu_->getParentWindow()->getCurrentScene();

            SceneSerializer serializer = SceneSerializer(scene);
            serializer.Serialize();

            scene->Finalize();

            auto newScene = std::make_unique<Scene>(newSceneName_);
            newScene->Initialize();
            parentMenu_->getParentWindow()->changeScene(std::move(newScene));

            // 初期化
            EditorController::getInstance()->clearCommandHistory();
            name_ = "";
        }
    } else {
        name_ = "";
    }
    isSelected_.set(isSelect);
    isEnable_.set(isEnable);
}

void CreateMenuItem::Finalize() {
}

#pragma endregion

#pragma region "SceneViewArea"

SceneViewArea::SceneViewArea(SceneEditorWindow* _parentWindow)
    : parentWindow_(_parentWindow), Editor::Area("SceneView") {}

void SceneViewArea::Initialize() {
    // DebugCameraの初期化
    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Initialize();
}

void SceneViewArea::DrawGui() {
    bool isOpen = isOpen_.current();
    if (ImGui::Begin(name_.c_str(), &isOpen)) {
        areaSize_ = ImGui::GetContentRegionAvail();

        auto renderTexture = parentWindow_->getCurrentScene()->getSceneView();
        if (areaSize_[X] >= 1.f && areaSize_[Y] >= 1.f && renderTexture->getTextureSize() != areaSize_) {
            renderTexture->Resize(areaSize_);

            float aspectRatio                                 = areaSize_[X] / areaSize_[Y];
            debugCamera_->getCameraTransformRef().aspectRatio = aspectRatio;
        }

        if (isFocused_.current()) {
            debugCamera_->Update();
        }

        DrawScene();

        ImGui::Image(reinterpret_cast<ImTextureID>(renderTexture->getBackBufferSrvHandle().ptr), areaSize_.toImVec2());

        for (auto& [name, region] : regions_) {
            if (!region) {
                continue;
            }
            ImGui::BeginGroup();
            region->DrawGui();
            ImGui::EndGroup();
        }
    }

    isOpen_.set(isOpen);
    isFocused_.set(ImGui::IsWindowFocused());
    UpdateFocusAndOpenState();

    ImGui::End();
}
void SceneViewArea::DrawScene() {
    CameraManager* cameraManager  = CameraManager::getInstance();
    CameraTransform prevTransform = cameraManager->getTransform();

    auto* currentScene = parentWindow_->getCurrentScene();
    cameraManager->setTransform(debugCamera_->getCameraTransform());
    currentScene->Render();
    cameraManager->setTransform(prevTransform);
}

void SceneViewArea::Finalize() {
    if (debugCamera_) {
        debugCamera_.reset();
    }
}

#pragma endregion

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

EntityComponentRegion::EntityComponentRegion(EntityInspectorArea* _parent)
    : Editor::Region(nameof<EntityComponentRegion>()), parentArea_(_parent) {}
EntityComponentRegion::~EntityComponentRegion() {}
void EntityComponentRegion::Initialize() {}
void EntityComponentRegion::DrawGui() {
    auto currentScene    = parentArea_->getParentWindow()->getCurrentScene();
    int32_t editEntityId = parentArea_->getEditEntityId();
    auto editEntity      = currentScene->getEntityRepositoryRef()->getEntity(editEntityId);

    ImGui::Text("Entity Components");
    if (!editEntity) {
        ImGui::Text("No entity selected.");
        return;
    }

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

    ImGui::Separator();
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

    ImGui::Text("Entity Systems");

    if (!editEntity) {
        ImGui::Text("No entity selected.");
        return;
    }

    if (ImGui::Button("+ System")) {
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

    if (parentArea_->componentTypeNames_.empty()) {
        bool selected = false;
        ImGui::Selectable("OK", &selected, ImGuiSelectableFlags_Disabled);
    } else {
        if (ImGui::Button("OK")) {
            // OKボタンが押された場合、選択されたコンポーネントを追加
            auto addCompCommand = std::make_unique<AddComponentsForTargetEntities>(parentArea_, parentArea_->targetEntityIds_, parentArea_->componentTypeNames_);
            EditorController::getInstance()->pushCommand(std::move(addCompCommand));

            auto clearEntitiesCommand = std::make_unique<ClearTargetEntities>(parentArea_);
            EditorController::getInstance()->pushCommand(std::move(clearEntitiesCommand));

            auto clearComponentNamesCommand = std::make_unique<ClearComponentTypeNames>(parentArea_);
            EditorController::getInstance()->pushCommand(std::move(clearComponentNamesCommand));

            parentArea_->isOpen_.set(false); // エリアを閉じる
            parentArea_->isFocused_.set(false); // フォーカスを外す
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("cancel")) {
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

#pragma endregion

#pragma region "SystemInspectorArea"

SystemInspectorArea::SystemInspectorArea(SceneEditorWindow* _window) : Editor::Area("SystemInspectorArea"), parentWindow_(_window) {}
SystemInspectorArea::~SystemInspectorArea() {}

void SystemInspectorArea::Initialize() {
    auto& allSystems = SystemRegistry::getInstance()->getSystemsRef();
    for (auto& [name, system] : allSystems) {
        if (!system) {
            LOG_ERROR("SystemInspectorArea::Initialize: System '{}' is null.", name);
            continue;
        }
        int32_t category           = int32_t(system->getCategory());
        systemMap_[category][name] = system.get();
    }
}

void SystemInspectorArea::DrawGui() {
    bool isOpen = isOpen_.current();
    if (ImGui::Begin(name_.c_str(), &isOpen)) {
        areaSize_ = ImGui::GetContentRegionAvail();
        ImGui::Text("System Inspector Area");
        ImGui::Separator();
        auto currentScene = parentWindow_->getCurrentScene();

        if (!currentScene) {
            ImGui::Text("No current scene found.");
            return;
        }

        std::string label = "";
        label             = "Filter##" + currentScene->getName();

        int32_t filter                        = static_cast<int32_t>(filter_);
        constexpr int32_t categoryFilterStart = 2;
        if (ImGui::TreeNode(label.c_str())) {
            constexpr int32_t categoryMask = ~((1 << categoryFilterStart) - 1); // 0b11111100...
            // 下位2ビットだけを操作するための一時変数
            int32_t baseFilter     = filter & ((1 << categoryFilterStart) - 1); // 0,1,2
            int32_t prevBaseFilter = baseFilter;

            // ラジオボタン
            if (ImGui::RadioButton("All", &baseFilter, 0)) {}
            ImGui::SameLine();
            if (ImGui::RadioButton("Active", &baseFilter, 1)) {}
            ImGui::SameLine();
            if (ImGui::RadioButton("Inactive", &baseFilter, 2)) {}

            // baseFilterが変化したら、カテゴリビットは維持したまま下位2ビットだけを更新
            if (baseFilter != prevBaseFilter) {
                filter = (filter & categoryMask) | baseFilter;
            }

            // カテゴリチェックボックス
            for (size_t i = 0; i < static_cast<size_t>(SystemCategory::Count); ++i) {
                label             = SystemCategoryString[i];
                int32_t filterBit = (1 << (i + categoryFilterStart));
                bool isChecked    = (filter & filterBit) != 0;
                if (ImGui::Checkbox(label.c_str(), &isChecked)) {
                    if (isChecked) {
                        filter |= filterBit;
                    } else {
                        filter &= ~filterBit;
                    }
                }
            }
            ImGui::TreePop();
        }

        {
            label = "SearchSystems##SystemInspectorArea";
            ImGui::InputText(label.c_str(), &searchBuffer_[0], sizeof(char) * searchBufferSize_);
            static GuiValuePool<std::string> entityNamePool;
            if (ImGui::IsItemActive()) {
                entityNamePool.setValue(label, searchBuffer_);
            } else if (ImGui::IsItemDeactivatedAfterEdit()) {
                auto command = std::make_unique<ChangeSearchFilter>(this, entityNamePool.popValue(label));
                EditorController::getInstance()->pushCommand(std::move(command));
            }
        }

        filter = searchBuffer_.empty() ? filter : static_cast<int32_t>(FilterType::SEARCH);

        if (filter_ != filter) {
            filter_      = static_cast<FilterType>(filter);
            auto command = std::make_unique<ChangeSystemFilter>(this, static_cast<int32_t>(filter_));
            EditorController::getInstance()->pushCommand(std::move(command));
        }

        bool activeFilter   = (filter_ & FilterType::ACTIVE) != 0;
        bool inactiveFilter = (filter_ & FilterType::INACTIVE) != 0;
        bool searchFilter   = (filter_ & FilterType::SEARCH) != 0;
        for (size_t i = 0; i < systemMap_.size(); ++i) {
            if (searchFilter) {
                for (const auto& [systemName, system] : systemMap_[i]) {
                    if (!system) {
                        LOG_ERROR("SystemInspectorArea::DrawGui: System '{}' is null.", systemName);
                        continue;
                    }
                    if (systemName.find(searchBuffer_) != std::string::npos) {
                        if (activeFilter && !system->isActive()) {
                            continue; // アクティブフィルタに合致しない場合はスキップ
                        } else if (inactiveFilter && system->isActive()) {
                            continue; // 非アクティブフィルタに合致しない場合はスキップ
                        }
                        SystemGui(systemName, system);
                    }
                }

            } else {
                int32_t filterBit = (1 << (i + categoryFilterStart));
                if ((filter_ & filterBit) == 0) {
                    continue; // フィルタリングされている場合はスキップ
                }
                if (ImGui::CollapsingHeader(SystemCategoryString[i].c_str())) {

                    ImGui::Indent();
                    for (auto& [systemName, system] : systemMap_[i]) {
                        if (!system) {
                            LOG_ERROR("SystemInspectorArea::DrawGui: System '{}' is null.", systemName);
                            continue;
                        }
                        if (activeFilter) {
                            if (!system->isActive()) {
                                continue; // アクティブフィルタに合致しない場合はスキップ
                            }
                        } else if (inactiveFilter) {
                            if (system->isActive()) {
                                continue; // 非アクティブフィルタに合致しない場合はスキップ
                            }
                        }
                        SystemGui(systemName, system);
                    }
                    ImGui::Unindent();
                }
            }
        }
    }

    isOpen_.set(isOpen);
    isFocused_.set(ImGui::IsWindowFocused());
    UpdateFocusAndOpenState();

    ImGui::End();
}

void SystemInspectorArea::Finalize() {
    Editor::Area::Finalize();
}

void SystemInspectorArea::SystemGui(const std::string& _systemName, ISystem* _system) {
    ImGui::PushID(_systemName.c_str());
    bool isActive = _system->isActive();

    if (ImGui::Checkbox("Active", &isActive)) {
        auto command = std::make_unique<ChangeSystemActivity>(this, _systemName, _system->isActive(), isActive);
        EditorController::getInstance()->pushCommand(std::move(command));
    }

    ImGui::SameLine();

    constexpr int32_t inputPriorityBoxWidth = 76;
    ImGui::SetNextItemWidth(inputPriorityBoxWidth);
    int32_t priority = _system->getPriority();
    if (ImGui::InputInt("Priority", &priority, 1)) {
        auto command = std::make_unique<ChangeSystemPriority>(this, _systemName, _system->getPriority(), priority);
        EditorController::getInstance()->pushCommand(std::move(command));
    }

    ImGui::SameLine();

    ImGui::Text("%s", _systemName.c_str());

    ImGui::PopID();
}

#pragma endregion

#pragma region "HierarchyArea"

HierarchyArea::HierarchyArea(SceneEditorWindow* _window) : Editor::Area(nameof<HierarchyArea>()), parentWindow_(_window) {}

HierarchyArea::~HierarchyArea() {}

void HierarchyArea::Initialize() {
    addRegion(std::make_shared<EntityHierarchy>(this));
}

void HierarchyArea::Finalize() {
    Editor::Area::Finalize();
}

EntityHierarchy::EntityHierarchy(HierarchyArea* _parent) : Editor::Region(nameof<EntityHierarchy>()), parentArea_(_parent) {}
EntityHierarchy::~EntityHierarchy() {}

void EntityHierarchy::Initialize() {}

void EntityHierarchy::DrawGui() {
    ImGui::Text("Entity Hierarchy");
    auto currentScene = parentArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        ImGui::Text("No current scene found.");
        return;
    }
    auto& entityRepository = currentScene->getEntityRepositoryRef()->getEntities();
    if (entityRepository.empty()) {
        ImGui::Text("No entities in the current scene.");
        return;
    }

    // Entity の作成,削除
    if (ImGui::Button("+ Entity")) {
        // 新しいエンティティを作成
        //! TODO : 初期Entity名を設定できるように
        auto command = std::make_unique<CreateEntityCommand>(parentArea_, "Entity");
        EditorController::getInstance()->pushCommand(std::move(command));
    }

    // ImGuiのスタイルで選択色を設定（必要に応じてアプリ全体で設定してもOK）
    ImVec4 winSelectColor = ImVec4(0.26f, 0.59f, 0.98f, 1.0f); // Windows風の青
    ImGui::PushStyleColor(ImGuiCol_Header, winSelectColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, winSelectColor);

    // 選択状態のエンティティIDを取得
    for (const auto& entity : entityRepository) {
        if (!entity || !entity.isAlive()) {
            continue; // 無効なエンティティはスキップ
        }

        int32_t entityId     = entity.getID();
        std::string uniqueId = entity.getUniqueID();

        // 選択状態か判定
        bool isSelected = std::find(selectedEntityIds_.begin(), selectedEntityIds_.end(), entityId) != selectedEntityIds_.end();

        // Selectableで表示
        if (ImGui::Selectable(uniqueId.c_str(), isSelected)) {
            // Shiftキーで複数選択、そうでなければ単一選択
            if (ImGui::GetIO().KeyShift) {
                if (!isSelected) {
                    // まだ選択されていなければ追加
                    auto command = std::make_unique<AddSelectedEntitiesCommand>(this, entityId);
                    EditorController::getInstance()->pushCommand(std::move(command));
                } else {
                    // すでに選択されていれば解除
                    auto command = std::make_unique<RemoveSelectedEntitiesCommand>(this, entityId);
                    EditorController::getInstance()->pushCommand(std::move(command));
                }
            } else {
                // Shiftキーが押されていない場合は選択をクリアしてから追加
                auto clearCommand = std::make_unique<ClearSelectedEntitiesCommand>(this);
                EditorController::getInstance()->pushCommand(std::move(clearCommand));
                auto addCommand = std::make_unique<AddSelectedEntitiesCommand>(this, entityId);
                EditorController::getInstance()->pushCommand(std::move(addCommand));

                auto& parentWindowHasAreas  = parentArea_->getParentWindow()->getAreas();
                auto entityInspectorAreaItr = parentWindowHasAreas.find("EntityInspectorArea");
                if (entityInspectorAreaItr == parentWindowHasAreas.end()) {
                    LOG_ERROR("EntityInspectorArea not found in parent window.");
                    ImGui::PopStyleColor(3);
                    return;
                }
                auto entityInspectorArea = dynamic_cast<EntityInspectorArea*>(entityInspectorAreaItr->second.get());
                if (!entityInspectorArea) {
                    LOG_ERROR("EntityInspectorArea not found in parent window.");
                    ImGui::PopStyleColor(3);
                    return;
                }
                auto changedEditEntity = std::make_unique<EntityInspectorArea::ChangeEditEntityCommand>(entityInspectorArea, entityId, entityInspectorArea->getEditEntityId());
                EditorController::getInstance()->pushCommand(std::move(changedEditEntity));
            }
        }
    }

    ImGui::PopStyleColor(3);
}

void EntityHierarchy::Finalize() {}

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

void EntityInspectorArea::AddComponentCommand::Execute() {
    auto currentScene  = inspectorArea_->getParentWindow()->getCurrentScene();
    GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(inspectorArea_->editEntityId_);
    if (!entity) {
        LOG_ERROR("AddComponentCommand::Execute: Entity with ID '{}' not found.", inspectorArea_->editEntityId_);
        return;
    }

    // コンポーネントの追加
    IComponentArray* compArray = currentScene->getComponentRepositoryRef()->getComponentArray(componentTypeName_);
    compArray->addComponent(entity);
    if (!compArray) {
        LOG_ERROR("AddComponentCommand::Execute: Failed to add component '{}'. \n ", componentTypeName_);
        return;
    }

    // コンポーネントをマップに追加
    if (entity->getID() == inspectorArea_->editEntityId_) {
        inspectorArea_->entityComponentMap_[componentTypeName_].emplace_back(compArray->getBackComponent(entity));
    }
}

void EntityInspectorArea::AddComponentCommand::Undo() {
    auto currentScene  = inspectorArea_->getParentWindow()->getCurrentScene();
    GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(inspectorArea_->editEntityId_);
    if (!entity) {
        LOG_ERROR("AddComponentCommand::Undo: Entity with ID '{}' not found.", inspectorArea_->editEntityId_);
        return;
    }
    // コンポーネントの削除
    IComponentArray* compArray = currentScene->getComponentRepositoryRef()->getComponentArray(componentTypeName_);
    if (!compArray) {
        LOG_ERROR("AddComponentCommand::Undo: ComponentArray '{}' not found.", componentTypeName_);
        return;
    }
    compArray->removeComponent(entity, compArray->getComponentSize(entity) - 1);

    if (entity->getID() == inspectorArea_->editEntityId_) {
        // コンポーネントをマップから削除
        auto& components = inspectorArea_->entityComponentMap_[componentTypeName_];
        if (!components.empty()) {
            components.pop_back(); // 最後のコンポーネントを削除
        }
    }
}

void EntityInspectorArea::RemoveComponentCommand::Execute() {
    auto currentScene  = inspectorArea_->getParentWindow()->getCurrentScene();
    GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(inspectorArea_->editEntityId_);
    if (!entity) {
        LOG_ERROR("AddComponentCommand::Undo: Entity with ID '{}' not found.", inspectorArea_->editEntityId_);
        return;
    }
    // コンポーネントの削除
    IComponentArray* compArray = currentScene->getComponentRepositoryRef()->getComponentArray(componentTypeName_);
    if (!compArray) {
        LOG_ERROR("AddComponentCommand::Undo: ComponentArray '{}' not found.", componentTypeName_);
        return;
    }
    compArray->removeComponent(entity, componentIndex_);

    if (entity->getID() == inspectorArea_->editEntityId_) {
        // コンポーネントをマップから削除
        auto& components = inspectorArea_->entityComponentMap_[componentTypeName_];
        if (!components.empty()) {
            components.pop_back(); // 最後のコンポーネントを削除
        }
    }
}

void EntityInspectorArea::RemoveComponentCommand::Undo() {
    auto currentScene  = inspectorArea_->getParentWindow()->getCurrentScene();
    GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(inspectorArea_->editEntityId_);
    if (!entity) {
        LOG_ERROR("AddComponentCommand::Execute: Entity with ID '{}' not found.", inspectorArea_->editEntityId_);
        return;
    }

    // コンポーネントの追加
    IComponentArray* compArray = currentScene->getComponentRepositoryRef()->getComponentArray(componentTypeName_);
    compArray->addComponent(entity);
    if (!compArray) {
        LOG_ERROR("AddComponentCommand::Execute: Failed to add component '{}'. \n ", componentTypeName_);
        return;
    }

    // コンポーネントをマップに追加
    if (entity->getID() == inspectorArea_->editEntityId_) {
        inspectorArea_->entityComponentMap_[componentTypeName_].emplace_back(compArray->getBackComponent(entity));
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

EntitySystemRegion::AddSystemCommand::AddSystemCommand(EntityInspectorArea* _inspectorArea, const std::string& _systemTypeName, SystemCategory _category)
    : inspectorArea_(_inspectorArea), systemTypeName_(_systemTypeName), systemCategory_(_category) {}

void EntitySystemRegion::AddSystemCommand::Execute() {
    auto currentScene = inspectorArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("AddSystemCommand::Execute: No current scene found.");
        return;
    }
    auto systemRunner = currentScene->getSystemRunnerRef();

    auto systemItr = systemRunner->getSystemsRef()[int32_t(systemCategory_)].find(systemTypeName_);
    if (systemItr == systemRunner->getSystemsRef()[int32_t(systemCategory_)].end()) {
        LOG_ERROR("AddSystemCommand::Execute: System '{}' not found in category '{}'.", systemTypeName_, SystemCategoryString[int32_t(systemCategory_)]);
        return;
    }

    ISystem* system                                                           = systemItr->second;
    inspectorArea_->getSystemMap()[int32_t(systemCategory_)][systemTypeName_] = system;
}

void EntitySystemRegion::AddSystemCommand::Undo() {
    auto currentScene = inspectorArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("AddSystemCommand::Undo: No current scene found.");
        return;
    }
    auto& systems  = inspectorArea_->getSystemMap()[int32_t(systemCategory_)];
    auto systemItr = systems.find(systemTypeName_);
    if (systemItr == systems.end()) {
        LOG_ERROR("AddSystemCommand::Undo: System '{}' not found in category '{}'.", systemTypeName_, SystemCategoryString[int32_t(systemCategory_)]);
        return;
    }
    systems.erase(systemItr);
}

SystemInspectorArea::ChangeSystemPriority::ChangeSystemPriority(SystemInspectorArea* _inspectorArea, const std::string& _systemName, int32_t _oldPriority, int32_t _newPriority)
    : inspectorArea_(_inspectorArea), systemName_(_systemName), oldPriority_(_oldPriority), newPriority_(_newPriority) {}
void SystemInspectorArea::ChangeSystemPriority::Execute() {
    auto currentScene = inspectorArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("ChangeSystemPriority::Execute: No current scene found.");
        return;
    }
    auto& systems = currentScene->getSystemRunnerRef()->getSystems();
    for (auto& systemCategory : systems) {
        auto systemItr = systemCategory.find(systemName_);
        if (systemItr != systemCategory.end()) {
            oldPriority_ = systemItr->second->getPriority();
            systemItr->second->setPriority(newPriority_);
            LOG_DEBUG("ChangeSystemPriority::Execute: Changed priority of system '{}' to {}.", systemName_, newPriority_);
            return;
        }
    }
    LOG_ERROR("ChangeSystemPriority::Execute: System '{}' not found.", systemName_);
}
void SystemInspectorArea::ChangeSystemPriority::Undo() {
    auto currentScene = inspectorArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("ChangeSystemPriority::Undo: No current scene found.");
        return;
    }
    auto& systems = currentScene->getSystemRunnerRef()->getSystems();
    for (auto& systemCategory : systems) {
        auto systemItr = systemCategory.find(systemName_);
        if (systemItr != systemCategory.end()) {
            systemItr->second->setPriority(oldPriority_);
            LOG_DEBUG("ChangeSystemPriority::Undo: Reverted priority of system '{}' to {}.", systemName_, oldPriority_);
            return;
        }
    }
    LOG_ERROR("ChangeSystemPriority::Undo: System '{}' not found.", systemName_);
}

SystemInspectorArea::ChangeSystemActivity::ChangeSystemActivity(SystemInspectorArea* _inspectorArea, const std::string& _systemName, bool _oldActivity, bool _newActivity)
    : inspectorArea_(_inspectorArea), systemName_(_systemName), oldActivity_(_oldActivity), newActivity_(_newActivity) {}
void SystemInspectorArea::ChangeSystemActivity::Execute() {
    auto currentScene = inspectorArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("ChangeSystemActivity::Execute: No current scene found.");
        return;
    }
    auto& systems = currentScene->getSystemRunnerRef()->getSystems();
    for (auto& systemCategory : systems) {
        auto systemItr = systemCategory.find(systemName_);
        if (systemItr != systemCategory.end()) {
            oldActivity_ = systemItr->second->isActive();
            systemItr->second->setIsActive(newActivity_);
            LOG_DEBUG("ChangeSystemActivity::Execute: Changed activity of system '{}' to {}.", systemName_, newActivity_);
            return;
        }
    }
    LOG_ERROR("ChangeSystemActivity::Execute: System '{}' not found.", systemName_);
}

void SystemInspectorArea::ChangeSystemActivity::Undo() {
    auto currentScene = inspectorArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("ChangeSystemActivity::Undo: No current scene found.");
        return;
    }
    auto& systems = currentScene->getSystemRunnerRef()->getSystems();
    for (auto& systemCategory : systems) {
        auto systemItr = systemCategory.find(systemName_);
        if (systemItr != systemCategory.end()) {
            systemItr->second->setIsActive(oldActivity_);
            LOG_DEBUG("ChangeSystemActivity::Undo: Reverted activity of system '{}' to {}.", systemName_, oldActivity_);
            return;
        }
    }
    LOG_ERROR("ChangeSystemActivity::Undo: System '{}' not found.", systemName_);
}

void SystemInspectorArea::ChangeSystemFilter::Execute() {
    inspectorArea_->filter_ = newFilter_;
    LOG_DEBUG("ChangeSystemFilter::Execute: Changed system filter to {}.", newFilter_);
}

void SystemInspectorArea::ChangeSystemFilter::Undo() {
    inspectorArea_->filter_ = oldFilter_;
    LOG_DEBUG("ChangeSystemFilter::Undo: Reverted system filter to {}.", oldFilter_);
}

void SystemInspectorArea::ChangeSearchFilter::Execute() {
    inspectorArea_->searchBuffer_ = newSearchBuffer_;
    LOG_DEBUG("ChangeSearchFilter::Execute: Changed search filter to '{}'.", newSearchBuffer_);
}

void SystemInspectorArea::ChangeSearchFilter::Undo() {
    inspectorArea_->searchBuffer_ = oldSearchBuffer_;
    LOG_DEBUG("ChangeSearchFilter::Undo: Reverted search filter to '{}'.", oldSearchBuffer_);
}

EntityHierarchy::AddSelectedEntitiesCommand::AddSelectedEntitiesCommand(EntityHierarchy* _hierarchy, int32_t _addedEntityId)
    : hierarchy_(_hierarchy), addedEntityId_(_addedEntityId) {}

void EntityHierarchy::AddSelectedEntitiesCommand::Execute() {
    auto& selectedEntityIds = hierarchy_->selectedEntityIds_;
    if (std::find(selectedEntityIds.begin(), selectedEntityIds.end(), addedEntityId_) == selectedEntityIds.end()) {
        selectedEntityIds.push_back(addedEntityId_);
        LOG_DEBUG("AddSelectedEntitiesCommand::Execute: Added entity ID '{}' to selection.", addedEntityId_);
    } else {
        LOG_DEBUG("AddSelectedEntitiesCommand::Execute: Entity ID '{}' is already selected.", addedEntityId_);
    }
}

void EntityHierarchy::AddSelectedEntitiesCommand::Undo() {
    auto& selectedEntityIds = hierarchy_->selectedEntityIds_;
    auto it                 = std::remove(selectedEntityIds.begin(), selectedEntityIds.end(), addedEntityId_);
    if (it != selectedEntityIds.end()) {
        selectedEntityIds.erase(it, selectedEntityIds.end());
        LOG_DEBUG("AddSelectedEntitiesCommand::Undo: Removed entity ID '{}' from selection.", addedEntityId_);
    } else {
        LOG_DEBUG("AddSelectedEntitiesCommand::Undo: Entity ID '{}' was not in selection.", addedEntityId_);
    }
}

EntityHierarchy::RemoveSelectedEntitiesCommand::RemoveSelectedEntitiesCommand(EntityHierarchy* _hierarchy, int32_t _removedEntityId)
    : hierarchy_(_hierarchy), removedEntityId_(_removedEntityId) {}

void EntityHierarchy::RemoveSelectedEntitiesCommand::Execute() {
    auto& selectedEntityIds = hierarchy_->selectedEntityIds_;
    auto it                 = std::remove(selectedEntityIds.begin(), selectedEntityIds.end(), removedEntityId_);
    if (it != selectedEntityIds.end()) {
        selectedEntityIds.erase(it, selectedEntityIds.end());
        LOG_DEBUG("RemoveSelectedEntitiesCommand::Execute: Removed entity ID '{}' from selection.", removedEntityId_);
    } else {
        LOG_DEBUG("RemoveSelectedEntitiesCommand::Execute: Entity ID '{}' was not in selection.", removedEntityId_);
    }
}

void EntityHierarchy::RemoveSelectedEntitiesCommand::Undo() {
    auto& selectedEntityIds = hierarchy_->selectedEntityIds_;
    if (std::find(selectedEntityIds.begin(), selectedEntityIds.end(), removedEntityId_) == selectedEntityIds.end()) {
        selectedEntityIds.push_back(removedEntityId_);
        LOG_DEBUG("RemoveSelectedEntitiesCommand::Undo: Added entity ID '{}' back to selection.", removedEntityId_);
    } else {
        LOG_DEBUG("RemoveSelectedEntitiesCommand::Undo: Entity ID '{}' is already in selection.", removedEntityId_);
    }
}

EntityHierarchy::ClearSelectedEntitiesCommand::ClearSelectedEntitiesCommand(EntityHierarchy* _hierarchy) : hierarchy_(_hierarchy) {}

void EntityHierarchy::ClearSelectedEntitiesCommand::Execute() {
    auto& selectedEntityIds    = hierarchy_->selectedEntityIds_;
    previousSelectedEntityIds_ = selectedEntityIds; // 現在の選択状態を保存
    if (!selectedEntityIds.empty()) {
        selectedEntityIds.clear();
        LOG_DEBUG("ClearSelectedEntitiesCommand::Execute: Cleared all selected entities.");
    } else {
        LOG_DEBUG("ClearSelectedEntitiesCommand::Execute: No entities were selected to clear.");
    }
}

void EntityHierarchy::ClearSelectedEntitiesCommand::Undo() {
    auto& selectedEntityIds = hierarchy_->selectedEntityIds_;
    if (previousSelectedEntityIds_.empty()) {
        LOG_DEBUG("ClearSelectedEntitiesCommand::Undo: No previous selection to restore.");
        return;
    }
    selectedEntityIds = previousSelectedEntityIds_; // 保存した選択状態を復元
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

void SelectAddComponentArea::SetTargeEntities::Execute() {
    parentArea_->targetEntityIds_ = targetEntityIds_; // 新しいターゲットエンティティIDを設定
}

void SelectAddComponentArea::SetTargeEntities::Undo() {
    parentArea_->targetEntityIds_ = previousTargetEntityIds_; // 保存したターゲットエンティティIDを復元
}

void SelectAddComponentArea::ClearTargetEntities::Execute() {
    parentArea_->targetEntityIds_.clear(); // ターゲットエンティティIDをクリア
}

void SelectAddComponentArea::ClearTargetEntities::Undo() {
    parentArea_->targetEntityIds_ = previousTargetEntityIds_; // 保存したターゲットエンティティIDを復元
}

void SelectAddComponentArea::AddComponentsForTargetEntities::Execute() {
    auto currentScene = parentArea_->parentWindow_->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("AddComponentsForTargetEntities::Execute: No current scene found.");
        return;
    }

    auto window = EditorController::getInstance()->getWindow<SceneEditorWindow>();
    if (!window) {
        LOG_ERROR("AddComponentsForTargetEntities::Undo: No SceneEditorWindow found.");
        return;
    }
    auto* entityInspectorArea = dynamic_cast<EntityInspectorArea*>(window->getArea(nameof<EntityInspectorArea>()).get()); // 選択状態をクリア
    if (!entityInspectorArea) {
        LOG_ERROR("AddComponentsForTargetEntities::Undo: EntityInspectorArea not found in SceneEditorWindow.");
        return;
    }

    int32_t editEntityId = entityInspectorArea->getEditEntityId();
    for (const auto& entityId : parentArea_->targetEntityIds_) {
        GameEntity* entity = currentScene->getEntity(entityId);
        if (!entity) {
            LOG_ERROR("AddComponentsForTargetEntities::Execute: Entity with ID '{}' not found.", entityId);
            continue;
        }
        if (editEntityId == entityId) {
            for (const auto& componentTypeName : parentArea_->componentTypeNames_) {
                currentScene->addComponent(componentTypeName, entityId, true);
                auto addCompCommand = std::make_unique<EntityInspectorArea::AddComponentCommand>(entityInspectorArea, componentTypeName);
                EditorController::getInstance()->pushCommand(std::move(addCompCommand));
            }
        } else {
            for (const auto& componentTypeName : parentArea_->componentTypeNames_) {
                currentScene->addComponent(componentTypeName, entityId, true);
            }
        }
    }
}

void SelectAddComponentArea::AddComponentsForTargetEntities::Undo() {
    auto currentScene = parentArea_->parentWindow_->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("AddComponentsForTargetEntities::Undo: No current scene found.");
        return;
    }
    auto window = EditorController::getInstance()->getWindow<SceneEditorWindow>();
    if (!window) {
        LOG_ERROR("AddComponentsForTargetEntities::Undo: No SceneEditorWindow found.");
        return;
    }
    auto* entityInspectorArea = dynamic_cast<EntityInspectorArea*>(window->getArea(nameof<EntityInspectorArea>()).get()); // 選択状態をクリア
    if (!entityInspectorArea) {
        LOG_ERROR("AddComponentsForTargetEntities::Undo: EntityInspectorArea not found in SceneEditorWindow.");
        return;
    }
    int32_t editEntityId = entityInspectorArea->getEditEntityId();
    for (const auto& entityId : parentArea_->targetEntityIds_) {
        GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(entityId);
        if (!entity) {
            LOG_ERROR("AddComponentsForTargetEntities::Undo: Entity with ID '{}' not found.", entityId);
            continue;
        }
        if (editEntityId == entityId) {
            for (const auto& componentTypeName : parentArea_->componentTypeNames_) {
                IComponentArray* compArray = currentScene->getComponentRepositoryRef()->getComponentArray(componentTypeName);
                if (!compArray) {
                    LOG_ERROR("AddComponentsForTargetEntities::Undo: ComponentArray '{}' not found.", componentTypeName);
                    continue;
                }
                if (compArray->hasEntity(entity)) {
                    compArray->removeComponent(entity, compArray->getComponentSize(entity) - 1); // 最後のコンポーネントを削除
                    auto removeCompCommand = std::make_unique<EntityInspectorArea::RemoveComponentCommand>(entityInspectorArea, componentTypeName);
                    EditorController::getInstance()->pushCommand(std::move(removeCompCommand));
                }
            }

        } else {
            for (const auto& componentTypeName : parentArea_->componentTypeNames_) {
                IComponentArray* compArray = currentScene->getComponentRepositoryRef()->getComponentArray(componentTypeName);
                if (!compArray) {
                    LOG_ERROR("AddComponentsForTargetEntities::Undo: ComponentArray '{}' not found.", componentTypeName);
                    continue;
                }
                if (compArray->hasEntity(entity)) {
                    compArray->removeComponent(entity, compArray->getComponentSize(entity) - 1); // 最後のコンポーネントを削除
                }
            }
        }
    }
}

EntityHierarchy::CreateEntityCommand::CreateEntityCommand(HierarchyArea* _parentArea, const std::string& _entityName) {
    parentArea_       = _parentArea;
    entityName_       = _entityName;
    auto currentScene = parentArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("CreateEntityCommand::CreateEntityCommand: No current scene found.");
        return;
    }
    SceneSerializer serializer(currentScene);
    entityData_ = nlohmann::json::object();
    serializer.EntityToJson(entityId_, entityData_);
}

void EntityHierarchy::CreateEntityCommand::Execute() {
    auto currentScene = parentArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("CreateEntityCommand::Execute: No current scene found.");
        return;
    }
    entityId_ = currentScene->getEntityRepositoryRef()->registerEntity(entityName_);

    SceneSerializer serializer(currentScene);
    serializer.EntityToJson(entityId_, entityData_);

    LOG_DEBUG("CreateEntityCommand::Execute: Created entity with ID '{}'.", entityId_);
}

void EntityHierarchy::CreateEntityCommand::Undo() {
    auto currentScene = parentArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("CreateEntityCommand::Undo: No current scene found.");
        return;
    }
    SceneSerializer serializer(currentScene);
    GameEntity* entity = serializer.EntityFromJson(entityId_, entityData_);

    if (!entity) {
        LOG_ERROR("CreateEntityCommand::Undo: Failed to restore entity with ID '{}'.", entityId_);
        return;
    }

    LOG_DEBUG("CreateEntityCommand::Undo: Removed entity with ID '{}'.", entityId_);
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
    currentScene->getEntityRepositoryRef()->unregisterEntity(entityId_);
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

void SelectAddComponentArea::ClearComponentTypeNames::Execute() {
    componentTypeNames_ = parentArea_->componentTypeNames_; // 現在のコンポーネントタイプ名を保存
    parentArea_->componentTypeNames_.clear(); // コンポーネントタイプ名をクリア

    LOG_DEBUG("SelectAddComponentArea::ClearComponentTypeNames::Execute: Cleared component type names.");
}

void SelectAddComponentArea::ClearComponentTypeNames::Undo() {
    parentArea_->componentTypeNames_ = componentTypeNames_; // 保存したコンポーネントタイプ名を復元
    LOG_DEBUG("SelectAddComponentArea::ClearComponentTypeNames::Undo: Restored component type names.");
}
