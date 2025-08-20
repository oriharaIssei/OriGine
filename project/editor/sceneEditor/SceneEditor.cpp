#include "SceneEditor.h"

#ifdef _DEBUG

/// engine
#include "Engine.h"
#include "scene/SceneManager.h"
#include "winApp/WinApp.h"

#define RESOURCE_DIRECTORY
#include "EngineInclude.h"

/// ecs
#include "ECS/Entity.h"
// component
#include "component/ComponentArray.h"
#include "component/IComponent.h"
#include "component/transform/Transform.h"
// system
#include "system/ISystem.h"

// directX12
#include "directX12/RenderTexture.h"
// camera
#include "camera/CameraManager.h"
#include "camera/debugCamera/DebugCamera.h"

/// editor
#include "editor/EditorController.h"
#include "editor/sceneEditor/EntityInspector.h"
#include "editor/sceneEditor/SystemInspector.h"

#include "myFileSystem/MyFileSystem.h"
#include <myGui/MyGui.h>
/// externals
#include <imgui/ImGuizmo/ImGuizmo.h>

static const std::string sceneFolderPath = kApplicationResourceDirectory + "/scene";

void SceneEditorWindow::Initialize() {
    InitializeScene();

    InitializeMenus();

    InitializeAreas();

    isMaximized_ = true; // 初期状態で最大化
}

void SceneEditorWindow::Finalize() {

    FinalizeScene();

    FinalizeMenus();
    FinalizeAreas();

    GlobalVariables::getInstance()->SaveFile("Settings", "SceneEditor");
}

void SceneEditorWindow::InitializeMenus() {
    // メニューの初期化
    auto fileMenu = std::make_unique<FileMenu>(this);
    addMenu(std::move(fileMenu));
}

void SceneEditorWindow::InitializeAreas() {
    // エリアの初期化
    addArea(std::make_unique<SceneViewArea>(this));
    addArea(std::make_unique<HierarchyArea>(this));
    addArea(std::make_unique<EntityInspectorArea>(this));
    addArea(std::make_unique<SelectAddComponentArea>(this));
    addArea(std::make_unique<SelectAddSystemArea>(this));
    addArea(std::make_unique<SystemInspectorArea>(this));
    addArea(std::make_unique<DevelopControlArea>(this));
}

void SceneEditorWindow::InitializeScene(const std::string& _sceneName) {
    currentScene_ = std::make_unique<Scene>(_sceneName);
    currentScene_->Initialize();
}

void SceneEditorWindow::InitializeScene() {
    InitializeScene(editSceneName_);
}

void SceneEditorWindow::FinalizeMenus() {
    if (menus_.empty()) {
        return;
    }
    // メニューの終了処理
    for (auto& [name, menu] : menus_) {
        menu->Finalize();
    }
    menus_.clear();
}

void SceneEditorWindow::FinalizeAreas() {
    if (areas_.empty()) {
        return;
    }
    // エリアの終了処理
    for (auto& [name, area] : areas_) {
        area->Finalize();
    }
    areas_.clear();
}

void SceneEditorWindow::FinalizeScene() {
    if (!currentScene_) {
        return;
    }
    currentScene_->Finalize();
}

#pragma region "Menus"
FileMenu::FileMenu(SceneEditorWindow* _parentWindow)
    : parentWindow_(_parentWindow), Editor::Menu("File") {}
