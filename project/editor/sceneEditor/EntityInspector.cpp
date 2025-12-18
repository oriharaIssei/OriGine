#include "EntityInspector.h"

#ifdef _DEBUG

/// engine
#define RESOURCE_DIRECTORY
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
    auto currentScene    = parentArea_->GetParentWindow()->GetCurrentScene();
    int32_t editEntityId = parentArea_->GetEditEntityId();
    auto editEntity      = currentScene->GetEntityRepositoryRef()->GetEntity(editEntityId);

    ::ImGui::Text("Entity Information");
    if (!editEntity) {
        ::ImGui::Text("No entity selected.");
        return;
    }

    if (::ImGui::Button("Delete")) {
        auto deleteCommand = ::std::make_unique<DeleteEntityCommand>(parentArea_, editEntityId);
        OriGine::EditorController::GetInstance()->PushCommand(::std::move(deleteCommand));

        auto changeEditEntityCommand = ::std::make_unique<EntityInspectorArea::ChangeEditEntityCommand>(parentArea_, -1, editEntityId);
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

    ::ImGui::Text("Entity ID  : %d", editEntityId);
    {
        ::std::string label = "EntityName##" + ::std::to_string(editEntityId);
        ::ImGui::InputText(label.c_str(), &parentArea_->GetEditEntityNameRef());

        if (::ImGui::IsItemDeactivatedAfterEdit()) {
            auto command = ::std::make_unique<ChangeEntityName>(parentArea_, editEntityId, parentArea_->GetEditEntityName());
            OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
        }
    }

    bool isUnique = editEntity->IsUnique();
    if (::ImGui::Checkbox("Entity is Unique", &isUnique)) {
        auto command = ::std::make_unique<ChangeEntityUniqueness>(parentArea_, editEntityId, editEntity->IsUnique());
        OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
    }

    if (::ImGui::IsItemHovered()) {
        ::ImGui::SetTooltip("Unique entities cannot be duplicated.");
    }

    bool ShouldSave = editEntity->ShouldSave();
    if (::ImGui::Checkbox("Should Save", &ShouldSave)) {
        auto command = ::std::make_unique<ChangeEntityShouldSave>(parentArea_, editEntityId, !ShouldSave /*変更前に戻す*/);
        OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
    }
}
void EntityInformationRegion::Finalize() {}

EntityInformationRegion::ChangeEntityName::ChangeEntityName(EntityInspectorArea* _inspectorArea, int32_t _entityID, const ::std::string& _newName)
    : inspectorArea_(_inspectorArea), entityId_(_entityID), newName_(_newName) {
    auto currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();
    oldName_          = currentScene->GetEntityRepositoryRef()->GetEntity(entityId_)->GetDataType();
}

EntityComponentRegion::EntityComponentRegion(EntityInspectorArea* _parent)
    : Editor::Region(nameof<EntityComponentRegion>()), parentArea_(_parent) {}
EntityComponentRegion::~EntityComponentRegion() {}
void EntityComponentRegion::Initialize() {}
void EntityComponentRegion::DrawGui() {
    auto currentScene    = parentArea_->GetParentWindow()->GetCurrentScene();
    int32_t editEntityId = parentArea_->GetEditEntityId();
    auto editEntity      = currentScene->GetEntityRepositoryRef()->GetEntity(editEntityId);

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
                selectArea->SetTargets({editEntityId});
                selectArea->SetOpen(true);
            } else {
                LOG_ERROR("Failed to cast SelectAddComponentArea.");
            }
        } else {
            LOG_ERROR("SelectAddComponentArea not found in parent window.");
        }
    }

    ::ImGui::Spacing();

    Scene* editScene         = parentArea_->GetParentWindow()->GetCurrentScene();
    auto& entityComponentMap = parentArea_->GetEntityComponentMap();
    for (const auto& [componentTypeName, components] : entityComponentMap) {
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
                        component->Edit(editScene, editEntity, label);
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
                component->Edit(editScene, editEntity, label);
            }
            ::ImGui::Unindent();
        }
    }

    ::ImGui::Unindent();
}
void EntityComponentRegion::Finalize() {}

