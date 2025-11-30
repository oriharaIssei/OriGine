#include "MouseInput.h"

#include <cassert>
#include <windows.h>

void MouseInput::Initialize(IDirectInput8* directInput, HWND hwnd) {
    assert(directInput);

    hwnd_ = hwnd;

    // マウスデバイスを作成
    HRESULT hr = directInput->CreateDevice(GUID_SysMouse, &mouse_, nullptr);
    assert(SUCCEEDED(hr) && "Failed to create mouse device.");

    // データフォーマットを設定
    hr = mouse_->SetDataFormat(&c_dfDIMouse2);
    assert(SUCCEEDED(hr));

    // 協調レベル設定
    hr = mouse_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    assert(SUCCEEDED(hr));

    mouse_->Acquire();

    // 初期座標の取得
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(hwnd, &p);
    pos_     = Vec2f(static_cast<float>(p.x), static_cast<float>(p.y));
    prevPos_ = pos_;
}

void MouseInput::Update() {
    if (!mouse_) {
        return;
    }

    DIMOUSESTATE2 current{};

    // デバイスの状態を取得
    HRESULT hr = mouse_->GetDeviceState(sizeof(DIMOUSESTATE2), &current);
    if (FAILED(hr)) {
        mouse_->Acquire();
        mouse_->GetDeviceState(sizeof(DIMOUSESTATE2), &current);
    }

    // マウス座標の更新
    prevPos_ = pos_;
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(hwnd_, &p);
    pos_ = Vec2f(static_cast<float>(p.x), static_cast<float>(p.y));
    // virtual position を更新
    virtualPos_ = pos_;

    // 移動量を算出
    velocity_ = pos_ - prevPos_;

    // ボタン状態の更新
    prevButtonStates_ = currentButtonStates_;
    for (size_t i = 0; i < currentButtonStates_.size(); ++i) {
        currentButtonStates_[i] = (current.rgbButtons[i] & 0x80) != 0;
    }

    // ホイール状態の更新
    prevWheelDelta_    = currentWheelDelta_;
    currentWheelDelta_ = static_cast<int32_t>(current.lZ);
}

void MouseInput::SetPosition(const Vec2f& pos) {
    if (mouse_) {
        POINT screenPos;
        screenPos.x = static_cast<LONG>(pos[X]);
        screenPos.y = static_cast<LONG>(pos[Y]);

        ClientToScreen(hwnd_, &screenPos);
        SetCursorPos(screenPos.x, screenPos.y);
    }

    pos_        = pos;
    virtualPos_ = pos;
}

void MouseInput::ShowCursor(bool show) {
    // カーソルの表示・非表示を切り替え
   
    if (isCursorVisible_ == show) {
        return;
    }
    isCursorVisible_ = show;
    ShowCursor(show);
}

void MouseInput::Finalize() {
    if (mouse_) {
        mouse_->Unacquire();
        mouse_.Reset();
    }
}

uint32_t MouseInput::ButtonStateToBitmask() const {
    uint32_t mask = 0;
    for (size_t i = 0; i < MOUSE_BUTTON_COUNT; ++i) {
        if (currentButtonStates_[i]) {
            mask |= (1u << i);
        }
    }
    return mask;
}
