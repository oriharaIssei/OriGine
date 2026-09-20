#include "DebugCamera.h"

/// engine
#include "input/InputManager.h"

#ifdef ORIGINE_EDITOR_ENABLED
/// externals
#include "imgui/imgui.h"
#endif // _DEBUG
/// math
#include <cmath>
#include <numbers>

using namespace OriGine;

/// <summary> 初期化. 初期状態として Neutral（入力待機）ステートを設定する. </summary>
void DebugCamera::Initialize() {
    currentState_ = std::make_unique<Neutral>(this);
}
/// <summary> 終了処理. 現在のステートを破棄する. </summary>
void DebugCamera::Finalize() {
    currentState_.reset();
}

/// <summary> ImGui によるデバッグ表示・操作を行う. </summary>
void DebugCamera::DebugUpdate() {
#ifdef ORIGINE_EDITOR_ENABLED
    if (ImGui::Begin("DebugCamera")) {
        ImGui::DragFloat4("Rotate", &cameraBuff_.rotate[X], 0.1f);
        ImGui::DragFloat3("Translate", &cameraBuff_.translate[X], 0.1f);

        cameraBuff_.UpdateMatrix();
    }
    ImGui::End();
#endif // _DEBUG
}

/// <summary> カメラの更新（ステート更新と行列計算）を行う. </summary>
void DebugCamera::Update() {
    // 現在の状態に応じた更新処理を実行
    // 実行終了後 cameraBuff_の行列を更新
    if (currentState_) {
        currentState_->Update();
    }
    cameraBuff_.UpdateMatrix();
}

/// <summary> 入力受付待機状態の更新. Alt + マウス操作を検知して他の状態へ遷移する. </summary>
void DebugCamera::Neutral::Update() {
    KeyboardInput* keyInput = InputManager::GetInstance()->GetKeyboard();
    MouseInput* mouseInput  = InputManager::GetInstance()->GetMouse();

    // Altキーが押されていない場合は何もしない
    if (!(keyInput->IsPress(DIK_LALT) || !keyInput->IsPress(DIK_RALT))) {
        return;
    }
    // Alt + 左クリックまたはホイール操作で移動状態へ遷移
    // Alt + 右クリックで回転状態へ遷移
    if (mouseInput->IsTrigger(MouseButton::LEFT) || mouseInput->IsWheel()) {
        host_->startMousePos_ = mouseInput->GetPosition();
        host_->currentState_  = std::make_unique<TranslationState>(host_);
        return;
    } else if (mouseInput->IsTrigger(MouseButton::RIGHT)) {
        host_->startMousePos_ = mouseInput->GetPosition();
        host_->currentState_  = std::make_unique<RotationState>(host_);
        return;
    }
}

/// <summary> カメラ並行移動・ズーム状態の更新. マウスの移動量・ホイール量からカメラ座標を変化させる. </summary>
void DebugCamera::TranslationState::Update() {
    KeyboardInput* keyInput = InputManager::GetInstance()->GetKeyboard();
    MouseInput* mouseInput  = InputManager::GetInstance()->GetMouse();

    constexpr Vec3f kMouseSensitivity = {0.1f, 0.1f, 0.7f};

    // マウス位置を開始位置にリセット
    mouseInput->SetPosition(host_->startMousePos_);

    // 入力状態をビットで管理
    uint32_t state = 0;
    bool a         = mouseInput->IsWheel();
    bool b         = mouseInput->IsPress(MouseButton::LEFT);
    uint32_t c     = (keyInput->IsPress(Key::L_ALT) | keyInput->IsPress(Key::R_ALT));
    state          = (a) + (b * 2);
    state *= c;
    Vec3f velo = {};

    Vector3 inputVal = {mouseInput->GetVelocity(), (float)mouseInput->GetWheelDelta()};

    // 状態に応じた移動処理
    // wheel -> Z方向移動
    // mouse move -> XY平面移動
    // wheel + mouse move -> XYZ全方向移動
    switch ((TranslationType)state) {
    case NONE:
        host_->currentState_ = std::make_unique<Neutral>(host_);
        return;
    case Z_WHEEL:
        velo = {0.0f, 0.0f, inputVal[Z]};
        break;
    case XY_MOUSEMOVE:
        velo = {inputVal[X], inputVal[Y], 0.0f};
        break;
    case XYZ_ALL:
        velo = inputVal;
        break;
    default:
        break;
    }

    velo[Y] *= -1.0f;

    for (size_t axis = 0; axis < 3; axis++) {
        velo[axis] *= kMouseSensitivity[axis];
    }

    host_->cameraBuff_.translate += velo * MakeMatrix4x4::RotateQuaternion(host_->cameraBuff_.rotate);
}

/// <summary> カメラ回転状態の更新. マウスの移動量をヨー・ピッチ回転に変換して現在の回転に合成する. </summary>
void DebugCamera::RotationState::Update() {
    constexpr float kMouseSensitivity = 0.01f;

    KeyboardInput* keyInput = InputManager::GetInstance()->GetKeyboard();
    MouseInput* mouseInput  = InputManager::GetInstance()->GetMouse();

    // マウス位置を開始位置にリセット
    mouseInput->SetPosition(host_->startMousePos_);

    if (!mouseInput->IsPress(MouseButton::RIGHT) || !(keyInput->IsPress(Key::L_ALT) || keyInput->IsPress(Key::R_ALT))) {
        host_->currentState_ = std::make_unique<Neutral>(host_);
        return;
    }

    // マウスの動きから回転量を取得
    Vec2f mouseVelocity = mouseInput->GetVelocity();
    float yaw           = mouseVelocity[X] * kMouseSensitivity; // Y軸回転（水平）
    float pitch         = mouseVelocity[Y] * kMouseSensitivity; // X軸回転（垂直）

    // クォータニオンを生成
    Quaternion yawRotation   = Quaternion::RotateAxisAngle({0.0f, 1.0f, 0.0f}, yaw); // Y軸回転
    Quaternion pitchRotation = Quaternion::RotateAxisAngle({1.0f, 0.0f, 0.0f}, pitch); // X軸回転

    // 現在の回転に新しい回転を適用
    host_->cameraBuff_.rotate *= yawRotation * pitchRotation;
}
