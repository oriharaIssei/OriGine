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

    // Gizmo操作モードの初期化
    currentGizmoOperation_ = ImGuizmo::TRANSLATE | ImGuizmo::ROTATE | ImGuizmo::SCALE;

    // ObjectPickerの初期化
    objectPicker_ = std::make_unique<ObjectPicker>();
    objectPicker_->Initialize();
}

void SceneViewArea::DrawGui() {
    bool isOpen = isOpen_.Current();

    if (!isOpen) {
        isOpen_.Set(isOpen);
        isFocused_.Set(ImGui::IsWindowFocused());
        UpdateFocusAndOpenState();
        return;
    }

    const auto& renderTexture = parentWindow_->GetCurrentScene()->GetSceneView();

    if (ImGui::Begin(name_.c_str(), &isOpen)) {

        areaSize_ = ImGui::GetContentRegionAvail();

        if (areaSize_[X] >= 1.f && areaSize_[Y] >= 1.f && renderTexture->GetTextureSize() != areaSize_) {
            renderTexture->Resize(areaSize_);

            float aspectRatio                                 = areaSize_[X] / areaSize_[Y];
            debugCamera_->GetCameraTransformRef().aspectRatio = aspectRatio;
            debugCamera_->Update();
        }

        DrawScene();

        ImGui::Image(reinterpret_cast<ImTextureID>(renderTexture->GetBackBufferSrvHandle().ptr), areaSize_.toImVec2());

        // マウス座標の変換処理
        ImVec2 imageLeftTop  = ImGui::GetItemRectMin();
        ImVec2 imageRightBot = ImGui::GetItemRectMax();
        Vec2f imageSize      = {imageRightBot.x - imageLeftTop.x,
                 imageRightBot.y - imageLeftTop.y};

        MouseInput* mouseInput = InputManager::GetInstance()->GetMouse();
        Vec2f mousePos         = mouseInput->GetPosition();
        Vec2f gamePos          = ConvertMouseToSceneView(mousePos, imageLeftTop, areaSize_.toImVec2(), imageSize);
        mouseInput->SetVirtualPosition(gamePos);

        if (isFocused_.Current()) {
            switch (DetermineInteractionType()) {
            case ToolInteractionType::Gizmo: {
                UseImGuizmo(imageLeftTop);
            } break;
            case ToolInteractionType::Camera:
                debugCamera_->Update();
                break;
            case ToolInteractionType::ObjectPicker: {
                objectPicker_->Activate(parentWindow_->GetCurrentScene());
                EntityHandle handle = objectPicker_->PickedObject(debugCamera_->GetCameraTransform(), gamePos, renderTexture->GetTextureSize());
                if (handle.IsValid()) {
                    auto* entityInspectorArea = dynamic_cast<EntityInspectorArea*>(parentWindow_->GetArea("EntityInspectorArea").get());
                    if (entityInspectorArea) {
                        EntityHandle currentHandle = entityInspectorArea->GetEditEntityHandle();
                        if (currentHandle != handle) {
                            auto command = std::make_unique<EntityInspectorArea::ChangeEditEntityCommand>(entityInspectorArea, handle, currentHandle);
                            EditorController::GetInstance()->PushCommand(std::move(command));
                        }
                    }
                }
            } break;
            default:
                break;
            }
        }

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
    Scene* currentScene = parentWindow_->GetCurrentScene();
    if (!currentScene) {
        LOG_ERROR("Current scene is nullptr in SceneViewArea::DrawScene.");
        return;
    }

    CameraManager* cameraManager  = CameraManager::GetInstance();
    CameraTransform prevTransform = cameraManager->GetTransform(currentScene);

    cameraManager->SetTransform(currentScene, debugCamera_->GetCameraTransform());
    cameraManager->DataConvertToBuffer(currentScene);

    // effect systemの更新
    currentScene->GetSystemRunnerRef()->UpdateCategory(SystemCategory::Effect);
    // 描画
    currentScene->Render();
    cameraManager->SetTransform(currentScene, prevTransform);
}

SceneViewArea::ToolInteractionType SceneViewArea::DetermineInteractionType() {
    InputManager* input     = InputManager::GetInstance();
    KeyboardInput* keyboard = input->GetKeyboard();
    MouseInput* mouse       = input->GetMouse();

    // Gizmo操作中は他の入力を無効化
    if (ImGuizmo::IsUsing()) {
        return ToolInteractionType::Gizmo;
    }
    // Altキーが押されている場合はカメラ操作
    if (keyboard->IsPress(Key::LALT)) {
        return ToolInteractionType::Camera;
    }
    // オブジェクトピッカーの入力処理
    if (mouse->IsTrigger(MouseButton::LEFT)) {
        return ToolInteractionType::ObjectPicker;
    }

    return ToolInteractionType::None;
}

void SceneViewArea::SetupGizmoSettings(const ImVec2& _sceneViewPos) {
    // ImGuizmo 基本設定
    ImGuizmo::BeginFrame();
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(_sceneViewPos.x, _sceneViewPos.y, areaSize_[X], areaSize_[Y]);
}

Transform* SceneViewArea::GetTargetTransform() {
    auto entityInspectorArea = dynamic_cast<EntityInspectorArea*>(parentWindow_->GetArea("EntityInspectorArea").get());
    if (!entityInspectorArea) {
        LOG_ERROR("EntityInspectorArea not found.");
        return nullptr;
    }

    auto* currentScene            = parentWindow_->GetCurrentScene();
    EntityHandle editEntityHandle = entityInspectorArea->GetEditEntityHandle();

    // エンティティ存在確認
    if (!currentScene->GetEntity(editEntityHandle)) {
        return nullptr;
    }

    // Transformコンポーネント確認
    auto transformArray = currentScene->GetComponentArray<Transform>();
    if (!transformArray->HasEntity(editEntityHandle)) {
        return nullptr;
    }

    return currentScene->GetComponent<Transform>(editEntityHandle);
}
void SceneViewArea::DetermineGizmoOperation() {
    InputManager* input     = InputManager::GetInstance();
    KeyboardInput* keyboard = input->GetKeyboard();

    // Shiftキーが押されていない場合はデフォルトに戻して終了
    if (!keyboard->IsPress(Key::L_SHIFT)) {
        currentGizmoOperation_ = ImGuizmo::TRANSLATE | ImGuizmo::SCALE | ImGuizmo::ROTATE;
        return;
    }

    // ショートカット判定 (S:Scale, R:Rotate, T:Translate)
    if (keyboard->IsPress(Key::S)) {
        if (keyboard->IsPress(Key::X))
            currentGizmoOperation_ = ImGuizmo::SCALE_X;
        else if (keyboard->IsPress(Key::Y))
            currentGizmoOperation_ = ImGuizmo::SCALE_Y;
        else if (keyboard->IsPress(Key::Z))
            currentGizmoOperation_ = ImGuizmo::SCALE_Z;
        else
            currentGizmoOperation_ = ImGuizmo::SCALE;
    } else if (keyboard->IsPress(Key::R)) {
        if (keyboard->IsPress(Key::X))
            currentGizmoOperation_ = ImGuizmo::ROTATE_X;
        else if (keyboard->IsPress(Key::Y))
            currentGizmoOperation_ = ImGuizmo::ROTATE_Y;
        else if (keyboard->IsPress(Key::Z))
            currentGizmoOperation_ = ImGuizmo::ROTATE_Z;
        else
            currentGizmoOperation_ = ImGuizmo::ROTATE;
    } else if (keyboard->IsPress(Key::T)) {
        if (keyboard->IsPress(Key::X))
            currentGizmoOperation_ = ImGuizmo::TRANSLATE_X;
        else if (keyboard->IsPress(Key::Y))
            currentGizmoOperation_ = ImGuizmo::TRANSLATE_Y;
        else if (keyboard->IsPress(Key::Z))
            currentGizmoOperation_ = ImGuizmo::TRANSLATE_Z;
        else
            currentGizmoOperation_ = ImGuizmo::TRANSLATE;
    }
}
bool SceneViewArea::ApplyGizmoManipulation(OriGine::Transform* transform, const OriGine::CameraTransform& cameraTrans) {
    // 行列の準備
    float worldMatrix[16];
    float viewMatrix[16];
    float projMatrix[16];

    transform->worldMat.toFloatArray(worldMatrix);
    cameraTrans.viewMat.toFloatArray(viewMatrix);
    cameraTrans.projectionMat.toFloatArray(projMatrix);

    // Gizmo操作の実行
    if (ImGuizmo::Manipulate(viewMatrix, projMatrix, currentGizmoOperation_, ImGuizmo::LOCAL, worldMatrix)) {
        // 操作された場合、Transformへ反映
        transform->worldMat.fromFloatArray(worldMatrix);
        transform->worldMat.decomposeMatrixToComponents(transform->scale, transform->rotate, transform->translate);
        return true;
    }
    return false;
}

void SceneViewArea::ManageGizmoCommands(Entity* editEntity, Transform* transform, bool isUsingGuizmo) {
    static bool wasUsingGuizmo = false;
    static GuiValuePool<Vec3f> vec3fPool;
    static GuiValuePool<Quaternion> quatPool;

    if (isUsingGuizmo) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

        // 操作開始時の値を保存
        if (!wasUsingGuizmo) {
            vec3fPool.SetValue(editEntity->GetUniqueID() + "Scale", transform->scale);
            quatPool.SetValue(editEntity->GetUniqueID() + "Rotation", transform->rotate);
            vec3fPool.SetValue(editEntity->GetUniqueID() + "Translate", transform->translate);
        }
    } else {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        // 操作終了時（マウスを離した瞬間）にコマンド生成
        if (wasUsingGuizmo) {
            auto commandCombo = std::make_unique<CommandCombo>();

            commandCombo->AddCommand(std::make_unique<SetterCommand<Vec3f>>(
                &transform->scale, transform->scale, vec3fPool.popValue(editEntity->GetUniqueID() + "Scale")));

            commandCombo->AddCommand(std::make_unique<SetterCommand<Quaternion>>(
                &transform->rotate, transform->rotate, quatPool.popValue(editEntity->GetUniqueID() + "Rotation")));

            commandCombo->AddCommand(std::make_unique<SetterCommand<Vec3f>>(
                &transform->translate, transform->translate, vec3fPool.popValue(editEntity->GetUniqueID() + "Translate")));

            commandCombo->SetFuncOnAfterCommand(
                [transform]() {
                    if (transform)
                        transform->UpdateMatrix();
                },
                true);

            OriGine::EditorController::GetInstance()->PushCommand(std::move(commandCombo));
        }
    }

    wasUsingGuizmo = isUsingGuizmo;
}
void SceneViewArea::UseImGuizmo(const ImVec2& _sceneViewPos) {
    // 1. マウス座標処理とImGuizmoのセットアップ
    SetupGizmoSettings(_sceneViewPos);

    // 2. 操作対象のTransformを取得 (なければ終了)
    Transform* transform = GetTargetTransform();
    if (!transform) {
        return;
    }

    // 3. キー入力で操作タイプ(移動/回転/拡大)を更新
    DetermineGizmoOperation();

    // 4. カメラ情報の取得
    const auto& cameraTrans = debugCamera_->GetCameraTransform();

    // 5. ギズモの描画と操作の反映
    //    (操作中であれば true が返るように実装変更も可能だが、ここではImGuizmo::IsUsingを使用)
    ApplyGizmoManipulation(transform, cameraTrans);

    // 6. 操作確定時のコマンド生成 (Undo/Redo)
    EntityHandle editEntityHandle = dynamic_cast<EntityInspectorArea*>(parentWindow_->GetArea("EntityInspectorArea").get())->GetEditEntityHandle();
    Entity* editEntity            = parentWindow_->GetCurrentScene()->GetEntity(editEntityHandle);

    ManageGizmoCommands(editEntity, transform, ImGuizmo::IsUsing());
}

void SceneViewArea::Finalize() {
    objectPicker_->Finalize();
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
