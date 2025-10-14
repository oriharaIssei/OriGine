#include "Input.h"

#include "Engine.h"
#include "winApp/WinApp.h"

Input* Input::getInstance() {
    static Input instance;
    return &instance;
}

void Input::Initialize() {
    // DirectInput の初期化
    DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);

    // キーボードの初期化
    directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, nullptr);
    keyboard_->SetDataFormat(&c_dfDIKeyboard);
    keyboard_->SetCooperativeLevel(nullptr, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    keyboard_->Acquire();

    // マウスの初期化
    directInput_->CreateDevice(GUID_SysMouse, &mouse_, nullptr);
    mouse_->SetDataFormat(&c_dfDIMouse2);
    mouse_->SetCooperativeLevel(nullptr, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    mouse_->Acquire();

    // ゲームパッドの初期化
    ZeroMemory(&padState_, sizeof(XINPUT_STATE));
    ZeroMemory(&prePadState_, sizeof(XINPUT_STATE));
    isPadActive_ = (XInputGetState(0, &padState_) == ERROR_SUCCESS);
}

void Input::Finalize() {
    if (keyboard_) {
        keyboard_->Unacquire();
        keyboard_.Reset();
    }
    if (mouse_) {
        mouse_->Unacquire();
        mouse_.Reset();
    }
}

void Input::Update() {
    // キーボードの状態を更新
    memcpy(preKeys_, keys_, sizeof(keys_));
    keyboard_->GetDeviceState(sizeof(keys_), (LPVOID)&keys_);

    // マウスの状態を更新
    preMouseState_ = currentMouseState_;
    mouse_->GetDeviceState(sizeof(DIMOUSESTATE2), &currentMouseState_);

    // マウスの位置を更新
    preMousePos_ = currentMousePos_;
    POINT mousePoint;
    GetCursorPos(&mousePoint);
    ScreenToClient(Engine::getInstance()->getWinApp()->getHwnd(), &mousePoint);
    currentMousePos_ = Vec2f(static_cast<float>(mousePoint.x), static_cast<float>(mousePoint.y));
    mouseVelocity_   = preMousePos_ - currentMousePos_;

    if (virtualMouseIsSynchronizedWithClientMouse_) {
        virtualMousePos_ = currentMousePos_;
    }

    // ゲームパッドの状態を更新
    prePadState_ = padState_;
    isPadActive_ = (XInputGetState(0, &padState_) == ERROR_SUCCESS);
    if (isPadActive_) {
        // スティックの速度を更新
        preLStickVelocity_     = currentLStickVelocity_;
        currentLStickVelocity_ = Vec2f(
            static_cast<float>(padState_.Gamepad.sThumbLX) / stickMax_,
            static_cast<float>(padState_.Gamepad.sThumbLY) / stickMax_);
        // デッドゾーンを適用
        if (currentLStickVelocity_.lengthSq() < deadZone_ * deadZone_) {
            currentLStickVelocity_ = {0.0f, 0.0f};
        }

        preRStickVelocity_     = currentRStickVelocity_;
        currentRStickVelocity_ = Vec2f(
            static_cast<float>(padState_.Gamepad.sThumbRX) / stickMax_,
            static_cast<float>(padState_.Gamepad.sThumbRY) / stickMax_);
        // デッドゾーンを適用
        if (currentRStickVelocity_.lengthSq() < deadZone_ * deadZone_) {
            currentRStickVelocity_ = {0.0f, 0.0f};
        }
    }
}

void Input::SetMousePos(const Vec2f& _fixedPos) {
    SetCursorPos(static_cast<int>(_fixedPos[X]), static_cast<int>(_fixedPos[Y]));
}

void Input::SetMousePos(const Vec2d& _fixedPos) {
    SetCursorPos(static_cast<int>(_fixedPos[X]), static_cast<int>(_fixedPos[Y]));
}

void Input::ShowMouseCursor(bool _isShow) {
    ShowCursor(_isShow);
}
