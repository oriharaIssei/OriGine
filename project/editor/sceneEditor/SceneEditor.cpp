#include "SceneEditor.h"

#ifdef _DEBUG

/// stl
#include <format>

/// engine
#include "Engine.h"
#include "input/InputManager.h"

#include "scene/SceneFactory.h"
#include "scene/SceneJsonRegistry.h"
#include "scene/SceneManager.h"

#include "winApp/WinApp.h"

#define RESOURCE_DIRECTORY
#include "EngineInclude.h"

/// ecs
#include "entity/Entity.h"
// component
#include "component/ComponentArray.h"
#include "component/IComponent.h"
#include "component/transform/Transform.h"
// system
#include "system/ISystem.h"
#include "system/SystemRunner.h"

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

using namespace OriGine;

static const std::string sceneFolderPath = kApplicationResourceDirectory + "/" + kSceneJsonFolder;

void SceneEditorWindow::Initialize() {
    InitializeScene();

    InitializeMenus();

    InitializeAreas();

    isMaximized_ = true; // 初期状態で最大化
}

void SceneEditorWindow::DrawGui() {
    if (isSceneChanged_) {
        LoadNextScene();
        isSceneChanged_ = false;
    } else {
        currentScene_->ExecuteDeleteEntities();
        Editor::Window::DrawGui();
    }
}
void SceneEditorWindow::LoadNextScene() {
    if (nextSceneName_.empty()) {
        LOG_ERROR("Next scene name is empty.");
        return;
    }
    FinalizeScene();
    FinalizeMenus();
    FinalizeAreas();

    OriGine::EditorController::GetInstance()->ClearCommandHistory();

    InitializeScene(nextSceneName_);
    InitializeMenus();
    InitializeAreas();

    editSceneName_.SetValue(currentScene_->GetName());
    GlobalVariables::GetInstance()->SaveFile("Settings", "SceneEditor");

    nextSceneName_ = "";
}

void SceneEditorWindow::Finalize() {
    editSceneName_.SetValue(currentScene_->GetName());

    FinalizeScene();

    FinalizeMenus();
    FinalizeAreas();

    GlobalVariables::GetInstance()->SaveFile("Settings", "SceneEditor");
}

void SceneEditorWindow::InitializeMenus() {
    // メニューの初期化
    auto fileMenu = std::make_unique<FileMenu>(this);
    AddMenu(std::move(fileMenu));
}

void SceneEditorWindow::InitializeAreas() {
    // エリアの初期化
    AddArea(std::make_unique<SceneViewArea>(this));
    AddArea(std::make_unique<HierarchyArea>(this));
    AddArea(std::make_unique<EntityInspectorArea>(this));
    AddArea(std::make_unique<SelectAddComponentArea>(this));
    AddArea(std::make_unique<SelectAddSystemArea>(this));
    AddArea(std::make_unique<SystemInspectorArea>(this));
    AddArea(std::make_unique<DevelopControlArea>(this));
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
    AddMenuItem(std::make_unique<SaveMenuItem>(this));
    AddMenuItem(std::make_unique<LoadMenuItem>(this));
    AddMenuItem(std::make_unique<CreateMenuItem>(this));
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
        auto* currentScene = parentMenu_->GetParentWindow()->GetCurrentScene();
        LOG_DEBUG("SaveMenuItem : Saving scene '{}'.", currentScene->GetName());
        SceneJsonRegistry::GetInstance()->SaveScene(currentScene, sceneFolderPath);
    }

    isSelected_.Set(isSelect);
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
        if (!myfs::SelectFileDialog(sceneFolderPath, directory, filename, {"json"}, true)) {
            return;
        }

        SceneEditorWindow* sceneEditorWindow = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
        sceneEditorWindow->ChangeScene(filename);
    }
    isSelected_.Set(isSelect);
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
        ImGui::InputText("New Scene Name", &newSceneName_);

        if (ImGui::Button("Create")) {
            auto scenes = myfs::SearchFile(kApplicationResourceDirectory + "/" + kSceneJsonFolder, {"json"});

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
                auto currentScene = parentMenu_->GetParentWindow()->GetCurrentScene();

                SceneJsonRegistry::GetInstance()->SaveScene(currentScene, sceneFolderPath);

                auto newScene = std::make_unique<Scene>(newSceneName_);
                newScene->Initialize();
                parentMenu_->GetParentWindow()->ChangeScene(newSceneName_);

                // 初期化
                OriGine::EditorController::GetInstance()->ClearCommandHistory();
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

    isSelected_.Set(isSelect);
}

void CreateMenuItem::Finalize() {}

#pragma endregion

#pragma region "SceneViewArea"

SceneViewArea::SceneViewArea(SceneEditorWindow* _parentWindow)
    : parentWindow_(_parentWindow), Editor::Area(nameof<SceneViewArea>()) {}

void SceneViewArea::Initialize() {
    // DebugCameraの初期化
    debugCamera_ = std::make_unique<OriGine::DebugCamera>();
    debugCamera_->Initialize();
}

