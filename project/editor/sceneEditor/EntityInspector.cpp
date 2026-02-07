#include "EntityInspector.h"
#include "EntityInspectorCommands.h"

#ifdef _DEBUG

/// engine
#define RESOURCE_DIRECTORY
#include "editor/EditorConfig.h"
#include "engine/code/EngineConfig.h"
#include "EngineInclude.h"

#include "scene/SceneFactory.h"
#include "scene/SceneJsonRegistry.h"

/// ECS
#include "system/SystemRunner.h"

// scene
#include "scene/SceneManager.h"

/// editor
#include "editor/EditorController.h"
#include "editor/sceneEditor/SceneEditor.h"
#include "editor/sceneEditor/SystemInspector.h"

/// externals
#include "myGui/MyGui.h"

using namespace OriGine;

#pragma region "EntityInspector"

EntityInspectorArea::EntityInspectorArea(SceneEditorWindow* _window)
    : parentWindow_(_window), Editor::Area(nameof<EntityInspectorArea>()) {}
EntityInspectorArea::~EntityInspectorArea() {}
void EntityInspectorArea::Initialize() {
    AddRegion(::std::make_shared<EntityInformationRegion>(this));
    AddRegion(::std::make_shared<EntityComponentRegion>(this));
    AddRegion(::std::make_shared<EntitySystemRegion>(this));
}
void EntityInspectorArea::Finalize() {
    for (auto& [name, region] : regions_) {
        region->Finalize();
        region.reset();
    }
    regions_.clear();
}

EntityInformationRegion::EntityInformationRegion(EntityInspectorArea* _parent)
    : Editor::Region(nameof(this)), parentArea_(_parent) {}
EntityInformationRegion::~EntityInformationRegion() {}
void EntityInformationRegion::Initialize() {}
void EntityInformationRegion::DrawGui() {
    auto currentScene             = parentArea_->GetParentWindow()->GetCurrentScene();
    EntityHandle editEntityHandle = parentArea_->GetEditEntityHandle();
    auto editEntity               = currentScene->GetEntityRepositoryRef()->GetEntity(editEntityHandle);

    ::ImGui::Text("Entity Information");
    if (!editEntity) {
        ::ImGui::Text("No entity selected.");
        return;
    }

    if (::ImGui::Button("Delete")) {
        auto deleteCommand = ::std::make_unique<DeleteEntityCommand>(parentArea_, editEntityHandle);
        OriGine::EditorController::GetInstance()->PushCommand(::std::move(deleteCommand));

        auto changeEditEntityCommand = ::std::make_unique<ChangeEditEntityCommand>(parentArea_, EntityHandle(), editEntityHandle);
        OriGine::EditorController::GetInstance()->PushCommand(::std::move(changeEditEntityCommand));
    }

    if (::ImGui::Button("SaveForFile")) {
        // templateに追加
        SceneJsonRegistry::GetInstance()->RegisterEntityTemplateFromEntity(editEntity->GetDataType(), currentScene, editEntity);
        // 追加したtemplateを保存
        SceneJsonRegistry::GetInstance()->SaveEntityTemplate(kApplicationResourceDirectory + '/' + kEntityTemplateFolder, editEntity->GetDataType());
    }

    ::ImGui::Spacing();
    ::ImGui::Separator();
    ::ImGui::Spacing();

    std::string handleStr = uuids::to_string(editEntityHandle.uuid);

    ::ImGui::Text("Entity Handle  : %s", handleStr.c_str());
    {
        ::std::string label = "EntityName##" + handleStr;
        ::ImGui::InputText(label.c_str(), &parentArea_->GetEditEntityNameRef());

        if (::ImGui::IsItemDeactivatedAfterEdit()) {
            auto command = ::std::make_unique<ChangeEntityNameCommand>(parentArea_, editEntityHandle, parentArea_->GetEditEntityName());
            OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
        }
    }

    bool isUnique = editEntity->IsUnique();
    if (::ImGui::Checkbox("Entity is Unique", &isUnique)) {
        auto command = ::std::make_unique<ChangeEntityUniquenessCommand>(parentArea_, editEntityHandle, editEntity->IsUnique());
        OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
    }

    if (::ImGui::IsItemHovered()) {
        ::ImGui::SetTooltip("Unique entities cannot be duplicated.");
    }

    bool ShouldSave = editEntity->ShouldSave();
    if (::ImGui::Checkbox("Should Save", &ShouldSave)) {
        auto command = ::std::make_unique<ChangeEntityShouldSaveCommand>(parentArea_, editEntityHandle, !ShouldSave /*変更前に戻す*/);
        OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
    }
}
void EntityInformationRegion::Finalize() {}

