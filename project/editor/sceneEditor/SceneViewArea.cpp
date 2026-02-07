#include "SceneViewArea.h"

#ifdef _DEBUG

/// engine
#include "camera/CameraManager.h"
#include "input/InputManager.h"
#include "scene/Scene.h"
// directX12
#include "directX12/RenderTexture.h"

/// ECS
// system
#include "system/SystemRunner.h"

/// editor
#include "EntityInspector.h"
#include "EntityInspectorCommands.h"

/// util
#include "myGui/MyGui.h"

using namespace OriGine;

SceneViewArea::SceneViewArea(SceneEditorWindow* _parentWindow)
    : parentWindow_(_parentWindow), Editor::Area(nameof<SceneViewArea>()) {}

void SceneViewArea::Initialize() {
    // DebugCameraの初期化
    debugCamera_ = std::make_unique<OriGine::DebugCamera>();
    debugCamera_->Initialize();

    // Gizmo操作モードの初期化
    currentGizmoOperation_ = ImGuizmo::TRANSLATE | ImGuizmo::ROTATE | ImGuizmo::SCALE;

    // ObjectPickerの初期化
    objectPicker_ = std::make_unique<OriGine::ObjectPicker>();
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

        debugCamera_->DebugUpdate();

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
            // guizmoは常に操作可能にしておく
            // guizmoを使用していない時に、他のツール操作を判定する
            UseImGuizmo(imageLeftTop);

            switch (DetermineInteractionType()) {
            case ToolInteractionType::Gizmo:
                break;
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
                            auto command = std::make_unique<ChangeEditEntityCommand>(entityInspectorArea, handle, currentHandle);
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

#endif // _DEBUG