void SceneViewArea::DrawGui() {
    bool isOpen = isOpen_.Current();

    if (!isOpen) {
        isOpen_.Set(isOpen);
        isFocused_.Set(ImGui::IsWindowFocused());
        UpdateFocusAndOpenState();
        return;
    }

    auto renderTexture = parentWindow_->GetCurrentScene()->GetSceneView();

    if (ImGui::Begin(name_.c_str(), &isOpen)) {

        areaSize_ = ImGui::GetContentRegionAvail();

        if (areaSize_[X] >= 1.f && areaSize_[Y] >= 1.f && renderTexture->GetTextureSize() != areaSize_) {
            renderTexture->Resize(areaSize_);

            float aspectRatio                                 = areaSize_[X] / areaSize_[Y];
            debugCamera_->GetCameraTransformRef().aspectRatio = aspectRatio;
            debugCamera_->Update();
        }

        if (isFocused_.Current()) {
            debugCamera_->Update();
        }

        DrawScene();

        ImGui::Image(reinterpret_cast<ImTextureID>(renderTexture->GetBackBufferSrvHandle().ptr), areaSize_.toImVec2());

        ImVec2 imageLeftTop  = ImGui::GetItemRectMin();
        ImVec2 imageRightBot = ImGui::GetItemRectMax();
        Vec2f imageSize      = {imageRightBot.x - imageLeftTop.x,
                 imageRightBot.y - imageLeftTop.y};

        UseImGuizmo(imageLeftTop, imageSize);

        for (auto& [name, region] : regions_) {
            if (!region) {
                continue;
            }
            ImGui::BeginGroup();
            region->DrawGui();
            ImGui::EndGroup();
        }
    }

    isOpen_.Set(isOpen);
    isFocused_.Set(ImGui::IsWindowFocused());
    UpdateFocusAndOpenState();

    ImGui::End();
}
void SceneViewArea::DrawScene() {
    CameraManager* cameraManager  = CameraManager::GetInstance();
    CameraTransform prevTransform = cameraManager->GetTransform();

    cameraManager->SetTransform(debugCamera_->GetCameraTransform());
    cameraManager->DataConvertToBuffer();
    auto* currentScene = parentWindow_->GetCurrentScene();

    // effect systemの更新
    currentScene->GetSystemRunnerRef()->UpdateCategory(SystemCategory::Effect);
    // 描画
    currentScene->Render();
    cameraManager->SetTransform(prevTransform);
}