FileMenu::~FileMenu() {}
void FileMenu::Initialize() {
    addMenuItem(std::make_unique<SaveMenuItem>(this));
    addMenuItem(std::make_unique<LoadMenuItem>(this));
    addMenuItem(std::make_unique<CreateMenuItem>(this));
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

void SaveMenuItem::Initialize() {}

void SaveMenuItem::DrawGui() {
    bool isSelect = false;

    if (ImGui::MenuItem(name_.c_str(), "ctl + s", &isSelect)) {
        auto* currentScene         = parentMenu_->getParentWindow()->getCurrentScene();
        SceneSerializer serializer = SceneSerializer(currentScene);
        LOG_DEBUG("SaveMenuItem : Saving scene '{}'.", currentScene->getName());
        serializer.Serialize();
    }

    isSelected_.set(isSelect);
}

void SaveMenuItem::Finalize() {}

LoadMenuItem::LoadMenuItem(FileMenu* _parent)
    : Editor::MenuItem("Load"), parentMenu_(_parent) {}

LoadMenuItem::~LoadMenuItem() {}

void LoadMenuItem::Initialize() {}

void LoadMenuItem::DrawGui() {
    bool isSelect = false;
    if (ImGui::MenuItem(name_.c_str(), "ctl + o", &isSelect)) {
        // シーンのロード処理
        std::string directory, filename;
        if (!myfs::selectFileDialog(sceneFolderPath, directory, filename, {"json"}, true)) {
            return;
        }

        SceneEditorWindow* sceneEditorWindow = EditorController::getInstance()->getWindow<SceneEditorWindow>();
        SceneSerializer serializer           = SceneSerializer(sceneEditorWindow->getCurrentScene());
        serializer.Serialize();
        sceneEditorWindow->getCurrentScene()->Finalize(); // 現在のシーンを終了処理

        sceneEditorWindow->getEditSceneName().setValue(filename);

        std::unique_ptr<Scene> scene = std::make_unique<Scene>(filename);
        scene->Initialize();

        LOG_DEBUG("LoadMenuItem : Loading scene '{}'.", scene->getName());

        sceneEditorWindow->changeScene(std::move(scene));

        sceneEditorWindow->FinalizeAreas();
        sceneEditorWindow->InitializeAreas();

        EditorController::getInstance()->clearCommandHistory();
    }
    isSelected_.set(isSelect);
}

void LoadMenuItem::Finalize() {
    loadScene_ = nullptr; // ロードするシーンへのポインタをクリア
}

CreateMenuItem::CreateMenuItem(FileMenu* _parent)
    : Editor::MenuItem(nameof<CreateMenuItem>()), parentMenu_(_parent) {}

CreateMenuItem::~CreateMenuItem() {}

void CreateMenuItem::Initialize() {}

void CreateMenuItem::DrawGui() {
    bool isSelect = false;
    if (ImGui::BeginMenu("Create NewScene")) {
        ImGui::InputText("New Scene Name", &newSceneName_[0], sizeof(char) * 256);
        if (ImGui::Button("Create")) {
            auto scenes = myfs::searchFile(SceneSerializer::SceneDirectory, {"json"});

            auto it = std::find_if(
                scenes.begin(), scenes.end(),
                [this](const std::pair<std::string, std::string>& scene) {
                    return scene.second == newSceneName_;
                });

            if (it != scenes.end()) {
                LOG_ERROR("Scene with name '{}' already exists.", newSceneName_);
                newSceneName_ = "";
                ImGui::OpenPopup("Scene Exists");
            } else {
                auto scene = parentMenu_->getParentWindow()->getCurrentScene();

                SceneSerializer serializer = SceneSerializer(scene);
                serializer.Serialize();

                auto newScene = std::make_unique<Scene>(newSceneName_);
                newScene->Initialize();
                parentMenu_->getParentWindow()->changeScene(std::move(newScene));

                // 初期化
                EditorController::getInstance()->clearCommandHistory();
                newSceneName_ = "";
            }
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginPopup("Scene Exists")) {
        ImGui::Text("Scene with this name already exists.");
        ImGui::Separator();
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    isSelected_.set(isSelect);
}

void CreateMenuItem::Finalize() {}

#pragma endregion

#pragma region "SceneViewArea"

SceneViewArea::SceneViewArea(SceneEditorWindow* _parentWindow)
    : parentWindow_(_parentWindow), Editor::Area(nameof<SceneViewArea>()) {}

void SceneViewArea::Initialize() {
    // DebugCameraの初期化
    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Initialize();
}

void SceneViewArea::DrawGui() {
    bool isOpen = isOpen_.current();

    if (!isOpen) {
        isOpen_.set(isOpen);
        isFocused_.set(ImGui::IsWindowFocused());
        UpdateFocusAndOpenState();
        return;
    }

    auto renderTexture = parentWindow_->getCurrentScene()->getSceneView();

    if (ImGui::Begin(name_.c_str(), &isOpen)) {

        areaSize_ = ImGui::GetContentRegionAvail();

        if (areaSize_[X] >= 1.f && areaSize_[Y] >= 1.f && renderTexture->getTextureSize() != areaSize_) {
            renderTexture->Resize(areaSize_);

            float aspectRatio                                 = areaSize_[X] / areaSize_[Y];
            debugCamera_->getCameraTransformRef().aspectRatio = aspectRatio;
            debugCamera_->Update();
        }

        if (isFocused_.current()) {
            debugCamera_->Update();
        }

        DrawScene();

        ImGui::Image(reinterpret_cast<ImTextureID>(renderTexture->getBackBufferSrvHandle().ptr), areaSize_.toImVec2());

        ImVec2 imageLeftTop = ImGui::GetCursorScreenPos();
        UseImGuizmo(imageLeftTop, renderTexture->getTextureSize());

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

    cameraManager->setTransform(debugCamera_->getCameraTransform());
    cameraManager->DataConvertToBuffer();
    auto* currentScene = parentWindow_->getCurrentScene();

    // effect systemの更新
    currentScene->getSystemRunnerRef()->UpdateCategory(SystemCategory::Effect);
    // 描画
    currentScene->Render();
    cameraManager->setTransform(prevTransform);
}

static Vec2f ConvertMouseToSceneView(const Vec2f& mousePos, const ImVec2& sceneViewPos, const ImVec2& sceneViewSize, const Vec2f& originalResolution) {
    // SceneView 内での相対的なマウス座標を計算
    float relativeX = mousePos[X] - sceneViewPos.x;
    float relativeY = mousePos[Y] - sceneViewPos.y;

    // SceneView のスケールを計算
    float scaleX = originalResolution[X] / sceneViewSize.x;
    float scaleY = originalResolution[Y] / sceneViewSize.y;

    // ゲーム内の座標に変換
    Vec2f gamePos;
    gamePos[X] = relativeX * scaleX;
    gamePos[Y] = relativeY * scaleY;

    return gamePos;
}

void SceneViewArea::UseImGuizmo(const ImVec2& _sceneViewPos, const Vec2f& _originalResolution) {
    // マウス座標を取得
    Vec2f mousePos = Input::getInstance()->getCurrentMousePos();

    // マウス座標をゲーム内の座標に変換
    Vec2f gamePos = ConvertMouseToSceneView(mousePos, _sceneViewPos, areaSize_.toImVec2(), _originalResolution);
    Input::getInstance()->setVirtualMousePos(gamePos);

    // ImGuizmo のフレーム開始
    ImGuizmo::BeginFrame();

    // ImGuizmo の設定
    ImGuizmo::SetOrthographic(false); // 透視投影かどうか
    ImGuizmo::SetDrawlist();

    // ImGuizmo のウィンドウサイズ・位置を設定
    ImGuizmo::SetRect(_sceneViewPos.x, _sceneViewPos.y, areaSize_[X], areaSize_[Y]);

    Vec2f virtualMousePos = Input::getInstance()->getVirtualMousePos();

    auto* currentScene       = parentWindow_->getCurrentScene();
    auto entityInspectorArea = dynamic_cast<EntityInspectorArea*>(parentWindow_->getArea("EntityInspectorArea").get());
    if (!entityInspectorArea) {
        LOG_ERROR("EntityInspectorArea not found in SceneEditorWindow.");
        return;
    }

    GameEntity* editEntity = currentScene->getEntity(entityInspectorArea->getEditEntityId());
    if (!editEntity) {
        return;
    }

    auto transformArray = currentScene->getComponentArray<Transform>();

    // Transformを持っていないエンティティは Skip
    if (!transformArray->hasEntity(editEntity)) {
        return;
    }

    Transform* transform = currentScene->getComponent<Transform>(editEntity);
    if (!transform) {
        return;
    }
    /// ==========================================
    // Guizmo Edit

    // 行列の用意
    float matrix[16];
    transform->worldMat.toFloatArray(matrix); // Transform の worldMat を float[16] に変換
    // ビュー行列とプロジェクション行列の取得
    float viewMatrix[16];
    float projectionMatrix[16];
    debugCamera_->getCameraTransform().viewMat.toFloatArray(viewMatrix); // カメラのビュー行列を取得
    debugCamera_->getCameraTransform().projectionMat.toFloatArray(projectionMatrix); // カメラのプロジェクション行列を取得

    // ギズモの操作タイプ
    static ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE | ImGuizmo::SCALE | ImGuizmo::ROTATE;

    [](ImGuizmo::OPERATION& _currentGizmoOperation) {
        Input* input = Input::getInstance();
        if (input->isPressKey(Key::L_SHIFT)) {
            if (input->isPressKey(Key::S)) {
                if (input->isPressKey(Key::X)) {
                    _currentGizmoOperation = ImGuizmo::SCALE_X;
                } else if (input->isPressKey(Key::Y)) {
                    _currentGizmoOperation = ImGuizmo::SCALE_Y;
                } else if (input->isPressKey(Key::Z)) {
                    _currentGizmoOperation = ImGuizmo::SCALE_Z;
                } else {
                    _currentGizmoOperation = ImGuizmo::SCALE; // Shift + S でスケール
                }
            } else if (input->isPressKey(Key::R)) {
                if (input->isPressKey(Key::X)) {
                    _currentGizmoOperation = ImGuizmo::ROTATE_X;
                } else if (input->isPressKey(Key::Y)) {
                    _currentGizmoOperation = ImGuizmo::ROTATE_Y;
                } else if (input->isPressKey(Key::Z)) {
                    _currentGizmoOperation = ImGuizmo::ROTATE_Z;
                } else {
                    _currentGizmoOperation = ImGuizmo::ROTATE; // Shift + R で回転
                }
            } else if (input->isPressKey(Key::T)) {
                if (input->isPressKey(Key::X)) {
                    _currentGizmoOperation = ImGuizmo::TRANSLATE_X;
                } else if (input->isPressKey(Key::Y)) {
                    _currentGizmoOperation = ImGuizmo::TRANSLATE_Y;
                } else if (input->isPressKey(Key::Z)) {
                    _currentGizmoOperation = ImGuizmo::TRANSLATE_Z;
                } else {
                    _currentGizmoOperation = ImGuizmo::TRANSLATE; // Shift + T で移動
                }
            }

        } else {
            _currentGizmoOperation = ImGuizmo::TRANSLATE | ImGuizmo::SCALE | ImGuizmo::ROTATE;
        }
    }(currentGizmoOperation);

    // ギズモの描画・操作
    if (ImGuizmo::Manipulate(
            viewMatrix, // カメラのビュー行列(float[16])
            projectionMatrix, // カメラのプロジェクション行列(float[16])
            currentGizmoOperation,
            ImGuizmo::LOCAL, // ローカル or ワールド
            matrix)) {

        // matrix から this->translate, this->rotate, this->scale を分解して反映
        // 例: DecomposeMatrixToComponents(matrix, this->translate, this->rotate, this->scale);
        transform->worldMat.fromFloatArray(matrix);

        transform->worldMat.decomposeMatrixToComponents(transform->scale, transform->rotate, transform->translate);
    }

    /// ==========================================
    // Editor Command
    static bool wasUsingGuizmo = false;
    bool isUsingGuizmo         = ImGuizmo::IsUsing();
    static GuiValuePool<Vec3f> vec3fPool;
    static GuiValuePool<Quaternion> quatPool;

    // Guizmo Trigger
    if (isUsingGuizmo) {
        // ImGuizmoが使用中ならば、他の操作は無効化
        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        if (!wasUsingGuizmo) {
            vec3fPool.setValue(editEntity->getUniqueID() + "Scale", transform->scale);
            quatPool.setValue(editEntity->getUniqueID() + "Rotation", transform->rotate);
            vec3fPool.setValue(editEntity->getUniqueID() + "Translate", transform->translate);
        }
    } else {
        // ImGuizmoが使用されていない場合は、通常のマウスカーソルに戻す
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        if (wasUsingGuizmo) {
            auto commandCombo = std::make_unique<CommandCombo>();

            /// S,R,T を コマンドで更新するように
            commandCombo->addCommand(std::make_unique<SetterCommand<Vec3f>>(&transform->scale, transform->scale, vec3fPool.popValue(editEntity->getUniqueID() + "Scale")));
            commandCombo->addCommand(std::make_unique<SetterCommand<Quaternion>>(&transform->rotate, transform->rotate, quatPool.popValue(editEntity->getUniqueID() + "Rotation")));
            commandCombo->addCommand(std::make_unique<SetterCommand<Vec3f>>(&transform->translate, transform->translate, vec3fPool.popValue(editEntity->getUniqueID() + "Translate")));

            commandCombo->setFuncOnAfterCommand(
                [transform]() {
                    if (!transform) {
                        return;
                    }
                    transform->Update();
                },
                true);

            // push
            EditorController::getInstance()->pushCommand(std::move(commandCombo));
        }
    }

    wasUsingGuizmo = isUsingGuizmo;
}

void SceneViewArea::Finalize() {
    if (debugCamera_) {
        debugCamera_.reset();
    }
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
    if (ImGui::Button("+ EntityFromFile")) {
        // 選択されているエンティティを削除
        SceneSerializer serializer(currentScene);
        std::string directory, filename;
        if (!myfs::selectFileDialog(kApplicationResourceDirectory + "/entities", directory, filename, {"json"}, true)) {
            return; // キャンセルされた場合は何もしない
        }
        // ファイルからエンティティを読み込む
        auto command = std::make_unique<LoadEntityCommand>(parentArea_, kApplicationResourceDirectory + "/entities" + directory, filename);
        EditorController::getInstance()->pushCommand(std::move(command));
    }

    // ImGuiのスタイルで選択色を設定（必要に応じてアプリ全体で設定してもOK）
    ImVec4 winSelectColor = ImVec4(0.26f, 0.59f, 0.98f, 1.0f); // Windows風の青
    ImGui::PushStyleColor(ImGuiCol_Header, winSelectColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, winSelectColor);

    // 選択状態のエンティティIDを取得
    for (const auto& entity : entityRepository) {
        if (!entity.isAlive()) {
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

EntityHierarchy::CreateEntityCommand::CreateEntityCommand(HierarchyArea* _parentArea, const std::string& _entityName) {
    parentArea_ = _parentArea;
    entityName_ = _entityName;
}
void EntityHierarchy::CreateEntityCommand::Execute() {
    auto currentScene = parentArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("CreateEntityCommand::Execute: No current scene found.");
        return;
    }

    entityId_ = currentScene->getEntityRepositoryRef()->CreateEntity(entityName_);

    LOG_DEBUG("CreateEntityCommand::Execute: Created entity with ID '{}'.", entityId_);
}
void EntityHierarchy::CreateEntityCommand::Undo() {
    auto currentScene = parentArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("CreateEntityCommand::Undo: No current scene found.");
        return;
    }

    currentScene->deleteEntity(entityId_);

    LOG_DEBUG("CreateEntityCommand::Undo: Removed entity with ID '{}'.", entityId_);
}

EntityHierarchy::LoadEntityCommand::LoadEntityCommand(HierarchyArea* _parentArea, const std::string& _directory, const std::string& _entityName) {
    parentArea_ = _parentArea;
    directory_  = _directory;
    entityName_ = _entityName;
}
void EntityHierarchy::LoadEntityCommand::Execute() {
    auto currentScene = parentArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("No current scene found.");
        return;
    }

    SceneSerializer serializer(currentScene);
    GameEntity* createdEntity = serializer.LoadEntity(directory_, entityName_);
    entityId_                 = createdEntity->getID();

    LOG_DEBUG("Created entity with ID '{}'.", entityId_);
}
void EntityHierarchy::LoadEntityCommand::Undo() {
    auto currentScene = parentArea_->getParentWindow()->getCurrentScene();
    if (!currentScene) {
        LOG_ERROR("No current scene found.");
        return;
    }

    currentScene->deleteEntity(entityId_);

    LOG_DEBUG("Removed entity with ID '{}'.", entityId_);
}

void AddComponentCommand::Execute() {
    auto sceneEditorWindow = EditorController::getInstance()->getWindow<SceneEditorWindow>();
    auto currentScene      = sceneEditorWindow->getCurrentScene();
    auto inspectorArea     = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->getArea("EntityInspectorArea").get());
    int32_t editEntityId   = inspectorArea->getEditEntityId();

    for (auto entityId : entityIds_) {
        GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with ID '{}' not found.", entityId);
            return;
        }

        // コンポーネントの追加
        IComponentArray* compArray = currentScene->getComponentRepositoryRef()->getComponentArray(componentTypeName_);
        compArray->addComponent(entity);
        if (!compArray) {
            LOG_ERROR("Failed to add component '{}'. \n ", componentTypeName_);
            return;
        }

        // コンポーネントをマップに追加
        if (entity->getID() == editEntityId) {
            inspectorArea->getEntityComponentMap()[componentTypeName_].emplace_back(compArray->getBackComponent(entity));
        }
    }
}

void AddComponentCommand::Undo() {
    auto sceneEditorWindow = EditorController::getInstance()->getWindow<SceneEditorWindow>();
    auto currentScene      = sceneEditorWindow->getCurrentScene();
    auto inspectorArea     = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->getArea("EntityInspectorArea").get());
    int32_t editEntityId   = inspectorArea->getEditEntityId();

    for (auto entityId : entityIds_) {
        GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with ID '{}' not found.", entityId);
            return;
        }
        // コンポーネントの削除
        IComponentArray* compArray = currentScene->getComponentRepositoryRef()->getComponentArray(componentTypeName_);
        if (!compArray) {
            LOG_ERROR("ComponentArray '{}' not found.", componentTypeName_);
            return;
        }
        compArray->removeComponent(entity, compArray->getComponentSize(entity) - 1);

        if (entityId == editEntityId) {
            // コンポーネントをマップから削除
            auto& components = inspectorArea->getEntityComponentMap()[componentTypeName_];
            if (!components.empty()) {
                components.pop_back(); // 最後のコンポーネントを削除
            }
        }
    }
}

