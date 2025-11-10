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
    if (!mouse_)
        return;

    prev_ = current_;

    // デバイスの状態を取得
    HRESULT hr = mouse_->GetDeviceState(sizeof(DIMOUSESTATE2), &current_);
    if (FAILED(hr)) {
        mouse_->Acquire();
        mouse_->GetDeviceState(sizeof(DIMOUSESTATE2), &current_);
    }

    // マウス座標の更新
    prevPos_ = pos_;
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(hwnd_, &p);
    pos_ = Vec2f(static_cast<float>(p.x), static_cast<float>(p.y));

    // 移動量を算出
    velocity_ = pos_ - prevPos_;

    // virtual position を更新
    virtualPos_ = pos_;
}

void MouseInput::setPosition(const Vec2f& pos) {
    POINT screenPos;
    screenPos.x = static_cast<LONG>(pos[X]);
    screenPos.y = static_cast<LONG>(pos[Y]);

    ClientToScreen(hwnd_, &screenPos);
    SetCursorPos(screenPos.x, screenPos.y);

    pos_        = pos;
    virtualPos_ = pos;
}

void MouseInput::ShowCursor(bool show) {
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