void SceneViewArea::UseImGuizmo(const ImVec2& _sceneViewPos, const Vec2f& _originalResolution) {
    // マウス座標を取得
    Vec2f mousePos = InputManager::GetInstance()->GetMouse()->GetPosition();

    // マウス座標をゲーム内の座標に変換
    MouseInput* mouseInput = InputManager::GetInstance()->GetMouse();
    Vec2f gamePos          = ConvertMouseToSceneView(mousePos, _sceneViewPos, areaSize_.toImVec2(), _originalResolution);
    mouseInput->SetVirtualPosition(gamePos);

    // ImGuizmo のフレーム開始
    ImGuizmo::BeginFrame();

    // ImGuizmo の設定
    ImGuizmo::SetOrthographic(false); // 透視投影かどうか
    ImGuizmo::SetDrawlist();

    // ImGuizmo のウィンドウサイズ・位置を設定
    ImGuizmo::SetRect(_sceneViewPos.x, _sceneViewPos.y, areaSize_[X], areaSize_[Y]);

    Vec2f virtualMousePos = mouseInput->GetVirtualPosition();

    auto* currentScene       = parentWindow_->GetCurrentScene();
    auto entityInspectorArea = dynamic_cast<EntityInspectorArea*>(parentWindow_->GetArea("EntityInspectorArea").get());
    if (!entityInspectorArea) {
        LOG_ERROR("EntityInspectorArea not found in SceneEditorWindow.");
        return;
    }

    EntityHandle editEntityHandle = entityInspectorArea->GetEditEntityHandle();
    Entity* editEntity            = currentScene->GetEntity(editEntityHandle);
    if (!editEntity) {
        return;
    }

    auto transformArray = currentScene->GetComponentArray<Transform>();

    // Transformを持っていないエンティティは Skip
    if (!transformArray->HasEntity(editEntityHandle)) {
        return;
    }

    Transform* transform = currentScene->GetComponent<Transform>(editEntityHandle);
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
    debugCamera_->GetCameraTransform().viewMat.toFloatArray(viewMatrix); // カメラのビュー行列を取得
    debugCamera_->GetCameraTransform().projectionMat.toFloatArray(projectionMatrix); // カメラのプロジェクション行列を取得

    // ギズモの操作タイプ
    static ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE | ImGuizmo::SCALE | ImGuizmo::ROTATE;

    [](ImGuizmo::OPERATION& _currentGizmoOperation) {
        InputManager* input     = InputManager::GetInstance();
        KeyboardInput* keyboard = input->GetKeyboard();
        if (keyboard->IsPress(Key::L_SHIFT)) {
            if (keyboard->IsPress(Key::S)) {
                if (keyboard->IsPress(Key::X)) {
                    _currentGizmoOperation = ImGuizmo::SCALE_X;
                } else if (keyboard->IsPress(Key::Y)) {
                    _currentGizmoOperation = ImGuizmo::SCALE_Y;
                } else if (keyboard->IsPress(Key::Z)) {
                    _currentGizmoOperation = ImGuizmo::SCALE_Z;
                } else {
                    _currentGizmoOperation = ImGuizmo::SCALE; // Shift + S でスケール
                }
            } else if (keyboard->IsPress(Key::R)) {
                if (keyboard->IsPress(Key::X)) {
                    _currentGizmoOperation = ImGuizmo::ROTATE_X;
                } else if (keyboard->IsPress(Key::Y)) {
                    _currentGizmoOperation = ImGuizmo::ROTATE_Y;
                } else if (keyboard->IsPress(Key::Z)) {
                    _currentGizmoOperation = ImGuizmo::ROTATE_Z;
                } else {
                    _currentGizmoOperation = ImGuizmo::ROTATE; // Shift + R で回転
                }
            } else if (keyboard->IsPress(Key::T)) {
                if (keyboard->IsPress(Key::X)) {
                    _currentGizmoOperation = ImGuizmo::TRANSLATE_X;
                } else if (keyboard->IsPress(Key::Y)) {
                    _currentGizmoOperation = ImGuizmo::TRANSLATE_Y;
                } else if (keyboard->IsPress(Key::Z)) {
                    _currentGizmoOperation = ImGuizmo::TRANSLATE_Z;
                } else {
                    _currentGizmoOperation = ImGuizmo::TRANSLATE; // Shift + T で移動
                }
            }

        } else {
            _currentGizmoOperation = ImGuizmo::TRANSLATE | ImGuizmo::SCALE | ImGuizmo::ROTATE;
        }
    }(currentGizmoOperation);

    if (ImGuizmo::Manipulate(
            viewMatrix,
            projectionMatrix,
            currentGizmoOperation,
            ImGuizmo::LOCAL,
            matrix)) {

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
            vec3fPool.SetValue(editEntity->GetUniqueID() + "Scale", transform->scale);
            quatPool.SetValue(editEntity->GetUniqueID() + "Rotation", transform->rotate);
            vec3fPool.SetValue(editEntity->GetUniqueID() + "Translate", transform->translate);
        }
    } else {
        // ImGuizmoが使用されていない場合は、通常のマウスカーソルに戻す
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        if (wasUsingGuizmo) {
            auto commandCombo = std::make_unique<CommandCombo>();

            /// S,R,T を コマンドで更新するように
            commandCombo->AddCommand(std::make_unique<SetterCommand<Vec3f>>(&transform->scale, transform->scale, vec3fPool.popValue(editEntity->GetUniqueID() + "Scale")));
            commandCombo->AddCommand(std::make_unique<SetterCommand<Quaternion>>(&transform->rotate, transform->rotate, quatPool.popValue(editEntity->GetUniqueID() + "Rotation")));
            commandCombo->AddCommand(std::make_unique<SetterCommand<Vec3f>>(&transform->translate, transform->translate, vec3fPool.popValue(editEntity->GetUniqueID() + "Translate")));

            commandCombo->SetFuncOnAfterCommand(
                [transform]() {
                    if (!transform) {
                        return;
                    }
                    transform->UpdateMatrix();
                },
                true);

            // push
            OriGine::EditorController::GetInstance()->PushCommand(std::move(commandCombo));
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
    AddRegion(std::make_shared<EntityHierarchy>(this));
}

void HierarchyArea::Finalize() {
    Editor::Area::Finalize();
}

EntityHierarchy::EntityHierarchy(HierarchyArea* _parent) : Editor::Region(nameof<EntityHierarchy>()), parentArea_(_parent) {}
EntityHierarchy::~EntityHierarchy() {}

void EntityHierarchy::Initialize() {}

void EntityHierarchy::DrawGui() {
    ImGui::SeparatorText("Entity Hierarchy");
    auto currentScene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!currentScene) {
        ImGui::SeparatorText("No current scene found.");
        return;
    }
    auto& entityRepository = currentScene->GetEntityRepositoryRef()->GetEntities();
    if (entityRepository.empty()) {
        ImGui::SeparatorText("No entities in the current scene.");
        return;
    }

    // Entity の作成,削除
    if (ImGui::Button("+ Entity")) {
        // 新しいエンティティを作成
        //! TODO : 初期Entity名を設定できるように
        auto command = std::make_unique<CreateEntityCommand>(parentArea_, "Entity");
        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
    }
    if (ImGui::Button("+ EntityFromFile")) {
        std::string directory, filename;
        if (!myfs::SelectFileDialog(kApplicationResourceDirectory + "/entities", directory, filename, {"ent"}, true)) {
            return; // キャンセルされた場合は何もしない
        }
        // ファイルからエンティティを読み込む
        auto command = std::make_unique<LoadEntityCommand>(parentArea_, kApplicationResourceDirectory + "/entities" + directory, filename);
        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
    }

    const auto& keyboardInput = InputManager::GetInstance()->GetKeyboard();

    /// コピー & ペースト
    if (parentArea_->IsFocused().Current()) {
        if (keyboardInput->IsPress(Key::L_CTRL) || keyboardInput->IsPress(Key::R_CTRL)) {
            // コピー
            if (keyboardInput->IsTrigger(Key::C)) {
                auto command = std::make_unique<CopyEntityCommand>(this);
                OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
            }
            // ペースト
            if (keyboardInput->IsTrigger(Key::V)) {
                auto command = std::make_unique<PasteEntityCommand>(this);
                OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
            }
        }
    }

    ImGui::InputText("Search", &searchBuff_, ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::SeparatorText("Entities");

    // ImGuiのスタイルで選択色を設定（必要に応じてアプリ全体で設定してもOK）
    ImVec4 winSelectColor       = ImVec4(0.26f, 0.59f, 0.98f, 1.0f); // Windows風の青
    ImVec4 winSelectHeaderColor = ImVec4(0.26f, 0.59f, 0.98f, 0.8f);
    ImGui::PushStyleColor(ImGuiCol_Header, winSelectColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, winSelectHeaderColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, winSelectColor);

    // 選択状態のエンティティIDを取得
    if (searchBuff_.empty()) {
        for (const auto& entity : entityRepository) {
            if (!entity.IsAlive()) {
                continue; // 無効なエンティティはスキップ
            }

            EntityHandle entityHandle = entity.GetHandle();
            std::string uniqueId      = entity.GetUniqueID();

            // 選択状態か判定
            bool isSelected = std::find(selectedEntityHandles_.begin(), selectedEntityHandles_.end(), entityHandle) != selectedEntityHandles_.end();

            // Selectableで表示
            if (ImGui::Selectable(uniqueId.c_str(), isSelected)) {
                // Shiftキーで複数選択、そうでなければ単一選択
                if (ImGui::GetIO().KeyShift) {
                    if (!isSelected) {
                        // まだ選択されていなければ追加
                        auto command = std::make_unique<AddSelectedEntitiesCommand>(this, entityHandle);
                        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
                    } else {
                        // すでに選択されていれば解除
                        auto command = std::make_unique<RemoveSelectedEntitiesCommand>(this, entityHandle);
                        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
                    }
                } else {
                    // Shiftキーが押されていない場合は選択をクリアしてから追加
                    auto clearCommand = std::make_unique<ClearSelectedEntitiesCommand>(this);
                    OriGine::EditorController::GetInstance()->PushCommand(std::move(clearCommand));
                    auto addCommand = std::make_unique<AddSelectedEntitiesCommand>(this, entityHandle);
                    OriGine::EditorController::GetInstance()->PushCommand(std::move(addCommand));

                    auto& parentWindowHasAreas  = parentArea_->GetParentWindow()->GetAreas();
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
                    auto changedEditEntity = std::make_unique<EntityInspectorArea::ChangeEditEntityCommand>(entityInspectorArea, entityHandle, entityInspectorArea->GetEditEntityHandle());
                    OriGine::EditorController::GetInstance()->PushCommand(std::move(changedEditEntity));
                }
            }
        }
    } else {
        for (const auto& entity : entityRepository) {
            if (!entity.IsAlive()) {
                continue; // 無効なエンティティはスキップ
            }
            if (entity.GetUniqueID().find(searchBuff_) == std::string::npos) {
                continue; // 検索文字列にマッチしないエンティティはスキップ
            }

            EntityHandle entityHandle = entity.GetHandle();
            std::string uniqueId      = entity.GetUniqueID();

            // 選択状態か判定
            bool isSelected = std::find(selectedEntityHandles_.begin(), selectedEntityHandles_.end(), entityHandle) != selectedEntityHandles_.end();

            // Selectableで表示
            if (ImGui::Selectable(uniqueId.c_str(), isSelected)) {
                // Shiftキーで複数選択、そうでなければ単一選択
                if (ImGui::GetIO().KeyShift) {
                    if (!isSelected) {
                        // まだ選択されていなければ追加
                        auto command = std::make_unique<AddSelectedEntitiesCommand>(this, entityHandle);
                        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
                    } else {
                        // すでに選択されていれば解除
                        auto command = std::make_unique<RemoveSelectedEntitiesCommand>(this, entityHandle);
                        OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
                    }
                } else {
                    // Shiftキーが押されていない場合は選択をクリアしてから追加
                    auto clearCommand = std::make_unique<ClearSelectedEntitiesCommand>(this);
                    OriGine::EditorController::GetInstance()->PushCommand(std::move(clearCommand));
                    auto addCommand = std::make_unique<AddSelectedEntitiesCommand>(this, entityHandle);
                    OriGine::EditorController::GetInstance()->PushCommand(std::move(addCommand));

                    auto& parentWindowHasAreas  = parentArea_->GetParentWindow()->GetAreas();
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
                    auto changedEditEntity = std::make_unique<EntityInspectorArea::ChangeEditEntityCommand>(entityInspectorArea, entityHandle, entityInspectorArea->GetEditEntityHandle());
                    OriGine::EditorController::GetInstance()->PushCommand(std::move(changedEditEntity));
                }
            }
        }
    }

    ImGui::PopStyleColor(3);
}

void EntityHierarchy::Finalize() {}

#pragma endregion

EntityHierarchy::AddSelectedEntitiesCommand::AddSelectedEntitiesCommand(EntityHierarchy* _hierarchy, EntityHandle _addedEntityHandle)
    : hierarchy_(_hierarchy), addedEntityHandle_(_addedEntityHandle) {}

void EntityHierarchy::AddSelectedEntitiesCommand::Execute() {
    auto& selectedEntityIds = hierarchy_->selectedEntityHandles_;
    if (std::find(selectedEntityIds.begin(), selectedEntityIds.end(), addedEntityHandle_) == selectedEntityIds.end()) {
        selectedEntityIds.push_back(addedEntityHandle_);
    } else {
        LOG_DEBUG("AddSelectedEntitiesCommand::Execute: Entity Handle : {} is already selected.", uuids::to_string(addedEntityHandle_.uuid));
    }
}

void EntityHierarchy::AddSelectedEntitiesCommand::Undo() {
    auto& selectedEntityIds = hierarchy_->selectedEntityHandles_;
    auto it                 = std::remove(selectedEntityIds.begin(), selectedEntityIds.end(), addedEntityHandle_);
    if (it != selectedEntityIds.end()) {
        selectedEntityIds.erase(it, selectedEntityIds.end());
        LOG_DEBUG("AddSelectedEntitiesCommand::Undo: Removed entity Handle : {} from selection.", uuids::to_string(addedEntityHandle_.uuid));
    } else {
        LOG_DEBUG("AddSelectedEntitiesCommand::Undo: Entity Handle : {} was not in selection.", uuids::to_string(addedEntityHandle_.uuid));
    }
}

EntityHierarchy::RemoveSelectedEntitiesCommand::RemoveSelectedEntitiesCommand(EntityHierarchy* _hierarchy, EntityHandle _removedEntityHandle)
    : hierarchy_(_hierarchy), removedEntityHandle_(_removedEntityHandle) {}

void EntityHierarchy::RemoveSelectedEntitiesCommand::Execute() {
    auto& selectedEntityIds = hierarchy_->selectedEntityHandles_;
    auto it                 = std::remove(selectedEntityIds.begin(), selectedEntityIds.end(), removedEntityHandle_);
    if (it != selectedEntityIds.end()) {
        selectedEntityIds.erase(it, selectedEntityIds.end());
        LOG_DEBUG("RemoveSelectedEntitiesCommand::Execute: Removed entity Handle : {} from selection.", uuids::to_string(removedEntityHandle_.uuid));
    } else {
        LOG_DEBUG("RemoveSelectedEntitiesCommand::Execute: Entity Handle : {} was not in selection.", uuids::to_string(removedEntityHandle_.uuid));
    }
}

void EntityHierarchy::RemoveSelectedEntitiesCommand::Undo() {
    auto& selectedEntityIds = hierarchy_->selectedEntityHandles_;
    if (std::find(selectedEntityIds.begin(), selectedEntityIds.end(), removedEntityHandle_) == selectedEntityIds.end()) {
        selectedEntityIds.push_back(removedEntityHandle_);
        LOG_DEBUG("RemoveSelectedEntitiesCommand::Undo: Added entity Handle : {} back to selection.", uuids::to_string(removedEntityHandle_.uuid));
    } else {
        LOG_DEBUG("RemoveSelectedEntitiesCommand::Undo: Entity Handle : {} is already in selection.", uuids::to_string(removedEntityHandle_.uuid));
    }
}

EntityHierarchy::ClearSelectedEntitiesCommand::ClearSelectedEntitiesCommand(EntityHierarchy* _hierarchy)
    : hierarchy_(_hierarchy) {}

void EntityHierarchy::ClearSelectedEntitiesCommand::Execute() {
    previousselectedEntityHandles_ = hierarchy_->selectedEntityHandles_; // 現在の選択状態を保存
    if (!previousselectedEntityHandles_.empty()) {
        hierarchy_->selectedEntityHandles_.clear();
        LOG_DEBUG("ClearSelectedEntitiesCommand::Execute: Cleared all selected entities.");
    } else {
        LOG_DEBUG("ClearSelectedEntitiesCommand::Execute: No entities were selected to Clear.");
    }
}

void EntityHierarchy::ClearSelectedEntitiesCommand::Undo() {
    auto& selectedEntityIds = hierarchy_->selectedEntityHandles_;
    if (previousselectedEntityHandles_.empty()) {
        LOG_DEBUG("ClearSelectedEntitiesCommand::Undo: No previous selection to restore.");
        return;
    }
    selectedEntityIds = previousselectedEntityHandles_; // 保存した選択状態を復元
}

EntityHierarchy::CreateEntityCommand::CreateEntityCommand(HierarchyArea* _parentArea, const std::string& _entityName) {
    parentArea_ = _parentArea;
    entityName_ = _entityName;
}
void EntityHierarchy::CreateEntityCommand::Execute() {
    auto currentScene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("CreateEntityCommand::Execute: No current scene found.");
        return;
    }

    entityHandle_ = currentScene->GetEntityRepositoryRef()->CreateEntity(entityName_);

    LOG_DEBUG("CreateEntityCommand::Execute: Created entity with Handle '{}'.", uuids::to_string(entityHandle_.uuid));
}
void EntityHierarchy::CreateEntityCommand::Undo() {
    auto currentScene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("No current scene found.");
        return;
    }

    currentScene->AddDeleteEntity(entityHandle_);

    LOG_DEBUG("Removed entity with UUHandle : {}.", uuids::to_string(entityHandle_.uuid));
}