void RemoveComponentCommand::Execute() {
    auto sceneEditorWindow = EditorController::getInstance()->getWindow<SceneEditorWindow>();
    auto currentScene      = sceneEditorWindow->getCurrentScene();
    auto inspectorArea     = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->getArea("EntityInspectorArea").get());
    int32_t editEntityId   = inspectorArea->getEditEntityId();

    GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(entityId_);
    if (!entity) {
        LOG_ERROR("Entity with ID '{}' not found.", entityId_);
        return;
    }
    // コンポーネントの削除
    IComponentArray* compArray = currentScene->getComponentRepositoryRef()->getComponentArray(componentTypeName_);
    if (!compArray) {
        LOG_ERROR("ComponentArray '{}' not found.", componentTypeName_);
        return;
    }
    compArray->removeComponent(entity, componentIndex_);

    if (entityId_ == editEntityId) {
        // コンポーネントをマップから削除
        auto& components = inspectorArea->getEntityComponentMap()[componentTypeName_];
        if (!components.empty()) {
            components.pop_back(); // 最後のコンポーネントを削除
        }
    }
}

void RemoveComponentCommand::Undo() {
    auto sceneEditorWindow = EditorController::getInstance()->getWindow<SceneEditorWindow>();
    auto currentScene      = sceneEditorWindow->getCurrentScene();
    auto inspectorArea     = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->getArea("EntityInspectorArea").get());
    int32_t editEntityId   = inspectorArea->getEditEntityId();

    GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(entityId_);
    if (!entity) {
        LOG_ERROR("Entity with ID '{}' not found.", entityId_);
        return;
    }

    // コンポーネントの追加
    IComponentArray* compArray = currentScene->getComponentRepositoryRef()->getComponentArray(componentTypeName_);
    compArray->addComponent(entity);
    if (!compArray) {
        LOG_ERROR("Failed to add component '{}'. \n ", componentTypeName_);
        return;
    }

    // コンポーネントをマップに追加
    if (entityId_ == editEntityId) {
        inspectorArea->getEntityComponentMap()[componentTypeName_].emplace_back(compArray->getBackComponent(entity));
    }
}