EntityComponentRegion::EntityComponentRegion(EntityInspectorArea* _parent)
    : Editor::Region(nameof<EntityComponentRegion>()), parentArea_(_parent) {}
EntityComponentRegion::~EntityComponentRegion() {}
void EntityComponentRegion::Initialize() {}
void EntityComponentRegion::DrawGui() {
    auto currentScene             = parentArea_->GetParentWindow()->GetCurrentScene();
    EntityHandle editEntityHandle = parentArea_->GetEditEntityHandle();
    auto editEntity               = currentScene->GetEntity(editEntityHandle);

    ::ImGui::SeparatorText("Entity Components");
    ::ImGui::Spacing();

    if (!editEntity) {
        return;
    }
    ::ImGui::Indent();

    if (::ImGui::Button("+ Add Component")) {
        auto selectAddComponentArea = parentArea_->GetParentWindow()->GetAreas().find("SelectAddComponentArea")->second.get();
        if (selectAddComponentArea) {
            auto selectArea = dynamic_cast<SelectAddComponentArea*>(selectAddComponentArea);
            if (selectArea) {
                selectArea->SetTargets({editEntityHandle});
                selectArea->SetOpen(true);
            } else {
                LOG_ERROR("Failed to cast SelectAddComponentArea.");
            }
        } else {
            LOG_ERROR("SelectAddComponentArea not found in parent window.");
        }
    }

    ::ImGui::Spacing();

    Scene* editScene = parentArea_->GetParentWindow()->GetCurrentScene();

    for (const auto& [componentTypeName, components] : editScene->GetComponentRepositoryRef()->GetAllComponentsOfEntity(editEntityHandle)) {
        if (::ImGui::CollapsingHeader(componentTypeName.c_str())) {
            ::ImGui::Indent();
            if (components.size() > 1) {
                int32_t componentIndex = 0;
                ::std::string label    = "";
                for (const auto& component : components) {
                    label = componentTypeName + ::std::to_string(componentIndex);

                    if (::ImGui::Button(::std::string("X##" + label).c_str())) {
                        auto removeCommand = ::std::make_unique<RemoveComponentFromEditListCommand>(parentArea_, componentTypeName, componentIndex);
                        OriGine::EditorController::GetInstance()->PushCommand(::std::move(removeCommand));
                        continue; // ボタンが押されたら次のコンポーネントへ
                    }
                    ::ImGui::SameLine();

                    label = ::std::format("{} : [{}]##{}", componentTypeName, ::std::to_string(componentIndex), editEntity->GetUniqueID());
                    if (::ImGui::TreeNode(label.c_str())) {
                        component->Edit(editScene, editEntityHandle, label);
                        ::ImGui::TreePop();
                    }

                    ++componentIndex;
                }
            } else if (components.size() == 1) {
                const auto& component = components.back();

                ::std::string label = "";
                label               = componentTypeName;

                if (::ImGui::Button(::std::string("X##" + label).c_str())) {
                    auto removeCommand = ::std::make_unique<RemoveComponentFromEditListCommand>(parentArea_, componentTypeName, 0);
                    OriGine::EditorController::GetInstance()->PushCommand(::std::move(removeCommand));
                    continue; // ボタンが押されたら次のコンポーネントへ
                }
                ::ImGui::SameLine();

                ::ImGui::Text("%s", componentTypeName.c_str());

                label = ::std::format("{}:[{}]{}", componentTypeName, ::std::to_string(0), editEntity->GetUniqueID());
                component->Edit(editScene, editEntityHandle, label);
            }
            ::ImGui::Unindent();
        }
    }

    ::ImGui::Unindent();
}
void EntityComponentRegion::Finalize() {}