EntityHierarchy::LoadEntityCommand::LoadEntityCommand(HierarchyArea* _parentArea, const std::string& _directory, const std::string& _entityName) {
    parentArea_ = _parentArea;
    directory_  = _directory;
    entityName_ = _entityName;

    // 読み込み ＆ テンプレート登録
    SceneJsonRegistry::GetInstance()->LoadEntityTemplate(directory_, entityName_);
}
void EntityHierarchy::LoadEntityCommand::Execute() {
    auto currentScene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("No current scene found.");
        return;
    }

    SceneFactory factory;
    Entity* createdEntity = factory.BuildEntityFromTemplate(currentScene, entityName_);
    entityHandle_         = createdEntity->GetHandle();

    LOG_DEBUG("Created entity with Handle : {}.", uuids::to_string(entityHandle_.uuid));
}
void EntityHierarchy::LoadEntityCommand::Undo() {
    auto currentScene = parentArea_->GetParentWindow()->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("No current scene found.");
        return;
    }

    currentScene->AddDeleteEntity(entityHandle_);

    LOG_DEBUG("Removed entity with Handle : {}.", uuids::to_string(entityHandle_.uuid));
}
EntityHierarchy::CopyEntityCommand::CopyEntityCommand(EntityHierarchy* _hierarchy) : hierarchy_(_hierarchy) {}