AddSystemCommand::AddSystemCommand(const std::list<int32_t>& _entityIds, const std::string& _systemTypeName, SystemCategory _category)
    : entityIds_(_entityIds), systemTypeName_(_systemTypeName), systemCategory_(_category) {};

void AddSystemCommand::Execute() {
    auto sceneEditorWindow = EditorController::getInstance()->getWindow<SceneEditorWindow>();
    auto currentScene      = sceneEditorWindow->getCurrentScene();
    auto inspectorArea     = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->getArea("EntityInspectorArea").get());
    int32_t editEntityId   = inspectorArea->getEditEntityId();
    if (!currentScene) {
        LOG_ERROR("AddSystemCommand::Execute: No current scene found.");
        return;
    }
    for (auto entityId : entityIds_) {
        GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(entityId);
        if (!entity) {
            LOG_ERROR("AddSystemCommand::Execute: Entity with ID '{}' not found.", entityId);
            continue;
        }
        currentScene->getSystemRunnerRef()->registerEntity(systemTypeName_, entity);

        if (editEntityId == entityId) {
            inspectorArea->getSystemMap()[int32_t(systemCategory_)][systemTypeName_] = currentScene->getSystemRunnerRef()->getSystem(systemTypeName_);
        }
    }
}

void AddSystemCommand::Undo() {
    auto sceneEditorWindow = EditorController::getInstance()->getWindow<SceneEditorWindow>();
    auto currentScene      = sceneEditorWindow->getCurrentScene();
    auto inspectorArea     = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->getArea("EntityInspectorArea").get());
    int32_t editEntityId   = inspectorArea->getEditEntityId();
    if (!currentScene) {
        LOG_ERROR("AddSystemCommand::Execute: No current scene found.");
        return;
    }
    for (auto entityId : entityIds_) {
        GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(entityId);
        if (!entity) {
            LOG_ERROR("AddSystemCommand::Execute: Entity with ID '{}' not found.", entityId);
            continue;
        }
        currentScene->getSystemRunnerRef()->removeEntity(systemTypeName_, entity);

        if (editEntityId == entityId) {
            auto itr = inspectorArea->getSystemMap()[int32_t(systemCategory_)].find(systemTypeName_);
            inspectorArea->getSystemMap()[int32_t(systemCategory_)].erase(itr);
        }
    }
}