EntityComponentRegion::RemoveComponentFromEditListCommand::RemoveComponentFromEditListCommand(EntityInspectorArea* _parentArea, const ::std::string& _componentTypeName, int32_t _compIndex)
    : parentArea_(_parentArea), componentTypeName_(_componentTypeName), componentIndex_(_compIndex) {
    if (!parentArea_) {
        LOG_ERROR("RemoveComponentFromEditListCommand: parentArea is null.");
        return;
    }
    // 現在のコンポーネントデータを保存
    auto* scene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!scene) {
        LOG_ERROR("RemoveComponentFromEditListCommand: Scene is null.");
        return;
    }
    const auto& compArray = scene->GetComponentArray(componentTypeName_);
    if (compArray) {
        Entity* editEntity = scene->GetEntity(parentArea_->GetEditEntityId());
        if (editEntity) {
            compArray->SaveComponent(editEntity, componentIndex_, componentData_);
        } else {
            LOG_ERROR("RemoveComponentFromEditListCommand: Edit entity is null.");
        }
    } else {
        LOG_ERROR("RemoveComponentFromEditListCommand: Component array '{}' not found.", componentTypeName_);
    }
}
void EntityComponentRegion::RemoveComponentFromEditListCommand::Execute() {
    if (!parentArea_) {
        LOG_ERROR("RemoveComponentFromEditListCommand: parentArea is null.");
        return;
    }
    auto* scene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!scene) {
        LOG_ERROR("RemoveComponentFromEditListCommand: Scene is null.");
        return;
    }
    scene->RemoveComponent(componentTypeName_, parentArea_->GetEditEntityId(), componentIndex_);

    // コンポーネントを削除した後、エンティティのコンポーネントマップからも削除
    auto& entityComponentMap = parentArea_->GetEntityComponentMap();
    auto it                  = entityComponentMap.find(componentTypeName_);
    if (it != entityComponentMap.end()) {
        auto& components = it->second;
        if (componentIndex_ < static_cast<int32_t>(components.size())) {
            components.erase(components.begin() + componentIndex_);
        } else {
            LOG_ERROR("RemoveComponentFromEditListCommand: Component index out of range for type '{}'.", componentTypeName_);
        }
    } else {
        LOG_ERROR("RemoveComponentFromEditListCommand: Component type '{}' not found in entity component map.", componentTypeName_);
    }
}

void EntityComponentRegion::RemoveComponentFromEditListCommand::Undo() {
    if (!parentArea_) {
        LOG_ERROR("RemoveComponentFromEditListCommand: parentArea is null.");
        return;
    }
    auto* scene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!scene) {
        LOG_ERROR("RemoveComponentFromEditListCommand: Scene is null.");
        return;
    }
    Entity* editEntity = scene->GetEntity(parentArea_->GetEditEntityId());
    if (editEntity) {
        auto componentArray = scene->GetComponentArray(componentTypeName_);
        if (!componentArray) {
            LOG_ERROR("RemoveComponentFromEditListCommand: Component array '{}' not found.", componentTypeName_);
            return;
        }

        componentArray->InsertComponent(editEntity, componentIndex_);
        componentArray->LoadComponent(editEntity, componentIndex_, componentData_[componentTypeName_]);

        // コンポーネントをエンティティのコンポーネントマップに再追加
        auto& entityComponentMap = parentArea_->GetEntityComponentMap();
        auto it                  = entityComponentMap.find(componentTypeName_);
        if (it != entityComponentMap.end()) {
            auto& components = it->second;
            if (componentIndex_ < static_cast<int32_t>(components.size())) {
                components.insert(components.begin() + componentIndex_, componentArray->GetComponent(editEntity, componentIndex_));
            } else {
                LOG_ERROR("RemoveComponentFromEditListCommand: Component index out of range for type '{}'.", componentTypeName_);
            }
        } else {
            LOG_ERROR("RemoveComponentFromEditListCommand: Component type '{}' not found in entity component map.", componentTypeName_);
        }

    } else {
        LOG_ERROR("RemoveComponentFromEditListCommand: Edit entity is null.");
    }
}

EntitySystemRegion::EntitySystemRegion(EntityInspectorArea* _parent)
    : Editor::Region(nameof<EntitySystemRegion>()), parentArea_(_parent) {}