void EntityHierarchy::CopyEntityCommand::Execute() {
    if (hierarchy_->selectedEntityHandles_.empty()) {
        return;
    }
    // 既存のコピー内容をクリア
    if (!hierarchy_->copyBuffer_.empty()) {
        hierarchy_->copyBuffer_.clear();
    }

    // Dataをコピー
    auto currentScene = hierarchy_->parentArea_->GetParentWindow()->GetCurrentScene();

    SceneFactory factory;
    for (auto entityId : hierarchy_->selectedEntityHandles_) {
        Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with Handle : {} not found for copying.", uuids::to_string(entityId.uuid));
            continue;
        }
        hierarchy_->copyBuffer_.emplace_back(factory.CreateEntityJsonFromEntity(currentScene, entity));
    }
}

void EntityHierarchy::CopyEntityCommand::Undo() {
    // コピーしたデータをクリア
    hierarchy_->copyBuffer_.clear();
}

EntityHierarchy::PasteEntityCommand::PasteEntityCommand(EntityHierarchy* _hierarchy) : hierarchy_(_hierarchy) {}

void EntityHierarchy::PasteEntityCommand::Execute() {
    if (hierarchy_->copyBuffer_.empty()) {
        return;
    }
    // copyBufferのデータをシーンに貼り付け
    auto currentScene = hierarchy_->parentArea_->GetParentWindow()->GetCurrentScene();

    // 貼り付けて生成したエンティティIDを保存(削除に利用)
    SceneFactory sceneFactory;
    for (const auto& entityJson : hierarchy_->copyBuffer_) {
        Entity* createdEntity = sceneFactory.BuildEntity(currentScene, entityJson);
        pastedEntityHandles_.emplace_back(createdEntity->GetHandle());
    }
}

