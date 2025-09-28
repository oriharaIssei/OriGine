#include "DebugCamera.h"

#include "imgui/imgui.h"

#include <cmath>
#include <numbers>

void DebugCamera::Initialize() {
    currentState_.reset(new Neutral(this));
}
void DebugCamera::Finalize() {
    currentState_.reset();
}

void DebugCamera::DebugUpdate() {
    if (ImGui::Begin("DebugCamera")) {
        ImGui::DragFloat3("Rotate", &cameraBuff_.rotate[X], 0.1f);
        ImGui::DragFloat3("Translate", &cameraBuff_.translate[X], 0.1f);

        cameraBuff_.UpdateMatrix();
    }
    ImGui::End();
}

void DebugCamera::Update() {
    if (currentState_) {
        currentState_->Update();
    }
    cameraBuff_.UpdateMatrix();
}

void DebugCamera::Neutral::Update() {
    Input* input = Input::getInstance();

    // Altキーが押されていない場合は何もしない
    if (!(input->isPressKey(DIK_LALT) || input->isPressKey(DIK_RALT))) {
        return;
    }
    if (input->isTriggerMouseButton(0) || input->isWheel()) {
        host_->startMousePos_ = input->getCurrentMousePos();
        host_->currentState_.reset(new TranslationState(host_));
        return;
    } else if (input->isTriggerMouseButton(1)) {
        host_->startMousePos_ = input->getCurrentMousePos();
        host_->currentState_.reset(new RotationState(host_));
        return;
    }
}

void DebugCamera::TranslationState::Update() {
    Input* input = Input::getInstance();
    // input->FixMousePos(host_->startMousePos_);

    uint32_t state = 0;
    bool a         = input->isPreWheel();
    bool b         = input->isPressMouseButton(0);
    uint32_t c     = (input->isPressKey(DIK_LALT) | input->isPressKey(DIK_RALT));
    state          = (a) + (b * 2);
    state *= c;
    Vec3f velo = {};

    switch ((TranslationType)state) {
    case NONE:
        host_->currentState_.reset(new Neutral(host_));
        return;
    case Z_WHEEL:
        velo = {0.0f, 0.0f, (float)input->getPreWheel() * 0.007f};
        break;
    case XY_MOUSEMOVE:
        velo = {input->getMouseVelocity() * 0.01f, 0.0f};
        break;
    case XYZ_ALL:
        velo = {input->getMouseVelocity() * 0.01f, (float)input->getPreWheel() * 0.007f};
        break;
    default:
        break;
    }
    velo[Y] *= -1.0f;

    host_->cameraBuff_.translate += velo * MakeMatrix::RotateQuaternion(host_->cameraBuff_.rotate);
}

void DebugCamera::RotationState::Update() {
    Input* input = Input::getInstance();

    // input->FixMousePos(host_->startMousePos_);

    if (!input->isPressMouseButton(1) || !(input->isPressKey(DIK_LALT) || input->isPressKey(DIK_RALT))) {
        host_->currentState_.reset(new Neutral(host_));
        return;
    }

    // マウスの動きから回転量を取得
    Vec2f mouseVelocity = input->getMouseVelocity();
    float yaw           = mouseVelocity[X] * 0.01f; // Y軸回転（水平）
    float pitch         = mouseVelocity[Y] * 0.01f; // X軸回転（垂直）

    // クォータニオンを生成
    Quaternion yawRotation   = Quaternion::RotateAxisAngle({0.0f, 1.0f, 0.0f}, yaw); // Y軸回転
    Quaternion pitchRotation = Quaternion::RotateAxisAngle({1.0f, 0.0f, 0.0f}, pitch); // X軸回転

    // 現在の回転に新しい回転を適用
    host_->cameraBuff_.rotate *= yawRotation * pitchRotation;
}
