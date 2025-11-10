#include "DebugCamera.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#include <cmath>
#include <numbers>

void DebugCamera::Initialize() {
    currentState_.reset(new Neutral(this));
}
void DebugCamera::Finalize() {
    currentState_.reset();
}

void DebugCamera::DebugUpdate() {
#ifdef _DEBUG
    if (ImGui::Begin("DebugCamera")) {
        ImGui::DragFloat3("Rotate", &cameraBuff_.rotate[X], 0.1f);
        ImGui::DragFloat3("Translate", &cameraBuff_.translate[X], 0.1f);

        cameraBuff_.UpdateMatrix();
    }
    ImGui::End();
#endif // _DEBUG
}

void DebugCamera::Update() {
    // 現在の状態に応じた更新処理を実行
    // 実行終了後 cameraBuff_の行列を更新
    if (currentState_) {
        currentState_->Update();
    }
    cameraBuff_.UpdateMatrix();
}

void DebugCamera::Neutral::Update() {
    KeyboardInput* keyInput = InputManager::getInstance()->getKeyboard();
    MouseInput* mouseInput  = InputManager::getInstance()->getMouse();

    // Altキーが押されていない場合は何もしない
    if (!(keyInput->isPress(DIK_LALT) || keyInput->isPress(DIK_RALT))) {
        return;
    }
    // Alt + 左クリックまたはホイール操作で移動状態へ遷移
    // Alt + 右クリックで回転状態へ遷移
    if (mouseInput->isTrigger(MouseButton::LEFT) || mouseInput->isWheel()) {
        host_->startMousePos_ = mouseInput->getVirtualPosition();
        host_->currentState_.reset(new TranslationState(host_));
        return;
    } else if (mouseInput->isTrigger(MouseButton::RIGHT)) {
        host_->startMousePos_ = mouseInput->getVirtualPosition();
        host_->currentState_.reset(new RotationState(host_));
        return;
    }
}

void DebugCamera::TranslationState::Update() {
    KeyboardInput* keyInput = InputManager::getInstance()->getKeyboard();
    MouseInput* mouseInput  = InputManager::getInstance()->getMouse();

    constexpr Vec3f kMouseSensitivity = {0.001f, 0.001f, 0.007f};

    // 入力状態をビットで管理
    uint32_t state = 0;
    bool a         = mouseInput->isWheel();
    bool b         = mouseInput->isPress(0);
    uint32_t c     = (keyInput->isPress(Key::LALT) | keyInput->isPress(Key::R_ALT));
    state          = (a) + (b * 2);
    state *= c;
    Vec3f velo = {};

    Vector3 inputVal = {mouseInput->getVelocity(), (float)mouseInput->getWheelDelta()};
    inputVal         = inputVal * kMouseSensitivity;

    // 状態に応じた移動処理
    // wheel -> Z方向移動
    // mouse move -> XY平面移動
    // wheel + mouse move -> XYZ全方向移動
    switch ((TranslationType)state) {
    case NONE:
        host_->currentState_.reset(new Neutral(host_));
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

    host_->cameraBuff_.translate += velo * MakeMatrix::RotateQuaternion(host_->cameraBuff_.rotate);
}

void DebugCamera::RotationState::Update() {
    constexpr float kMouseSensitivity = 0.01f;

    KeyboardInput* keyInput = InputManager::getInstance()->getKeyboard();
    MouseInput* mouseInput  = InputManager::getInstance()->getMouse();

    // マウス位置を開始位置にリセット
    mouseInput->setPosition(host_->startMousePos_);

    if (!mouseInput->isPress(MouseButton::RIGHT) || !(keyInput->isPress(Key::L_ALT) || keyInput->isPress(Key::R_ALT))) {
        host_->currentState_.reset(new Neutral(host_));
        return;
    }

    // マウスの動きから回転量を取得
    Vec2f mouseVelocity = mouseInput->getVelocity();
    float yaw           = mouseVelocity[X] * kMouseSensitivity; // Y軸回転（水平）
    float pitch         = mouseVelocity[Y] * kMouseSensitivity; // X軸回転（垂直）

    // クォータニオンを生成
    Quaternion yawRotation   = Quaternion::RotateAxisAngle({0.0f, 1.0f, 0.0f}, yaw); // Y軸回転
    Quaternion pitchRotation = Quaternion::RotateAxisAngle({1.0f, 0.0f, 0.0f}, pitch); // X軸回転

    // 現在の回転に新しい回転を適用
    host_->cameraBuff_.rotate *= yawRotation * pitchRotation;
}