void EntityHierarchy::PasteEntityCommand::Undo() {
    // 貼り付けたエンティティを削除
    auto currentScene = hierarchy_->parentArea_->GetParentWindow()->GetCurrentScene();
    for (auto entityId : pastedEntityHandles_) {
        currentScene->AddDeleteEntity(entityId);
    }
    pastedEntityHandles_.clear();
}

void AddComponentCommand::Execute() {
    auto sceneEditorWindow        = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene             = sceneEditorWindow->GetCurrentScene();
    auto inspectorArea            = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->GetArea("EntityInspectorArea").get());
    EntityHandle editEntityHandle = inspectorArea->GetEditEntityHandle();

    for (auto entityId : entityHandles_) {
        Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with Handle : {} not found.", uuids::to_string(entityId.uuid));
            return;
        }

        // コンポーネントの追加
        IComponentArray* compArray = currentScene->GetComponentRepositoryRef()->GetComponentArray(componentTypeName_);
        compArray->AddComponent(currentScene, editEntityHandle);
        if (!compArray) {
            LOG_ERROR("Failed to add component '{}'. \n ", componentTypeName_);
            return;
        }
    }
}

void AddComponentCommand::Undo() {
    auto sceneEditorWindow        = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene             = sceneEditorWindow->GetCurrentScene();
    auto inspectorArea            = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->GetArea("EntityInspectorArea").get());
    EntityHandle editEntityHandle = inspectorArea->GetEditEntityHandle();

    for (auto entityId : entityHandles_) {
        Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with Handle : {} not found.", uuids::to_string(entityId.uuid));
            return;
        }
        // コンポーネントの削除
        IComponentArray* compArray = currentScene->GetComponentRepositoryRef()->GetComponentArray(componentTypeName_);
        if (!compArray) {
            LOG_ERROR("ComponentArray '{}' not found.", componentTypeName_);
            return;
        }
        compArray->RemoveComponent(entityId, compArray->GetComponentCount(entityId));
    }
}

