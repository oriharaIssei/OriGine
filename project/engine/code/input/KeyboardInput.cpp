#include "KeyboardInput.h"

/// stl
#include <algorithm>

/// engine
// input
#include "MouseInput.h"

#include "logger/Logger.h"

using namespace OriGine;

void KeyboardInput::Initialize(IDirectInput8* _directInput, HWND _hwnd) {
    assert(_directInput);

    // キーボードデバイスを作成
    HRESULT hr = _directInput->CreateDevice(GUID_SysKeyboard, &keyboard_, nullptr);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create keyboard device.");
        return;
    }

    // データフォーマットを設定
    hr = keyboard_->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to set keyboard data format.");
        return;
    }

    // 協調レベル設定（フォアグラウンド・非排他）
    hr = keyboard_->SetCooperativeLevel(_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to set keyboard cooperative level.");
        return;
    }

    // 入力開始
    keyboard_->Acquire();
}

void KeyboardInput::Update() {
    if (!keyboard_) {
        return;
    }

    KeyboardState state;

    // 現在のキー状態を取得
    HRESULT hr = keyboard_->GetDeviceState(static_cast<DWORD>(state.keys.size()), state.keys.data());
    if (FAILED(hr)) {
        // 一時的にフォーカスを失ったときは再取得
        keyboard_->Acquire();
        keyboard_->GetDeviceState(static_cast<DWORD>(state.keys.size()), state.keys.data());
    }

    // 履歴に追加
    inputHistory_.push_front(state);

    // 履歴が多すぎたら削除
    if (inputHistory_.size() > kInputHistoryCount) {
        inputHistory_.pop_back();
    }
}

void KeyboardInput::Finalize() {
    if (!inputHistory_.empty()) {
        inputHistory_.clear();
    }

    if (keyboard_) {
        keyboard_->Unacquire();
        keyboard_.Reset();
    }
}

void OriGine::KeyboardInput::ClearHistory() {
    // 履歴を全てクリアし、空の状態を1つ積んでおく
    inputHistory_.clear();
    KeyboardState emptyState{};
    emptyState.keys.fill(0);
    inputHistory_.push_front(emptyState);
}

const KeyboardState* OriGine::KeyboardInput::GetState(size_t _historyIndex) const {
    if (_historyIndex >= inputHistory_.size()) {
        return nullptr;
    }
    return &inputHistory_[_historyIndex];
}

const ::std::array<BYTE, KEY_COUNT>& OriGine::KeyboardInput::GetKeyStates() const {
    static std::array<BYTE, KEY_COUNT> emptyKeys{};
    const auto* current = GetState(0);
    return current ? current->keys : emptyKeys;
}

bool KeyboardInput::IsPress(uint32_t _key) const {
    if (_key >= KEY_COUNT) {
        return false;
    }

    const auto* current = GetState(0);
    if (!current) {
        return false;
    }

    // DirectInputは最上位ビット(0x80)で判定
    return (current->keys[_key] & 0x80) != 0;
}

bool KeyboardInput::IsTrigger(uint32_t _key) const {
    if (_key >= KEY_COUNT) {
        return false;
    }

    const auto* current = GetState(0);
    const auto* prev    = GetState(1);
    if (!current || !prev) {
        return false;
    }

    bool isDownNow  = (current->keys[_key] & 0x80) != 0;
    bool isDownPrev = (prev->keys[_key] & 0x80) != 0;

    return isDownNow && !isDownPrev;
}

bool KeyboardInput::IsRelease(uint32_t _key) const {
    if (_key >= KEY_COUNT) {
        return false;
    }

    const auto* current = GetState(0);
    const auto* prev    = GetState(1);
    if (!current || !prev) {
        return false;
    }

    bool isDownNow  = (current->keys[_key] & 0x80) != 0;
    bool isDownPrev = (prev->keys[_key] & 0x80) != 0;

    return !isDownNow && isDownPrev;
}

bool KeyboardInput::WasPressedRecently(Key _key, size_t _framesToCheck) const {
    uint32_t keyIdx = static_cast<uint32_t>(_key);
    if (keyIdx >= KEY_COUNT) {
        return false;
    }

    // 履歴バッファサイズとチェック要求数の小さい方までループ
    size_t loopCount = (std::min)(inputHistory_.size(), _framesToCheck);

    for (int i = 0; i < loopCount; ++i) {
        // 過去のどこかで押されていたら true
        if ((inputHistory_[i].keys[keyIdx] & 0x80) != 0) {
            return true;
        }
    }
    return false;
}

bool OriGine::KeyboardInput::WasTriggeredRecently(Key _key, size_t _framesToCheck) const {
    uint32_t keyIdx = static_cast<uint32_t>(_key);
    if (keyIdx >= KEY_COUNT) {
        return false;
    }
    // 履歴バッファサイズとチェック要求数の小さい方までループ
    size_t loopCount = (std::min)(inputHistory_.size(), _framesToCheck);
    for (int i = 1; i < loopCount; ++i) {
        const auto& current = inputHistory_[i];
        const auto& prev    = inputHistory_[i - 1];
        bool isDownNow      = (current.keys[keyIdx] & 0x80) != 0;
        bool isDownPrev     = (prev.keys[keyIdx] & 0x80) != 0;
        if (isDownNow && !isDownPrev) {
            return true;
        }
    }
    return false;
}

bool OriGine::KeyboardInput::WasReleasedRecently(Key _key, size_t _framesToCheck) const {
    uint32_t keyIdx = static_cast<uint32_t>(_key);
    if (keyIdx >= KEY_COUNT) {
        return false;
    }

    // 履歴バッファサイズとチェック要求数の小さい方までループ
    size_t loopCount = (std::min)(inputHistory_.size(), _framesToCheck);
    for (int i = 1; i < loopCount; ++i) {
        const auto& current = inputHistory_[i];
        const auto& prev    = inputHistory_[i - 1];
        bool isDownNow      = (current.keys[keyIdx] & 0x80) != 0;
        bool isDownPrev     = (prev.keys[keyIdx] & 0x80) != 0;
        if (!isDownNow && isDownPrev) {
            return true;
        }
    }

    return false;
}