RemoveSystemCommand::RemoveSystemCommand(const std::list<int32_t>& _entityIds, const std::string& _systemTypeName, SystemCategory _category)
    : entityIds_(_entityIds), systemTypeName_(_systemTypeName), systemCategory_(_category) {}

void RemoveSystemCommand::Execute() {
    auto sceneEditorWindow = EditorController::getInstance()->getWindow<SceneEditorWindow>();
    auto currentScene      = sceneEditorWindow->getCurrentScene();
    auto inspectorArea     = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->getArea("EntityInspectorArea").get());
    int32_t editEntityId   = inspectorArea->getEditEntityId();

    if (!currentScene) {
        LOG_ERROR("RemoveSystemCommand::Execute: No current scene found.");
        return;
    }
    for (auto entityId : entityIds_) {
        GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(entityId);
        if (!entity) {
            LOG_ERROR("RemoveSystemCommand::Execute: Entity with ID '{}' not found.", entityId);
            continue;
        }
        currentScene->getSystemRunnerRef()->removeEntity(systemTypeName_, entity);
        if (editEntityId == entityId) {
            auto itr = inspectorArea->getSystemMap()[int32_t(systemCategory_)].find(systemTypeName_);
            inspectorArea->getSystemMap()[int32_t(systemCategory_)].erase(itr);
        }
    }
}