void RemoveComponentCommand::Execute() {
    auto sceneEditorWindow        = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene             = sceneEditorWindow->GetCurrentScene();
    auto inspectorArea            = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->GetArea("EntityInspectorArea").get());
    EntityHandle editEntityHandle = inspectorArea->GetEditEntityHandle();

    Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityHandle_);
    if (!entity) {
        LOG_ERROR("Entity with Handle : {} not found.", uuids::to_string(entityHandle_.uuid));
        return;
    }
    // コンポーネントの削除
    IComponentArray* compArray = currentScene->GetComponentRepositoryRef()->GetComponentArray(componentTypeName_);
    if (!compArray) {
        LOG_ERROR("ComponentArray '{}' not found.", componentTypeName_);
        return;
    }
    compArray->RemoveComponent(entityHandle_, componentIndex_);
}

void RemoveComponentCommand::Undo() {
    auto sceneEditorWindow        = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene             = sceneEditorWindow->GetCurrentScene();
    auto inspectorArea            = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->GetArea("EntityInspectorArea").get());
    EntityHandle editEntityHandle = inspectorArea->GetEditEntityHandle();

    Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityHandle_);
    if (!entity) {
        LOG_ERROR("Entity with Handle : {} not found.", uuids::to_string(entityHandle_.uuid));
        return;
    }

    // コンポーネントの追加
    IComponentArray* compArray = currentScene->GetComponentRepositoryRef()->GetComponentArray(componentTypeName_);
    compArray->AddComponent(currentScene, editEntityHandle);
    if (!compArray) {
        LOG_ERROR("Failed to add component '{}'. \n ", componentTypeName_);
        return;
    }
}

AddSystemCommand::AddSystemCommand(const std::list<EntityHandle>& _entityHandles, const std::string& _systemTypeName, SystemCategory _category)
    : entityHandles_(_entityHandles), systemTypeName_(_systemTypeName), systemCategory_(_category) {};

void AddSystemCommand::Execute() {
    auto sceneEditorWindow        = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene             = sceneEditorWindow->GetCurrentScene();
    auto inspectorArea            = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->GetArea("EntityInspectorArea").get());
    EntityHandle editEntityHandle = inspectorArea->GetEditEntityHandle();
    if (!currentScene) {
        LOG_ERROR("AddSystemCommand::Execute: No current scene found.");
        return;
    }
    for (auto entityId : entityHandles_) {
        Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("AddSystemCommand::Execute: Entity with Handle : {} not found.", uuids::to_string(entityId.uuid));
            continue;
        }
        currentScene->GetSystemRunnerRef()->RegisterEntity(systemTypeName_, entityId);

        if (editEntityHandle == entityId) {
            inspectorArea->GetSystemMap()[int32_t(systemCategory_)][systemTypeName_] = currentScene->GetSystemRunnerRef()->GetSystem(systemTypeName_);
        }
    }
}

void AddSystemCommand::Undo() {
    auto sceneEditorWindow        = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene             = sceneEditorWindow->GetCurrentScene();
    auto inspectorArea            = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->GetArea("EntityInspectorArea").get());
    EntityHandle editEntityHandle = inspectorArea->GetEditEntityHandle();
    if (!currentScene) {
        LOG_ERROR("AddSystemCommand::Execute: No current scene found.");
        return;
    }
    for (auto entityId : entityHandles_) {
        Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("AddSystemCommand::Execute: Entity with Handle : {} not found.", uuids::to_string(entityId.uuid));
            continue;
        }
        currentScene->GetSystemRunnerRef()->RemoveEntity(systemTypeName_, entityId);

        if (editEntityHandle == entityId) {
            auto itr = inspectorArea->GetSystemMap()[int32_t(systemCategory_)].find(systemTypeName_);
            inspectorArea->GetSystemMap()[int32_t(systemCategory_)].erase(itr);
        }
    }
}