EntitySystemRegion::~EntitySystemRegion() {}
void EntitySystemRegion::Initialize() {}
void EntitySystemRegion::DrawGui() {
    auto currentScene    = parentArea_->GetParentWindow()->GetCurrentScene();
    int32_t editEntityId = parentArea_->GetEditEntityId();
    auto editEntity      = currentScene->GetEntityRepositoryRef()->GetEntity(editEntityId);

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
                ::std::list<int32_t> targets = {editEntityId};
                auto SetTargetsCommand       = ::std::make_unique<SelectAddSystemArea::SetTargeEntities>(selectArea, targets);
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
                        ::std::list<int32_t> editEntityIds = {editEntityId};
                        auto command                       = ::std::make_unique<RemoveSystemCommand>(editEntityIds, systemName, system->GetCategory());
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
    targetEntityIds_.clear();
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
    targetEntityIds_.clear(); // 対象のエンティティIDリストをクリア
}

void SelectAddComponentArea::SetTargets(const ::std::list<int32_t>& _targets) {
    targetEntityIds_ = _targets;
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

    auto& componentRegistryMap = ComponentRegistry::GetInstance()->GetComponentArrayMap();

    // ImGuiのスタイルで選択色を設定（必要に応じてアプリ全体で設定してもOK）
    ImVec4 winSelectColor = ImVec4(0.26f, 0.59f, 0.98f, 1.0f); // Windows風の青
    ::ImGui::PushStyleColor(ImGuiCol_Header, winSelectColor);
    ::ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.8f));
    ::ImGui::PushStyleColor(ImGuiCol_HeaderActive, winSelectColor);
    for (auto& [name, array] : componentRegistryMap) {

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
                    auto clearCommand = ::std::make_unique<ClearComponentTypeNames>(parentArea_);
                    OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearCommand));
                }
                // まだ選択されていなければ追加
                auto command = ::std::make_unique<AddComponentTypeNames>(parentArea_, name);
                OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
            } else {
                // すでに選択されていれば解除
                auto command = ::std::make_unique<RemoveComponentTypeNames>(parentArea_, name);
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
                auto addCompCommand = ::std::make_unique<AddComponentCommand>(parentArea_->targetEntityIds_, compTypeName);
                OriGine::EditorController::GetInstance()->PushCommand(::std::move(addCompCommand));
            }

            auto ClearEntitiesCommand = ::std::make_unique<ClearTargetEntities>(parentArea_);
            OriGine::EditorController::GetInstance()->PushCommand(::std::move(ClearEntitiesCommand));

            auto clearComponentNamesCommand = ::std::make_unique<ClearComponentTypeNames>(parentArea_);
            OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearComponentNamesCommand));

            parentArea_->isOpen_.Set(false); // エリアを閉じる
            parentArea_->isFocused_.Set(false); // フォーカスを外す
        }
    }

    ::ImGui::SameLine();

    if (::ImGui::Button("CANCEL")) {
        // キャンセルボタンが押された場合、選択をクリア
        auto ClearEntitiesCommand = ::std::make_unique<ClearTargetEntities>(parentArea_);
        OriGine::EditorController::GetInstance()->PushCommand(::std::move(ClearEntitiesCommand));

        auto clearComponentNamesCommand = ::std::make_unique<ClearComponentTypeNames>(parentArea_);
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
    targetEntityIds_.clear(); // 対象のエンティティIDリストをクリア
}
void SelectAddSystemArea::SetTargets(const ::std::list<int32_t>& _targets) {
    targetEntityIds_ = _targets;
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

    ImVec4 winSelectColor = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
    ::ImGui::PushStyleColor(ImGuiCol_Header, winSelectColor);
    ::ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.8f));
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
                    OriGine::ISystem* system = currentScene->GetSystem(name);

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
                                auto clearCommand = ::std::make_unique<ClearSystemNames>(parentArea_);
                                OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearCommand));
                            }
                            auto command = ::std::make_unique<AddSystemNames>(parentArea_, name);
                            OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
                        } else {
                            auto command = ::std::make_unique<RemoveSystemNames>(parentArea_, name);
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
                OriGine::ISystem* system = currentScene->GetSystem(name);

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
                            auto clearCommand = ::std::make_unique<ClearSystemNames>(parentArea_);
                            OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearCommand));
                        }
                        auto command = ::std::make_unique<AddSystemNames>(parentArea_, name);
                        OriGine::EditorController::GetInstance()->PushCommand(::std::move(command));
                    } else {
                        auto command = ::std::make_unique<RemoveSystemNames>(parentArea_, name);
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
            auto addSystemCommand = ::std::make_unique<AddSystemsForTargetEntities>(parentArea_, parentArea_->targetEntityIds_, parentArea_->systemTypeNames_);
            OriGine::EditorController::GetInstance()->PushCommand(::std::move(addSystemCommand));
            auto ClearEntitiesCommand = ::std::make_unique<ClearTargetEntities>(parentArea_);
            OriGine::EditorController::GetInstance()->PushCommand(::std::move(ClearEntitiesCommand));
            auto clearSystemNamesCommand = ::std::make_unique<ClearSystemNames>(parentArea_);
            OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearSystemNamesCommand));
            parentArea_->isOpen_.Set(false);
            parentArea_->isFocused_.Set(false);
        }
    }
    ::ImGui::SameLine();
    if (::ImGui::Button("CANCEL")) {
        auto ClearEntitiesCommand = ::std::make_unique<ClearTargetEntities>(parentArea_);
        OriGine::EditorController::GetInstance()->PushCommand(::std::move(ClearEntitiesCommand));
        auto clearSystemNamesCommand = ::std::make_unique<ClearSystemNames>(parentArea_);
        OriGine::EditorController::GetInstance()->PushCommand(::std::move(clearSystemNamesCommand));
        parentArea_->isOpen_.Set(false);
        parentArea_->isFocused_.Set(false);
    }
}
void SelectAddSystemArea::SystemListRegion::Finalize() {}