void RemoveSystemCommand::Undo() {
    auto sceneEditorWindow = EditorController::getInstance()->getWindow<SceneEditorWindow>();
    auto currentScene      = sceneEditorWindow->getCurrentScene();
    auto inspectorArea     = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->getArea("EntityInspectorArea").get());
    int32_t editEntityId   = inspectorArea->getEditEntityId();
    if (!currentScene) {
        LOG_ERROR("RemoveSystemCommand::Execute: No current scene found.");
        return;
    }
    for (auto entityId : entityIds_) {
        GameEntity* entity = currentScene->getEntityRepositoryRef()->getEntity(entityId);
        if (!entity) {
            LOG_ERROR("RemoveSystemCommand::Execute: Entity with ID '{}' not found.", entityId);
            continue;
        }
        currentScene->getSystemRunnerRef()->registerEntity(systemTypeName_, entity);
        if (editEntityId == entityId) {
            inspectorArea->getSystemMap()[int32_t(systemCategory_)][systemTypeName_] = currentScene->getSystemRunnerRef()->getSystem(systemTypeName_);
        }
    }
}

#pragma region "DevelopControlArea"
DevelopControlArea::DevelopControlArea(SceneEditorWindow* _parentWindow)
    : Editor::Area(nameof<DevelopControlArea>()), parentWindow_(_parentWindow) {}