EntitySystemRegion::EntitySystemRegion(EntityInspectorArea* _parent)
    : Editor::Region(nameof<EntitySystemRegion>()), parentArea_(_parent) {}
EntitySystemRegion::~EntitySystemRegion() {}
void EntitySystemRegion::Initialize() {}
void EntitySystemRegion::DrawGui() {
    auto currentScene             = parentArea_->GetParentWindow()->GetCurrentScene();
    EntityHandle editEntityHandle = parentArea_->GetEditEntityHandle();
    auto editEntity               = currentScene->GetEntityRepositoryRef()->GetEntity(editEntityHandle);

    ::ImGui::SeparatorText("Entity Systems");
    ::ImGui::Spacing();

    if (!editEntity) {
        return;
    }

    ::ImGui::Indent();

    if (::ImGui::Button("+ System")) {
        auto selectAddSystemArea = parentArea_->GetParentWindow()->GetAreas().find("SelectAddSystemArea")->second.get();
        if (selectAddSystemArea) {
            auto selectArea = dynamic_cast<SelectAddSystemArea*>(selectAddSystemArea);
            if (selectArea) {
                ::std::list<EntityHandle> targets = {editEntityHandle};
                auto SetTargetsCommand            = ::std::make_unique<SetSystemTargetEntitiesCommand>(selectArea, targets);
                OriGine::EditorController::GetInstance()->PushCommand(::std::move(SetTargetsCommand));
                selectArea->SetOpen(true);
                selectArea->SetFocused(true);
            } else {
                LOG_ERROR("Failed to cast SelectAddSystemArea.");
            }
        } else {
            LOG_ERROR("SelectAddSystemArea not found in parent window.");
        }
    }

    auto& systemMap = parentArea_->GetSystemMap();
    for (size_t i = 0; i < systemMap.size(); ++i) {
        auto& systems = systemMap[i];
        if (systems.empty()) {
            continue;
        }
        if (::ImGui::CollapsingHeader(kSystemCategoryString[i].c_str())) {
            ::ImGui::Indent();
            for (auto& [systemName, system] : systems) {
                ::std::string popupId = "ConfirmRemoveSystem##" + systemName;
                if (::ImGui::Button(systemName.c_str())) {
                    ::ImGui::OpenPopup(popupId.c_str());
                }
                if (::ImGui::BeginPopup(popupId.c_str())) {
                    ::ImGui::Text("Are you sure you want to remove the system '%s'?", systemName.c_str());
                    if (::ImGui::Button("Yes")) {
                        ::std::list<EntityHandle> editEntityHandles = {editEntityHandle};
                        auto command                                = ::std::make_unique<RemoveSystemCommand>(editEntityHandles, systemName, system->GetCategory());
                        OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
                        ::ImGui::CloseCurrentPopup();
                    }
                    ::ImGui::SameLine();
                    if (::ImGui::Button("No")) {
                        ::ImGui::CloseCurrentPopup();
                    }
                    ::ImGui::EndPopup();
                }
            }
            ::ImGui::Unindent();
        }
    }

    ::ImGui::Unindent();
}
void EntitySystemRegion::Finalize() {}

