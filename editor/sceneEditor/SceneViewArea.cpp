#include "SceneViewArea.h"

#ifdef _DEBUG

/// engine
#include "camera/CameraManager.h"
#include "input/InputManager.h"
#include "scene/Scene.h"
#include "Engine.h"
#include "winApp/WinApp.h"
// directX12
#include "directX12/RenderTexture.h"

/// math
#include "math/Matrix4x4.h"

/// stl
#include <cmath>

/// externals
#include <imgui/imgui.h>

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

    // ゲームプレイ全カテゴリ(Input/StateTransition/Movement/Collision/Effect)を更新する。
    // これにより Debug(エディタ) でも Release(LaviGame) と同じレベルで機能を確認できる。
    currentScene->Update();
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
Transform2d* SceneViewArea::GetTargetTransform2d() {
    auto entityInspectorArea = dynamic_cast<EntityInspectorArea*>(parentWindow_->GetArea("EntityInspectorArea").get());
    if (!entityInspectorArea) {
        return nullptr;
    }

    auto* currentScene            = parentWindow_->GetCurrentScene();
    EntityHandle editEntityHandle = entityInspectorArea->GetEditEntityHandle();

    // エンティティ存在確認
    if (!currentScene->GetEntity(editEntityHandle)) {
        return nullptr;
    }

    // Transform2d コンポーネント確認（未登録のシーンでは配列が無いこともあるので null ガード）
    auto transform2dArray = currentScene->GetComponentArray<Transform2d>();
    if (!transform2dArray || !transform2dArray->HasEntity(editEntityHandle)) {
        return nullptr;
    }

    return currentScene->GetComponent<Transform2d>(editEntityHandle);
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
bool SceneViewArea::ApplyGizmoManipulation2d(OriGine::Transform2d* _transform2d) {
    constexpr float kPi      = 3.14159265358979323846f;
    constexpr float kRad2Deg = 180.0f / kPi;
    constexpr float kDeg2Rad = kPi / 180.0f;

    // スプライト描画と同じスクリーン空間正射影を使い、view を単位行列にすることで
    // Gizmo が UI 要素(スクリーン座標)にぴったり重なって表示される（解像度差は NDC で相殺）。
    const Vec2f window = Engine::GetInstance()->GetWinApp()->GetWindowSize();
    Matrix4x4 viewMat  = MakeMatrix4x4::Identity();
    Matrix4x4 projMat  = MakeMatrix4x4::Orthographic(0.0f, 0.0f, window[0], window[1], 0.0f, 1000.0f);

    float viewMatrix[16];
    float projMatrix[16];
    viewMat.toFloatArray(viewMatrix);
    projMat.toFloatArray(projMatrix);

    // 現在のワールド TRS（親子付き）を ImGuizmo の TRS 表現（translate, rotateDeg, scale）で組む。
    const Vec2f worldScale  = _transform2d->GetWorldScale();
    const float worldRotDeg = _transform2d->CalculateWorldRotate() * kRad2Deg;
    const Vec2f worldTrans  = _transform2d->GetWorldTranslate();

    float t[3] = {worldTrans[0], worldTrans[1], 0.0f};
    float r[3] = {0.0f, 0.0f, worldRotDeg};
    float s[3] = {worldScale[0], worldScale[1], 1.0f};
    float worldMatrix[16];
    ImGuizmo::RecomposeMatrixFromComponents(t, r, s, worldMatrix);

    // 操作種別はキー設定（Shift+T/R/S）を流用。2D に無意味な Z 成分は後段で破棄する。
    ImGuizmo::SetOrthographic(true); // 2D はスクリーン正射影。3D 経路は SetupGizmoSettings で false に戻す。
    if (ImGuizmo::Manipulate(viewMatrix, projMatrix, currentGizmoOperation_, ImGuizmo::WORLD, worldMatrix)) {
        ImGuizmo::DecomposeMatrixToComponents(worldMatrix, t, r, s);
        const Vec2f newWorldScale = {s[0], s[1]};
        const float newWorldRot   = r[2] * kDeg2Rad; // ラジアン
        const Vec2f newWorldTrans  = {t[0], t[1]};

        // ワールド → ローカル（親 TRS の逆変換）。親が無ければワールド=ローカル。
        // worldT = R(pRot) * (pScale ⊙ localT) + pTrans  ⇒  localT = pScale^-1 ⊙ ( R(-pRot) * (worldT - pTrans) )
        Vec2f pScale = {1.0f, 1.0f};
        float pRot   = 0.0f;
        Vec2f pTrans = {0.0f, 0.0f};
        if (_transform2d->parent) {
            pScale = _transform2d->parent->GetWorldScale();
            pRot   = _transform2d->parent->CalculateWorldRotate();
            pTrans = _transform2d->parent->GetWorldTranslate();
        }

        _transform2d->scale[0] = (pScale[0] != 0.0f) ? newWorldScale[0] / pScale[0] : newWorldScale[0];
        _transform2d->scale[1] = (pScale[1] != 0.0f) ? newWorldScale[1] / pScale[1] : newWorldScale[1];
        _transform2d->rotate   = newWorldRot - pRot;

        const float dx = newWorldTrans[0] - pTrans[0];
        const float dy = newWorldTrans[1] - pTrans[1];
        const float c  = std::cos(-pRot);
        const float sn = std::sin(-pRot);
        float lx = c * dx - sn * dy;
        float ly = sn * dx + c * dy;
        if (pScale[0] != 0.0f) lx /= pScale[0];
        if (pScale[1] != 0.0f) ly /= pScale[1];
        _transform2d->translate[0] = lx;
        _transform2d->translate[1] = ly;

        _transform2d->UpdateMatrix();
        return true;
    }
    return false;
}
void SceneViewArea::ManageGizmoCommands2d(Entity* editEntity, Transform2d* transform2d, bool isUsingGuizmo) {
    static bool wasUsing2d = false;
    static GuiValuePool<Vec2f> vec2fPool;
    static GuiValuePool<float> floatPool;

    if (!editEntity || !transform2d) {
        wasUsing2d = isUsingGuizmo;
        return;
    }

    if (isUsingGuizmo) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        // 操作開始時の値を保存
        if (!wasUsing2d) {
            vec2fPool.SetValue(editEntity->GetUniqueID() + "Scale2d", transform2d->scale);
            floatPool.SetValue(editEntity->GetUniqueID() + "Rotate2d", transform2d->rotate);
            vec2fPool.SetValue(editEntity->GetUniqueID() + "Translate2d", transform2d->translate);
        }
    } else {
        // 操作終了時（マウスを離した瞬間）にコマンド生成
        if (wasUsing2d) {
            auto commandCombo = std::make_unique<CommandCombo>();

            commandCombo->AddCommand(std::make_unique<SetterCommand<Vec2f>>(
                &transform2d->scale, transform2d->scale, vec2fPool.popValue(editEntity->GetUniqueID() + "Scale2d")));

            commandCombo->AddCommand(std::make_unique<SetterCommand<float>>(
                &transform2d->rotate, transform2d->rotate, floatPool.popValue(editEntity->GetUniqueID() + "Rotate2d")));

            commandCombo->AddCommand(std::make_unique<SetterCommand<Vec2f>>(
                &transform2d->translate, transform2d->translate, vec2fPool.popValue(editEntity->GetUniqueID() + "Translate2d")));

            commandCombo->SetFuncOnAfterCommand(
                [transform2d]() {
                    if (transform2d)
                        transform2d->UpdateMatrix();
                },
                true);

            OriGine::EditorController::GetInstance()->PushCommand(std::move(commandCombo));
        }
    }

    wasUsing2d = isUsingGuizmo;
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

    // 2. 操作対象を取得 (3D Transform と 2D Transform2d。どちらも無ければ終了)
    Transform* transform     = GetTargetTransform();
    Transform2d* transform2d = GetTargetTransform2d();
    if (!transform && !transform2d) {
        return;
    }

    // 3. 両方を持つエンティティでは 2D/3D を選択できるようにする
    //    (片方しか無い場合はそれが自動的に使われる)
    if (transform && transform2d) {
        ImGui::Begin("Gizmo Target");
        ImGui::TextUnformatted("Transform / Transform2d 両方あり");
        int mode = gizmoUse2d_ ? 1 : 0;
        if (ImGui::RadioButton("3D (Transform)", &mode, 0)) {
            gizmoUse2d_ = false;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("2D (Transform2d)", &mode, 1)) {
            gizmoUse2d_ = true;
        }
        ImGui::End();
    }
    const bool use2d = transform2d && (!transform || gizmoUse2d_);

    // 4-2D. スクリーン空間 Gizmo (Transform2d を移動/回転/拡縮。Undo/Redo 対応)
    if (use2d) {
        DetermineGizmoOperation(); // Shift+T/R/S で操作種別を切替（3D と共通）
        ApplyGizmoManipulation2d(transform2d);

        EntityHandle editEntityHandle = dynamic_cast<EntityInspectorArea*>(parentWindow_->GetArea("EntityInspectorArea").get())->GetEditEntityHandle();
        Entity* editEntity            = parentWindow_->GetCurrentScene()->GetEntity(editEntityHandle);
        ManageGizmoCommands2d(editEntity, transform2d, ImGuizmo::IsUsing());
        return;
    }

    // 4-3D. 既存の3D経路
    DetermineGizmoOperation();
    const auto& cameraTrans = debugCamera_->GetCameraTransform();
    ApplyGizmoManipulation(transform, cameraTrans);

    // 操作確定時のコマンド生成 (Undo/Redo)
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