DevelopControlArea::~DevelopControlArea() {}

void DevelopControlArea::Initialize() {
    addRegion(std::make_shared<ControlRegion>(this));
}

DevelopControlArea::ControlRegion::ControlRegion(DevelopControlArea* _parentArea)
    : Editor::Region(nameof<ControlRegion>()), parentArea_(_parentArea) {}

DevelopControlArea::ControlRegion::~ControlRegion() {}

void DevelopControlArea::ControlRegion::Initialize() {}

void DevelopControlArea::ControlRegion::DrawGui() {
    if (ImGui::Button("Build Develop")) {
        auto* currentScene = parentArea_->getParentWindow()->getCurrentScene();
        if (!currentScene) {
            LOG_ERROR("ControlRegion::DrawGui: No current scene found.");
            return;
        }
        /// ==========================================
        // Build
        /// ==========================================
        // ビルドコマンドの作成
        std::string buildCommand =
            parentArea_->buildTool_ + " "
            + parentArea_->projectName_
            + " /p:Configuration=" + parentArea_->configuration
            + " /p:Platform=" + parentArea_->platform;
        LOG_DEBUG("ControlRegion::DrawGui: Executing build command: {}", buildCommand);

        // ビルドコマンドの実行
        if (!RunProcessAndWait(buildCommand)) {
            LOG_ERROR("ControlRegion::DrawGui: Build command execution failed.");
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Run")) {
        auto* currentScene  = parentArea_->getParentWindow()->getCurrentScene();
        std::string exePath = std::filesystem::current_path().string() + parentArea_->exePath_;
        LOG_DEBUG("ControlRegion::DrawGui: Executing application at path: {}", exePath);

        std::string runCommand = exePath + " " + currentScene->getName(); // 実行ファイルパスと 実行する scene を送る
        // アプリケーションの実行
        int32_t result = std::system(runCommand.c_str());
        if (result != 0) {
            LOG_ERROR("ControlRegion::DrawGui: Failed to run application. Error code: {}", result);
        } else {
            LOG_DEBUG("ControlRegion::DrawGui: Application executed successfully.");
        }
    }
}

void DevelopControlArea::ControlRegion::Finalize() {}

#endif
