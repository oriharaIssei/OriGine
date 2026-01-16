#include "KeyboardInput.h"

/// stl
#include <cassert>
/// engine
// input
#include "MouseInput.h"

using namespace OriGine;

void KeyboardInput::Initialize(IDirectInput8* _directInput, HWND _hwnd) {
    assert(_directInput);

    // キーボードデバイスを作成
    HRESULT hr = _directInput->CreateDevice(GUID_SysKeyboard, &keyboard_, nullptr);
    assert(SUCCEEDED(hr) && "Failed to create keyboard device.");

    // データフォーマットを設定
    hr = keyboard_->SetDataFormat(&c_dfDIKeyboard);
    assert(SUCCEEDED(hr));

    // 協調レベル設定（フォアグラウンド・非排他）
    hr = keyboard_->SetCooperativeLevel(_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    assert(SUCCEEDED(hr));

    // 入力開始
    keyboard_->Acquire();
}

void KeyboardInput::Update() {
    if (!keyboard_) {
        return;
    }

    // 前回の状態を保存
    prevKeys_ = keys_;

    // 現在のキー状態を取得
    HRESULT hr = keyboard_->GetDeviceState(static_cast<DWORD>(keys_.size()), keys_.data());
    if (FAILED(hr)) {
        // 一時的にフォーカスを失ったときは再取得
        keyboard_->Acquire();
        keyboard_->GetDeviceState(static_cast<DWORD>(keys_.size()), keys_.data());
    }
}

void KeyboardInput::Finalize() {
    if (keyboard_) {
        keyboard_->Unacquire();
        keyboard_.Reset();
    }
    ClearKeyStates();
}