RemoveSystemCommand::RemoveSystemCommand(const std::list<EntityHandle>& _entityIds, const std::string& _systemTypeName, SystemCategory _category)
    : entityHandles_(_entityIds), systemTypeName_(_systemTypeName), systemCategory_(_category) {}

void RemoveSystemCommand::Execute() {
    auto sceneEditorWindow        = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene             = sceneEditorWindow->GetCurrentScene();
    auto inspectorArea            = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->GetArea("EntityInspectorArea").get());
    EntityHandle editEntityHandle = inspectorArea->GetEditEntityHandle();

    if (!currentScene) {
        LOG_ERROR("No current scene found.");
        return;
    }
    for (auto entityId : entityHandles_) {
        Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with Handle : {} not found.", uuids::to_string(entityId.uuid));
            continue;
        }
        currentScene->GetSystemRunnerRef()->RemoveEntity(systemTypeName_, entityId);
        if (editEntityHandle == entityId) {
            auto itr = inspectorArea->GetSystemMap()[int32_t(systemCategory_)].find(systemTypeName_);
            inspectorArea->GetSystemMap()[int32_t(systemCategory_)].erase(itr);
        }
    }
}

void RemoveSystemCommand::Undo() {
    auto sceneEditorWindow        = OriGine::EditorController::GetInstance()->GetWindow<SceneEditorWindow>();
    auto currentScene             = sceneEditorWindow->GetCurrentScene();
    auto inspectorArea            = dynamic_cast<EntityInspectorArea*>(sceneEditorWindow->GetArea("EntityInspectorArea").get());
    EntityHandle editEntityHandle = inspectorArea->GetEditEntityHandle();
    if (!currentScene) {
        LOG_ERROR("No current scene found.");
        return;
    }
    for (auto entityId : entityHandles_) {
        Entity* entity = currentScene->GetEntityRepositoryRef()->GetEntity(entityId);
        if (!entity) {
            LOG_ERROR("Entity with Handle : {} not found.", uuids::to_string(entityId.uuid));
            continue;
        }
        currentScene->GetSystemRunnerRef()->RegisterEntity(systemTypeName_, entityId);
        if (editEntityHandle == entityId) {
            inspectorArea->GetSystemMap()[int32_t(systemCategory_)][systemTypeName_] = currentScene->GetSystemRunnerRef()->GetSystem(systemTypeName_);
        }
    }
}

#pragma region "DevelopControlArea"
DevelopControlArea::DevelopControlArea(SceneEditorWindow* _parentWindow)
    : Editor::Area(nameof<DevelopControlArea>()), parentWindow_(_parentWindow) {}
DevelopControlArea::~DevelopControlArea() {}

void DevelopControlArea::Initialize() {
    AddRegion(std::make_shared<ControlRegion>(this));
}

DevelopControlArea::ControlRegion::ControlRegion(DevelopControlArea* _parentArea)
    : Editor::Region(nameof<ControlRegion>()), parentArea_(_parentArea) {}

DevelopControlArea::ControlRegion::~ControlRegion() {}

void DevelopControlArea::ControlRegion::Initialize() {}

void DevelopControlArea::ControlRegion::DrawGui() {

    if (ImGui::Button("Build Develop") && !parentArea_->isBuilding_) {
        auto* currentScene = parentArea_->GetParentWindow()->GetCurrentScene();
        if (!currentScene) {
            LOG_ERROR("No current scene found.");
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
        LOG_DEBUG("Executing build command: {}", buildCommand);

        // buildThread を立てる(Build中もエディターを操作できるように)
        parentArea_->isBuilding_ = true;
        std::thread([this, cmd = std::move(buildCommand)]() {
            RunProcessAndWait(cmd);
            parentArea_->isBuilding_ = false;
        }).detach();
    }

    ImGui::SameLine();

    if (ImGui::Button("Run") && !parentArea_->isBuilding_) {
        auto* currentScene  = parentArea_->GetParentWindow()->GetCurrentScene();
        std::string exePath = std::filesystem::current_path().string() + parentArea_->exePath_;
        LOG_DEBUG("Executing application at path: {}", exePath);

        std::string runCommand = std::format("{} {} {}", exePath, "-s", currentScene->GetName()); // 実行ファイルパスと 実行する scene を送る
        // アプリケーションの実行
        int32_t result = std::system(runCommand.c_str());
        if (result != 0) {
            LOG_ERROR("Failed to run application. Error code: {}", result);
        } else {
            LOG_DEBUG("Application executed successfully.");
        }
    }
}

void DevelopControlArea::ControlRegion::Finalize() {}

#endif