SelectAddComponentArea::SelectAddComponentArea(SceneEditorWindow* _parentWindow)
    : Editor::Area(nameof(this)), parentWindow_(_parentWindow) {
    isOpen_.Set(false);
    isFocused_.Set(false);
}
SelectAddComponentArea::~SelectAddComponentArea() {}

void SelectAddComponentArea::Initialize() {
    AddRegion(::std::make_shared<ComponentListRegion>(this));
    // 初期化時にコンポーネントのタイプ名をクリア
    componentTypeNames_.clear();
    targetEntityHandles_.clear();
}

void SelectAddComponentArea::DrawGui() {
    bool isOpen = isOpen_.Current();

    if (!isOpen) {
        isOpen_.Set(isOpen);
        isFocused_.Set(::ImGui::IsWindowFocused());
        UpdateFocusAndOpenState();
        return;
    }

    if (::ImGui::Begin(name_.c_str(), &isOpen)) {
        if (!::ImGui::IsWindowFocused()) {
            ::ImGui::SetWindowFocus(); // このウィンドウを最前面に
        }

        areaSize_ = ::ImGui::GetWindowSize();

        for (auto& [name, region] : regions_) {
            if (region) {
                ::ImGui::BeginGroup();
                region->DrawGui();
                ::ImGui::EndGroup();
            }
        }
    }

    isOpen_.Set(isOpen);
    isFocused_.Set(::ImGui::IsWindowFocused());
    UpdateFocusAndOpenState();

    ::ImGui::End();
}

void SelectAddComponentArea::Finalize() {
    Editor::Area::Finalize();
    componentTypeNames_.clear(); // コンポーネントのタイプ名をクリア
    targetEntityHandles_.clear(); // 対象のエンティティIDリストをクリア
}

void SelectAddComponentArea::SetTargets(const ::std::list<EntityHandle>& _targets) {
    targetEntityHandles_ = _targets;
}

SelectAddComponentArea::ComponentListRegion::ComponentListRegion(SelectAddComponentArea* _parentArea) : Editor::Region(nameof(this)), parentArea_(_parentArea) {}
SelectAddComponentArea::ComponentListRegion::~ComponentListRegion() {}

void SelectAddComponentArea::ComponentListRegion::Initialize() {
}