#pragma endregion

EntityInspectorArea::ChangeEditEntityCommand::ChangeEditEntityCommand(EntityInspectorArea* _inspectorArea, int32_t _to, int32_t _from)
    : inspectorArea_(_inspectorArea), toId_(_to), fromId_(_from) {
    Scene* currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();

    if (!currentScene) {
        return;
    }

    SceneFactory factory;
    if (toId_ > kInvalidEntityID) {
        Entity* toEntity = currentScene->GetEntityRepositoryRef()->GetEntity(toId_);
        toEntityData_    = factory.CreateEntityJsonFromEntity(currentScene, toEntity);
    }
    if (fromId_ > kInvalidEntityID) {
        Entity* toEntity = currentScene->GetEntityRepositoryRef()->GetEntity(fromId_);
        fromEntityData_  = factory.CreateEntityJsonFromEntity(currentScene, toEntity);
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
    Scene* currentScene             = inspectorArea_->GetParentWindow()->GetCurrentScene();
    Entity* toEntity                = currentScene->GetEntityRepositoryRef()->GetEntity(toId_);
    inspectorArea_->editEntityName_ = toEntity->GetDataType();

    if (toEntityData_.empty()) {
        LOG_DEBUG("ChangeEditEntityCommand::Execute: toEntityData is empty, skipping entity change.");
        return;
    }

    /// 必要なら 読み込み (InspectorArea の entityComponentMap と systemMapに対して)
    auto& componentMap = currentScene->GetComponentRepositoryRef()->GetComponentArrayMap();

    /// Components の読み込み
    auto& entityCompData = toEntityData_.at("Components");
    for (auto& compData : entityCompData.items()) {
        const ::std::string& compTypeName = compData.key();
        auto compArray                    = componentMap.find(compTypeName);
        if (compArray == componentMap.end()) {
            LOG_ERROR("ChangeEditEntityCommand::Execute: Component type '{}' not found in component map.", compTypeName);
            continue;
        }
        IComponentArray* compArrayPtr = compArray->second.get();
        for (uint32_t i = 0; i < static_cast<uint32_t>((compArrayPtr->GetComponentSize(toEntity))); ++i) {
            auto component = compArrayPtr->GetComponent(toEntity, i);
            if (!component) {
                LOG_ERROR("ChangeEditEntityCommand::Execute: Component '{}' not found for entity ID '{}'.", compTypeName, toId_);
                continue;
            }
            inspectorArea_->entityComponentMap_[compTypeName].emplace_back(component);
        }
    }

    /// Systems の読み込み
    auto& entitySystemData = toEntityData_.at("Systems");
    auto& systemMap        = currentScene->GetSystemRunnerRef()->GetSystems();
    for (auto& systemData : entitySystemData.items()) {
        const ::std::string& systemName = systemData.value().at("SystemName");
        auto systemItr                  = systemMap.find(systemName);
        if (systemItr == systemMap.end()) {
            LOG_ERROR("ChangeEditEntityCommand::Execute: System '{}' not found .", systemName);
            continue;
        }
        OriGine::ISystem* system = systemItr->second.get();
        if (!system) {
            LOG_ERROR("ChangeEditEntityCommand::Execute: System '{}' not found for entity ID '{}'.", systemName, toId_);
            continue;
        }

        inspectorArea_->systemMap_[int32_t(system->GetCategory())][systemName] = system;
    }
}

void EntityInspectorArea::ChangeEditEntityCommand::Undo() {
    inspectorArea_->editEntityId_ = fromId_;
    inspectorArea_->entityComponentMap_.clear();
    inspectorArea_->systemMap_.fill({});

    if (fromId_ < 0) {
        return;
    }

    Scene* currentScene             = inspectorArea_->GetParentWindow()->GetCurrentScene();
    Entity* frontEntity             = currentScene->GetEntityRepositoryRef()->GetEntity(fromId_);
    inspectorArea_->editEntityName_ = frontEntity->GetDataType();

    if (fromEntityData_.empty()) {
        LOG_DEBUG("ChangeEditEntityCommand::Undo: fromEntityData is empty, skipping entity change.");
        return;
    }

    /// 必要なら 読み込み (InspectorArea の entityComponentMap と systemMapに対して)
    auto& componentMap = currentScene->GetComponentRepositoryRef()->GetComponentArrayMap();

    /// Components の読み込み
    auto& entityCompData = fromEntityData_.at("Components");
    for (auto& compData : entityCompData.items()) {
        const ::std::string& compTypeName = compData.key();
        auto compArray                    = componentMap.find(compTypeName);
        if (compArray == componentMap.end()) {
            LOG_ERROR("ChangeEditEntityCommand::Execute: Component type '{}' not found in component map.", compTypeName);
            continue;
        }
        IComponentArray* compArrayPtr = compArray->second.get();
        for (uint32_t i = 0; i < static_cast<uint32_t>(compArrayPtr->GetComponentSize(frontEntity)); ++i) {
            auto component = compArrayPtr->GetComponent(frontEntity, i);
            if (!component) {
                LOG_ERROR("ChangeEditEntityCommand::Execute: Component '{}' not found for entity ID '{}'.", compTypeName, toId_);
                continue;
            }
            inspectorArea_->entityComponentMap_[compTypeName].emplace_back(component);
        }
    }

    /// Systems の読み込み
    auto& entitySystemData = fromEntityData_.at("Systems");
    auto& systemMap        = currentScene->GetSystemRunnerRef()->GetSystems();
    for (auto& systemData : entitySystemData.items()) {
        const ::std::string& systemName = systemData.value().at("SystemName");
        auto systemItr                  = systemMap.find(systemName);
        if (systemItr == systemMap.end()) {
            LOG_ERROR("ChangeEditEntityCommand::Execute: System '{}' not found .", systemName);
            continue;
        }
        OriGine::ISystem* system = systemItr->second.get();
        if (!system) {
            LOG_ERROR("ChangeEditEntityCommand::Execute: System '{}' not found for entity ID '{}'.", systemName, toId_);
            continue;
        }
        inspectorArea_->systemMap_[int32_t(system->GetCategory())][systemName] = system;
    }
}

void EntityInformationRegion::ChangeEntityUniqueness::Execute() {
    auto currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();
    Entity* entity    = currentScene->GetEntityRepositoryRef()->GetEntity(entityId_);

    if (!entity) {
        LOG_ERROR("ChangeEntityUniqueness::Execute: Entity with ID '{}' not found.", entityId_);
        return;
    }

    if (newValue_) {
        currentScene->GetEntityRepositoryRef()->RegisterUniqueEntity(entity);
    } else {
        currentScene->GetEntityRepositoryRef()->UnregisterUniqueEntity(entity->GetDataType());
    }
}

void EntityInformationRegion::ChangeEntityUniqueness::Undo() {
    auto currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();
    Entity* entity    = currentScene->GetEntityRepositoryRef()->GetEntity(entityId_);

    if (!entity) {
        LOG_ERROR("ChangeEntityUniqueness::Execute: Entity with ID '{}' not found.", entityId_);
        return;
    }

    if (oldValue_) {
        currentScene->GetEntityRepositoryRef()->RegisterUniqueEntity(entity);
    } else {
        currentScene->GetEntityRepositoryRef()->UnregisterUniqueEntity(entity->GetDataType());
    }
}

void EntityInformationRegion::ChangeEntityShouldSave::Execute() {
    auto currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();
    Entity* entity    = currentScene->GetEntityRepositoryRef()->GetEntity(entityId_);

    if (!entity) {
        LOG_ERROR("Entity with ID '{}' not found.", entityId_);
        return;
    }

    entity->SetShouldSave(newValue_);
}

void EntityInformationRegion::ChangeEntityShouldSave::Undo() {
    auto currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();
    Entity* entity    = currentScene->GetEntityRepositoryRef()->GetEntity(entityId_);

    if (!entity) {
        LOG_ERROR("Entity with ID '{}' not found.", entityId_);
        return;
    }

    entity->SetShouldSave(oldValue_);
}

void EntityInformationRegion::ChangeEntityName::Execute() {
    auto currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();
    Entity* entity    = currentScene->GetEntityRepositoryRef()->GetEntity(entityId_);
    if (!entity) {
        LOG_ERROR("ChangeEntityName::Execute: Entity with ID '{}' not found.", entityId_);
        return;
    }
    entity->SetDataType(newName_);
    // UniqueEntity の場合は、名前変更後に再登録
    if (entity->IsUnique()) {
        currentScene->GetEntityRepositoryRef()->UnregisterUniqueEntity(oldName_);
        currentScene->GetEntityRepositoryRef()->RegisterUniqueEntity(entity);
    }
}

void EntityInformationRegion::ChangeEntityName::Undo() {
    auto currentScene = inspectorArea_->GetParentWindow()->GetCurrentScene();
    Entity* entity    = currentScene->GetEntityRepositoryRef()->GetEntity(entityId_);
    if (!entity) {
        LOG_ERROR("ChangeEntityName::Undo: Entity with ID '{}' not found.", entityId_);
        return;
    }
    entity->SetDataType(oldName_);
    // UniqueEntity の場合は、名前変更後に再登録
    if (entity->IsUnique()) {
        currentScene->GetEntityRepositoryRef()->UnregisterUniqueEntity(newName_);
        currentScene->GetEntityRepositoryRef()->RegisterUniqueEntity(entity);
    }
}

void SelectAddComponentArea::AddComponentTypeNames::Execute() {
    parentArea_->componentTypeNames_.push_back(componentTypeName_);
}

void SelectAddComponentArea::AddComponentTypeNames::Undo() {
    auto& componentTypeNames = parentArea_->componentTypeNames_;
    auto it                  = ::std::remove(componentTypeNames.begin(), componentTypeNames.end(), componentTypeName_);
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
    auto it                  = ::std::remove(componentTypeNames.begin(), componentTypeNames.end(), componentTypeName_);
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

void SelectAddSystemArea::AddSystemNames::Execute() {
    parentArea_->systemTypeNames_.push_back(systemTypeName_);
    LOG_DEBUG("SelectAddSystemArea::AddSystemNames::Execute: Added system type name '{}'.", systemTypeName_);
}

void SelectAddSystemArea::AddSystemNames::Undo() {
    ::std::string popName = parentArea_->systemTypeNames_.back();
    parentArea_->systemTypeNames_.pop_back();
    LOG_DEBUG("SelectAddSystemArea::AddSystemNames::Undo: Removed system type name '{}'.", popName);
}

void SelectAddSystemArea::RemoveSystemNames::Execute() {
    auto& systemTypeNames = parentArea_->systemTypeNames_;
    auto it               = ::std::remove(systemTypeNames.begin(), systemTypeNames.end(), systemTypeName_);
    if (it != systemTypeNames.end()) {
        systemTypeNames.erase(it, systemTypeNames.end());
        LOG_DEBUG("SelectAddSystemArea::RemoveSystemNames::Execute: Removed system type name '{}'.", systemTypeName_);
    } else {
        LOG_DEBUG("SelectAddSystemArea::RemoveSystemNames::Execute: System type name '{}' not found.", systemTypeName_);
    }
}

void SelectAddSystemArea::RemoveSystemNames::Undo() {
    parentArea_->systemTypeNames_.push_back(systemTypeName_);
    LOG_DEBUG("SelectAddSystemArea::RemoveSystemNames::Undo: Added system type name '{}' back.", systemTypeName_);
}

void SelectAddSystemArea::ClearSystemNames::Execute() {
    systemTypeNames_ = parentArea_->systemTypeNames_; // 現在のシステムタイプ名を保存
    parentArea_->systemTypeNames_.clear(); // システムタイプ名をクリア
    LOG_DEBUG("SelectAddSystemArea::ClearSystemNames::Execute: Cleared system type names.");
}

void SelectAddSystemArea::ClearSystemNames::Undo() {
    parentArea_->systemTypeNames_ = systemTypeNames_; // 保存したシステムタイプ名を復元
    LOG_DEBUG("SelectAddSystemArea::ClearSystemNames::Undo: Restored system type names.");
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
    auto currentScene = window->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("AddSystemsForTargetEntities::Execute: No current scene found.");
        return;
    }
    auto* entityInspectorArea = dynamic_cast<EntityInspectorArea*>(window->GetArea(nameof<EntityInspectorArea>()).get()); // 選択状態をクリア
    if (!entityInspectorArea) {
        LOG_ERROR("entityInspectorArea not found in SceneEditorWindow.");
        return;
    }
    int32_t editEntityId = entityInspectorArea->GetEditEntityId();
    for (const auto& entityId : parentArea_->targetEntityIds_) {
        Entity* entity = currentScene->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with ID '{}' not found.", entityId);
            continue;
        }

        if (editEntityId == entityId) {
            for (const auto& systemTypeName : parentArea_->systemTypeNames_) {
                currentScene->GetSystemRunnerRef()->RegisterEntity(systemTypeName, entity);

                OriGine::ISystem* system                                                            = currentScene->GetSystemRunnerRef()->GetSystem(systemTypeName);
                entityInspectorArea->GetSystemMap()[int32_t(system->GetCategory())][systemTypeName] = system;
            }
        } else {
            for (const auto& systemTypeName : parentArea_->systemTypeNames_) {
                currentScene->GetSystemRunnerRef()->RegisterEntity(systemTypeName, entity);
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
    auto currentScene = window->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("AddSystemsForTargetEntities::Execute: No current scene found.");
        return;
    }
    auto* entityInspectorArea = dynamic_cast<EntityInspectorArea*>(window->GetArea(nameof<EntityInspectorArea>()).get()); // 選択状態をクリア
    if (!entityInspectorArea) {
        LOG_ERROR("entityInspectorArea not found in SceneEditorWindow.");
        return;
    }
    int32_t editEntityId = entityInspectorArea->GetEditEntityId();
    for (const auto& entityId : parentArea_->targetEntityIds_) {
        Entity* entity = currentScene->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with ID '{}' not found.", entityId);
            continue;
        }

        if (editEntityId == entityId) {
            for (const auto& systemTypeName : parentArea_->systemTypeNames_) {
                currentScene->GetSystemRunnerRef()->RemoveEntity(systemTypeName, entity);
                OriGine::ISystem* system = currentScene->GetSystemRunnerRef()->GetSystem(systemTypeName);
                auto& systems            = entityInspectorArea->GetSystemMap()[int32_t(system->GetCategory())];
                auto itr                 = systems.find(systemTypeName);
                if (itr != systems.end()) {
                    systems.erase(itr);
                } else {
                    LOG_ERROR("System '{}' not found in EntityInspectorArea's system map.", systemTypeName);
                }
            }
        } else {
            for (const auto& systemTypeName : parentArea_->systemTypeNames_) {
                currentScene->GetSystemRunnerRef()->RemoveEntity(systemTypeName, entity);
            }
        }
    }
}

EntityInformationRegion::DeleteEntityCommand::DeleteEntityCommand(EntityInspectorArea* _parentArea, int32_t _entityId)
    : parentArea_(_parentArea), entityId_(_entityId) {}

void EntityInformationRegion::DeleteEntityCommand::Execute() {
    auto currentScene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("DeleteEntityCommand::Execute: No current scene found.");
        return;
    }
    Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityId_);
    if (!entity) {
        LOG_ERROR("DeleteEntityCommand::Execute: Entity with ID '{}' not found.", entityId_);
        return;
    }

    SceneFactory factory;

    entityData_ = factory.CreateEntityJsonFromEntity(currentScene, entity);
    currentScene->DeleteEntity(entityId_);
    LOG_DEBUG("DeleteEntityCommand::Execute: Deleted entity with ID '{}'.", entityId_);
}

