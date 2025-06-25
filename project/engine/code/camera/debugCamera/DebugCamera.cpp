#include "DebugCamera.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#include <cmath>
#include <numbers>

void DebugCamera::Initialize(){
    input_ = Input::getInstance();
    currentState_.reset(new Neutral(this));
}

void DebugCamera::DebugUpdate(){
#ifdef _DEBUG
    if(ImGui::Begin("DebugCamera")){
        ImGui::DragFloat3("Rotate",&cameraBuff_.rotate[X],0.1f);
        ImGui::DragFloat3("Translate",&cameraBuff_.translate[X],0.1f);
    }
    ImGui::End();
#endif // _DEBUG
}

void DebugCamera::Update(){
    if(currentState_){
        currentState_->Update();
    }
    cameraBuff_.UpdateMatrix();
}

void DebugCamera::Neutral::Update(){
    if(!(host_->input_->isPressKey(DIK_LALT) || host_->input_->isPressKey(DIK_RALT))){
        return;
    }
    if(host_->input_->isTriggerMouseButton(0) || host_->input_->isWheel()){
        host_->currentState_.reset(new TranslationState(host_));
        return;
    } else if(host_->input_->isTriggerMouseButton(1)){
        host_->currentState_.reset(new RotationState(host_));
        return;
    }
}

void DebugCamera::TranslationState::Update(){
    uint32_t state = 0;
    bool a         = host_->input_->isPreWheel();
    bool b         = host_->input_->isPressMouseButton(0);
    uint32_t c     = (host_->input_->isPressKey(DIK_LALT) | host_->input_->isPressKey(DIK_RALT));
    state          = (a)+(b * 2);
    state *= c;
    Vec3f velo = {};
    switch((TranslationType)state){
        case NONE:
            host_->currentState_.reset(new Neutral(host_));
            return;
        case Z_WHEEL:
            velo = {0.0f,0.0f,(float)host_->input_->getPreWheel() * 0.007f};
            break;
        case XY_MOUSEMOVE:
            velo = {host_->input_->getMouseVelocity() * 0.01f,0.0f};
            break;
        case XYZ_ALL:
            velo = {host_->input_->getMouseVelocity() * 0.01f,(float)host_->input_->getPreWheel() * 0.007f};
            break;
        default:
            break;
    }
    velo[Y] *= -1.0f;
    host_->cameraBuff_.translate += velo * MakeMatrix::RotateQuaternion(host_->cameraBuff_.rotate);
}

void DebugCamera::RotationState::Update(){
    if(!host_->input_->isPressMouseButton(1) || !(host_->input_->isPressKey(DIK_LALT) || host_->input_->isPressKey(DIK_RALT))){
        host_->currentState_.reset(new Neutral(host_));
        return;
    }

    // マウスの動きから回転量を取得
    Vec2f mouseVelocity = host_->input_->getMouseVelocity();
    float yaw           = mouseVelocity[X] * 0.01f; // Y軸回転（水平）
    float pitch         = mouseVelocity[Y] * 0.01f; // X軸回転（垂直）

    // クォータニオンを生成
    Quaternion yawRotation   = Quaternion::RotateAxisAngle({0.0f,1.0f,0.0f},yaw); // Y軸回転
    Quaternion pitchRotation = Quaternion::RotateAxisAngle({1.0f,0.0f,0.0f},pitch); // X軸回転

    // 現在の回転に新しい回転を適用
    host_->cameraBuff_.rotate    = pitchRotation * yawRotation * host_->cameraBuff_.rotate;
}