void SelectAddComponentArea::ComponentListRegion::DrawGui() {
    ::std::string label = "Search##SelectAddComponent";

    ::ImGui::InputText(label.c_str(), &searchBuff_[0], sizeof(char) * 256);
    searchBuff_ = ::std::string(searchBuff_.c_str());

    float parentHeight = ::ImGui::GetWindowHeight();
    float itemHeight   = ::ImGui::GetItemRectSize().y * 2.f; // 直前のアイテムの高さ
    float padding      = ::ImGui::GetStyle().WindowPadding.y * 6.f;

    // 必要に応じてパディングを2倍（上下分）にする場合も
    float childHeight = parentHeight - (itemHeight + padding);

    ::ImGui::BeginChild(
        "ComponentList",
        ImVec2(0, childHeight),
        ImGuiChildFlags_Border);

    auto& componentTypeNames = ComponentRegistry::GetInstance()->GetComponentTypeNames();

    // ImGuiのスタイルで選択色を設定（必要に応じてアプリ全体で設定してもOK）
    const auto& c         = EditorConfig::Color::kWinSelectColor;
    ImVec4 winSelectColor = ImVec4(c[R], c[G], c[B], c[A]); // Windows風の青
    ::ImGui::PushStyleColor(ImGuiCol_Header, winSelectColor);
    ::ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(c[R], c[G], c[B], EditorConfig::Color::kWinSelectHeaderAlpha));
    ::ImGui::PushStyleColor(ImGuiCol_HeaderActive, winSelectColor);
    for (auto& name : componentTypeNames) {

        if (searchBuff_.size() > 0) {
            if (name.find(searchBuff_) == ::std::string::npos) {
                continue; // 検索文字列に一致しない場合はスキップ
            }
        }

        // 選択状態か判定
        bool isSelected = ::std::find(parentArea_->componentTypeNames_.begin(), parentArea_->componentTypeNames_.end(), name) != parentArea_->componentTypeNames_.end();

        // Selectableで表示
        if (::ImGui::Selectable(name.c_str(), isSelected)) {
            if (!isSelected) {
                if (!::ImGui::GetIO().KeyShift) {
                    auto clearCommand = ::std::make_unique<ClearComponentTypeNamesCommand>(parentArea_);
                    OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearCommand));
                }
                // まだ選択されていなければ追加
                auto command = ::std::make_unique<AddComponentTypeNamesCommand>(parentArea_, name);
                OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
            } else {
                // すでに選択されていれば解除
                auto command = ::std::make_unique<RemoveComponentTypeNamesCommand>(parentArea_, name);
                OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
            }
        }
    }

    ::ImGui::PopStyleColor(3); // スタイルのポップ

    ::ImGui::EndChild();

    ::ImGui::Spacing();

    if (parentArea_->componentTypeNames_.empty()) {
        bool selected = false;
        ::ImGui::Selectable("OK", &selected, ImGuiSelectableFlags_Disabled);
    } else {
        if (::ImGui::Button("OK")) {
            // OKボタンが押された場合、選択されたコンポーネントを追加
            for (auto& compTypeName : parentArea_->componentTypeNames_) {
                auto addCompCommand = ::std::make_unique<AddComponentCommand>(parentArea_->targetEntityHandles_, compTypeName);
                OriGine::EditorController::GetInstance()->PushCommand(::std::move(addCompCommand));
            }

            auto clearEntitiesCommand = ::std::make_unique<ClearComponentTargetEntitiesCommand>(parentArea_);
            OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearEntitiesCommand));

            auto clearComponentNamesCommand = ::std::make_unique<ClearComponentTypeNamesCommand>(parentArea_);
            OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearComponentNamesCommand));

            parentArea_->isOpen_.Set(false); // エリアを閉じる
            parentArea_->isFocused_.Set(false); // フォーカスを外す
        }
    }

    ::ImGui::SameLine();

    if (::ImGui::Button("CANCEL")) {
        // キャンセルボタンが押された場合、選択をクリア
        auto clearEntitiesCommand = ::std::make_unique<ClearComponentTargetEntitiesCommand>(parentArea_);
        OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearEntitiesCommand));

        auto clearComponentNamesCommand = ::std::make_unique<ClearComponentTypeNamesCommand>(parentArea_);
        OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearComponentNamesCommand));

        parentArea_->isOpen_.Set(false); // エリアを閉じる
        parentArea_->isFocused_.Set(false); // フォーカスを外す
    }
}

void SelectAddComponentArea::ComponentListRegion::Finalize() {}

SelectAddSystemArea::SelectAddSystemArea(SceneEditorWindow* _parentWindow)
    : Editor::Area(nameof<SelectAddSystemArea>()), parentWindow_(_parentWindow) {}
SelectAddSystemArea::~SelectAddSystemArea() {}

void SelectAddSystemArea::Initialize() {
    AddRegion(::std::make_shared<SystemListRegion>(this));

    isOpen_.Set(false); // 初期状態では閉じている
    isFocused_.Set(false); // 初期状態ではフォーカスされていない
}
void SelectAddSystemArea::DrawGui() {
    bool isOpen = isOpen_.Current();

    if (!isOpen) {
        isOpen_.Set(isOpen);
        isFocused_.Set(::ImGui::IsWindowFocused());
        UpdateFocusAndOpenState();
        return;
    }

    if (::ImGui::Begin(name_.c_str(), &isOpen)) {
        if (!::ImGui::IsWindowFocused()) {
            ::ImGui::SetWindowFocus(); // このウィンドウを最前面に
        }

        areaSize_ = ::ImGui::GetWindowSize();

        for (auto& [name, region] : regions_) {
            if (region) {
                ::ImGui::BeginGroup();
                region->DrawGui();
                ::ImGui::EndGroup();
            }
        }
    }

    isOpen_.Set(isOpen);
    isFocused_.Set(::ImGui::IsWindowFocused());
    UpdateFocusAndOpenState();

    ::ImGui::End();
}
void SelectAddSystemArea::Finalize() {
    Editor::Area::Finalize();
    systemTypeNames_.clear(); // システムのタイプ名をクリア
    targetEntityHandles_.clear(); // 対象のエンティティIDリストをクリア
}
void SelectAddSystemArea::SetTargets(const ::std::list<EntityHandle>& _targets) {
    targetEntityHandles_ = _targets;
}