void EntityInformationRegion::DeleteEntityCommand::Undo() {
    auto currentScene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("DeleteEntityCommand::Undo: No current scene found.");
        return;
    }

    SceneFactory factory;
    // エンティティを復元
    Entity* entity = factory.BuildEntity(currentScene, entityData_);
    entityId_      = entity->GetID(); // 復元後のエンティティIDを更新
    // 編集対象エンティティを復元したエンティティに変更
    EntityInspectorArea::ChangeEditEntityCommand changeEditEntity = EntityInspectorArea::ChangeEditEntityCommand(parentArea_, entityId_, parentArea_->GetEditEntityId());
    changeEditEntity.Execute();

    if (!entity) {
        LOG_ERROR("DeleteEntityCommand::Undo: Failed to restore entity with ID '{}'.", entityId_);
        return;
    }
    LOG_DEBUG("DeleteEntityCommand::Undo: Restored entity with ID '{}'.", entityId_);
}

RemoveComponentForEntityCommand::RemoveComponentForEntityCommand(Scene* _scene, const ::std::string& _componentTypeName, int32_t _entityId, int32_t _compIndex)
    : scene_(_scene), componentTypeName_(_componentTypeName), entityId_(_entityId), compIndex_(_compIndex) {
    if (!scene_) {
        LOG_ERROR("RemoveComponentForEntityCommand: Scene is null.");
        return;
    }

    if (entityId_ > kInvalidEntityID) {
        Entity* entity = scene_->GetEntityRepositoryRef()->GetEntity(entityId_);
        if (!entity) {
            LOG_ERROR("RemoveComponentForEntityCommand: Entity with ID '{}' not found.", entityId_);
            return;
        }
        const auto& compArray = scene_->GetComponentArray(componentTypeName_);
        compArray->SaveComponent(entity, compIndex_, componentData_);
    }
}

void RemoveComponentForEntityCommand::Execute() {
    if (!scene_) {
        LOG_ERROR("RemoveComponentForEntityCommand::Execute: Scene is null.");
        return;
    }
    Entity* entity = scene_->GetEntityRepositoryRef()->GetEntity(entityId_);
    if (!entity) {
        LOG_ERROR("RemoveComponentForEntityCommand::Execute: Entity with ID '{}' not found.", entityId_);
        return;
    }
    const auto& compArray = scene_->GetComponentArray(componentTypeName_);
    if (!compArray) {
        LOG_ERROR("RemoveComponentForEntityCommand::Execute: Component array '{}' not found.", componentTypeName_);
        return;
    }
    compArray->RemoveComponent(entity, compIndex_);
}

void RemoveComponentForEntityCommand::Undo() {
    if (!scene_) {
        LOG_ERROR("RemoveComponentForEntityCommand::Undo: Scene is null.");
        return;
    }
    Entity* entity = scene_->GetEntityRepositoryRef()->GetEntity(entityId_);
    if (!entity) {
        LOG_ERROR("RemoveComponentForEntityCommand::Undo: Entity with ID '{}' not found.", entityId_);
        return;
    }
    const auto& compArray = scene_->GetComponentArray(componentTypeName_);
    if (!compArray) {
        LOG_ERROR("RemoveComponentForEntityCommand::Undo: Component array '{}' not found.", componentTypeName_);
        return;
    }
    compArray->InsertComponent(entity, compIndex_);
    compArray->LoadComponent(entity, compIndex_, componentData_);
}

#endif // _DEBUG