SelectAddSystemArea::SystemListRegion::SystemListRegion(SelectAddSystemArea* _parentArea) : Editor::Region(nameof(this)), parentArea_(_parentArea) {}
SelectAddSystemArea::SystemListRegion::~SystemListRegion() {}
void SelectAddSystemArea::SystemListRegion::Initialize() {
}
void SelectAddSystemArea::SystemListRegion::DrawGui() {
    ::std::string label = "Search##SelectAddSystem";
    ::ImGui::InputText(label.c_str(), &searchBuff_[0], sizeof(char) * 256);
    searchBuff_ = ::std::string(searchBuff_.c_str());

    float parentHeight = ::ImGui::GetWindowHeight();
    float itemHeight   = ::ImGui::GetItemRectSize().y * 2.f; // 直前のアイテムの高さ
    float padding      = ::ImGui::GetStyle().WindowPadding.y * 6.f;

    // 必要に応じてパディングを2倍（上下分）にする場合も
    float childHeight = parentHeight - (itemHeight + padding);

    ::ImGui::BeginChild(
        "ComponentList",
        ImVec2(0, childHeight),
        ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysUseWindowPadding);

    auto sceneEditorWindow = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene      = sceneEditorWindow->GetCurrentScene();
    auto systemInspector   = dynamic_cast<SystemInspectorArea*>(sceneEditorWindow->GetArea(nameof<SystemInspectorArea>()).get());
    auto& systemsMap       = systemInspector->GetSystemMap();

    const auto& c         = EditorConfig::Color::kWinSelectColor;
    ImVec4 winSelectColor = ImVec4(c[R], c[G], c[B], c[A]);
    ::ImGui::PushStyleColor(ImGuiCol_Header, winSelectColor);
    ::ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(c[R], c[G], c[B], EditorConfig::Color::kWinSelectHeaderAlpha));
    ::ImGui::PushStyleColor(ImGuiCol_HeaderActive, winSelectColor);

    if (searchBuff_.empty()) {
        // カテゴリごとに見出しをつけて表示
        for (size_t i = 0; i < systemsMap.size(); ++i) {
            auto& systemsByCategory = systemsMap[i];
            if (systemsByCategory.empty()) {
                continue;
            }
            const ::std::string& categoryName = kSystemCategoryString[i];
            if (::ImGui::CollapsingHeader(categoryName.c_str())) {
                ::ImGui::Indent();
                for (auto& [name, priority] : systemsByCategory) {
                    std::shared_ptr<OriGine::ISystem> system = currentScene->GetSystem(name);

                    if (!system) {
                        continue;
                    }
                    if (!system->IsActive()) {
                        continue;
                    }

                    bool isSelected = ::std::find(parentArea_->systemTypeNames_.begin(), parentArea_->systemTypeNames_.end(), name) != parentArea_->systemTypeNames_.end();
                    if (::ImGui::Selectable(name.c_str(), isSelected)) {
                        if (!isSelected) {
                            if (!::ImGui::GetIO().KeyShift) {
                                auto clearCommand = ::std::make_unique<ClearSystemNamesCommand>(parentArea_);
                                OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearCommand));
                            }
                            auto command = ::std::make_unique<AddSystemNamesCommand>(parentArea_, name);
                            OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
                        } else {
                            auto command = ::std::make_unique<RemoveSystemNamesCommand>(parentArea_, name);
                            OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
                        }
                    }
                }
                ::ImGui::Unindent();
            }
        }
    } else {
        // 検索結果のみカテゴリ見出しなしで表示
        for (size_t i = 0; i < systemsMap.size(); ++i) {
            auto& systemsByCategory = systemsMap[i];
            for (auto& [name, priority] : systemsByCategory) {
                std::shared_ptr<OriGine::ISystem> system = currentScene->GetSystem(name);

                if (!system) {
                    continue;
                }
                if (!system->IsActive()) {
                    continue;
                }

                if (name.find(searchBuff_) == ::std::string::npos) {
                    continue;
                }

                bool isSelected = ::std::find(parentArea_->systemTypeNames_.begin(), parentArea_->systemTypeNames_.end(), name) != parentArea_->systemTypeNames_.end();
                if (::ImGui::Selectable(name.c_str(), isSelected)) {
                    if (!isSelected) {
                        if (!::ImGui::GetIO().KeyShift) {
                            auto clearCommand = ::std::make_unique<ClearSystemNamesCommand>(parentArea_);
                            OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearCommand));
                        }
                        auto command = ::std::make_unique<AddSystemNamesCommand>(parentArea_, name);
                        OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
                    } else {
                        auto command = ::std::make_unique<RemoveSystemNamesCommand>(parentArea_, name);
                        OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
                    }
                }
            }
        }
    }
    ::ImGui::PopStyleColor(3);

    ::ImGui::EndChild();

    ::ImGui::Spacing();

    if (parentArea_->systemTypeNames_.empty()) {
        bool selected = false;
        ::ImGui::Selectable("OK", &selected, ImGuiSelectableFlags_Disabled);
    } else {
        if (::ImGui::Button("OK")) {
            auto addSystemCommand = ::std::make_unique<AddSystemsForTargetEntitiesCommand>(parentArea_, parentArea_->targetEntityHandles_, parentArea_->systemTypeNames_);
            OriGine::EditorController::GetInstance()->PushCommand(::std::move(addSystemCommand));
            auto clearEntitiesCommand = ::std::make_unique<ClearSystemTargetEntitiesCommand>(parentArea_);
            OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearEntitiesCommand));
            auto clearSystemNamesCommand = ::std::make_unique<ClearSystemNamesCommand>(parentArea_);
            OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearSystemNamesCommand));
            parentArea_->isOpen_.Set(false);
            parentArea_->isFocused_.Set(false);
        }
    }
    ::ImGui::SameLine();
    if (::ImGui::Button("CANCEL")) {
        auto clearEntitiesCommand = ::std::make_unique<ClearSystemTargetEntitiesCommand>(parentArea_);
        OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearEntitiesCommand));
        auto clearSystemNamesCommand = ::std::make_unique<ClearSystemNamesCommand>(parentArea_);
        OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearSystemNamesCommand));
        parentArea_->isOpen_.Set(false);
        parentArea_->isFocused_.Set(false);
    }
}
void SelectAddSystemArea::SystemListRegion::Finalize() {}

#pragma endregion

void SelectAddComponentArea::RemoveComponentTypeName(const ::std::string& _name) {
    auto& names = componentTypeNames_;
    auto it     = ::std::remove(names.begin(), names.end(), _name);
    if (it != names.end()) {
        names.erase(it, names.end());
    }
}

void SelectAddSystemArea::RemoveSystemTypeName(const ::std::string& _name) {
    auto& names = systemTypeNames_;
    auto it     = ::std::remove(names.begin(), names.end(), _name);
    if (it != names.end()) {
        names.erase(it, names.end());
    }
}

#endif // _DEBUG
