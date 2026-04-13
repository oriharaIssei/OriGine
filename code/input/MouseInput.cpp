#include "MouseInput.h"

/// microsoft
#include <windows.h>
/// stl
#include <cassert>

/// engine
#include "EngineConfig.h"
#include "logger/Logger.h"

using namespace OriGine;

void MouseInput::Initialize(IDirectInput8* _directInput, HWND _hwnd) {
    hwnd_ = _hwnd;

    // デバイス生成
    HRESULT hr = _directInput->CreateDevice(GUID_SysMouse, &mouse_, NULL);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create mouse device.");
        return;
    }

    // データフォーマット設定 (DIMOUSESTATE2: 8ボタン対応)
    hr = mouse_->SetDataFormat(&c_dfDIMouse2);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to set mouse data format.");
        return;
    }

    // 協調レベル設定 (バックグラウンドでも取得するか、フォアグラウンドのみか)
    // 通常は DISCL_FOREGROUND | DISCL_NONEXCLUSIVE
    hr = mouse_->SetCooperativeLevel(hwnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to set mouse cooperative level.");
        return;
    }

    // デバイス取得開始
    mouse_->Acquire();

    // 初期状態を履歴に入れておく（クラッシュ防止）
    MouseState initialState{};
    inputHistory_.push_front(initialState);
}

void MouseInput::Finalize() {
    if (!inputHistory_.empty()) {
        inputHistory_.clear();
    }

    if (mouse_) {
        mouse_->Unacquire();
    }
    mouse_.Reset();
}

void MouseInput::Update() {
    if (!mouse_) {
        return;
    }

    // デバイス状態の取得
    DIMOUSESTATE2 diMouseState{};
    HRESULT hr = mouse_->GetDeviceState(sizeof(DIMOUSESTATE2), &diMouseState);

    // 取得失敗時の再取得 (ロスト対策)
    if (FAILED(hr)) {
        mouse_->Acquire();
        hr = mouse_->GetDeviceState(sizeof(DIMOUSESTATE2), &diMouseState);
        if (FAILED(hr)) {
            // それでもダメならクリアして終わる
            MouseState emptyState{};

            // 位置だけは前回の値を維持するケースが多いが、ここではクリア
            // 必要なら inputHistory_.front().mousePos を継承する
            if (!inputHistory_.empty()) {
                emptyState.mousePos = inputHistory_.front().mousePos;
            }
            inputHistory_.push_front(emptyState);
            if (inputHistory_.size() > Config::Input::kHistoryCount) {
                inputHistory_.pop_back();
            }
            return;
        }
    }

    // --- 新しいステートの作成 ---
    MouseState newState{};

    // 1. 位置情報の計算
    // DirectInputの lX, lY は「移動量」だが、ここでは「スクリーン座標」を計算して保持する
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(hwnd_, &p);
    newState.mousePos = Vec2f(static_cast<float>(p.x), static_cast<float>(p.y));

    // 2. 移動量 (Velocity)
    const auto* prevState = GetState(1);
    if (prevState) {
        newState.velocity = newState.mousePos - prevState->mousePos;
    } else {
        newState.velocity = Vec2f(0.0f, 0.0f);
    }

    // 3. ホイール
    newState.wheelDelta = diMouseState.lZ;

    // 4. ボタン状態のビットマスク化
    newState.buttonData = CreateButtonBitmask(diMouseState);

    // --- 履歴に追加 ---
    inputHistory_.push_front(newState);
    if (inputHistory_.size() > Config::Input::kHistoryCount) {
        inputHistory_.pop_back();
    }
}

uint32_t MouseInput::CreateButtonBitmask(const DIMOUSESTATE2& _diState) const {
    uint32_t mask = 0;
    // DirectInputでは最上位ビットが立っていれば押されている (0x80)
    if (_diState.rgbButtons[0] & 0x80) {
        mask |= static_cast<uint32_t>(MouseButton::LEFT);
    }
    if (_diState.rgbButtons[1] & 0x80) {
        mask |= static_cast<uint32_t>(MouseButton::RIGHT);
    }
    if (_diState.rgbButtons[2] & 0x80) {
        mask |= static_cast<uint32_t>(MouseButton::MIDDLE);
    }
    if (_diState.rgbButtons[3] & 0x80) {
        mask |= static_cast<uint32_t>(MouseButton::BTN_3);
    }
    if (_diState.rgbButtons[4] & 0x80) {
        mask |= static_cast<uint32_t>(MouseButton::BTN_4);
    }
    if (_diState.rgbButtons[5] & 0x80) {
        mask |= static_cast<uint32_t>(MouseButton::BTN_5);
    }
    if (_diState.rgbButtons[6] & 0x80) {
        mask |= static_cast<uint32_t>(MouseButton::BTN_6);
    }
    if (_diState.rgbButtons[7] & 0x80) {
        mask |= static_cast<uint32_t>(MouseButton::BTN_7);
    }

    return mask;
}

// ==========================================
// アクセッサ
// ==========================================

void OriGine::MouseInput::ClearHistory() {
    // 履歴を全てクリアし、空の状態を1つ積んでおく
    inputHistory_.clear();
    MouseState emptyState{};
    inputHistory_.push_front(emptyState);
}

const MouseState* MouseInput::GetState(size_t _historyIndex) const {
    if (_historyIndex >= inputHistory_.size()) {
        return nullptr;
    }
    return &inputHistory_[_historyIndex];
}

MouseState* OriGine::MouseInput::GetStateRef(size_t _historyIndex) {
    if (_historyIndex >= inputHistory_.size()) {
        return nullptr;
    }
    return &inputHistory_[_historyIndex];
}

const MouseState& OriGine::MouseInput::GetCurrentState() const {
    static const MouseState emptyState{};
    const auto* current = GetState(0);
    if (!current) {
        return emptyState;
    }
    return *current;
}

bool MouseInput::IsPress(MouseButton _button) const {
    const auto* current = GetState(0);
    if (!current) {
        return false;
    }
    return (current->buttonData & static_cast<uint32_t>(_button)) != 0;
}

bool MouseInput::IsTrigger(MouseButton _button) const {
    const auto* current = GetState(0);
    const auto* prev    = GetState(1);
    if (!current || !prev) {
        return false;
    }

    uint32_t mask = static_cast<uint32_t>(_button);
    return ((current->buttonData & mask) != 0) && ((prev->buttonData & mask) == 0);
}

bool MouseInput::IsRelease(MouseButton _button) const {
    const auto* current = GetState(0);
    const auto* prev    = GetState(1);
    if (!current || !prev) {
        return false;
    }

    uint32_t mask = static_cast<uint32_t>(_button);
    return ((current->buttonData & mask) == 0) && ((prev->buttonData & mask) != 0);
}

Vec2f MouseInput::GetPosition() const {
    const auto* current = GetState(0);
    return current ? current->mousePos : Vec2f(0.0f, 0.0f);
}

void MouseInput::SetPosition(const Vec2f& _pos) {
    if (mouse_) {
        POINT screenPos;
        screenPos.x = static_cast<LONG>(_pos[X]);
        screenPos.y = static_cast<LONG>(_pos[Y]);

        ClientToScreen(hwnd_, &screenPos);
        SetCursorPos(screenPos.x, screenPos.y);

        auto* current = GetStateRef(0);
        if (current) {
            current->mousePos = _pos;
            virtualPos_       = _pos;
        }
    }
}

Vec2f MouseInput::GetVelocity() const {
    const auto* current = GetState(0);
    return current ? current->velocity : Vec2f(0.0f, 0.0f);
}

Vec2f OriGine::MouseInput::CalculateVelocityBetween(size_t _recentIdx, size_t _oldIdx) const {
    // インデックスの正当性チェック
    // 古い方が数値として大きくあるべき
    if (_recentIdx >= _oldIdx) {
        return Vec2f(0.0f, 0.0f);
    }

    const auto* stateRecent = GetState(_recentIdx);
    const auto* stateOld    = GetState(_oldIdx);

    if (!stateRecent || !stateOld) {
        return Vec2f(0.0f, 0.0f);
    }

    // (新しい座標 - 古い座標) = 移動ベクトル
    Vec2f displacement = stateRecent->mousePos - stateOld->mousePos;

    // 経過フレーム数 (正の値にする)
    float frameDuration = static_cast<float>(_oldIdx - _recentIdx);

    return displacement / frameDuration;
}

Vec2f OriGine::MouseInput::GetAverageVelocity(size_t _from, size_t _historyRange) const {
    const auto* stateFrom = GetState(_from);
    if (_historyRange <= 0) {
        return Vec2f(0.0f, 0.0f);
    }

    Vec2f velocity;
    for (size_t to = 0; to < _historyRange; ++to) {
        const auto* stateTo = GetState(to);
        if (!stateTo || !stateFrom) {
            break;
        }
        velocity += stateTo->mousePos - stateFrom->mousePos;
        stateFrom = stateTo;
    }

    velocity = velocity.normalize() * (velocity.length() / static_cast<float>(_historyRange));

    return velocity;
}

int32_t MouseInput::GetWheelDelta() const {
    const auto* current = GetState(0);
    return current ? current->wheelDelta : 0;
}

int32_t OriGine::MouseInput::WheelAverageDelta(size_t _from, size_t _historyRange) const {
    const auto* stateFrom = GetState(_from);
    if (!stateFrom || _historyRange <= 0) {
        return 0;
    }
    int32_t totalDelta = 0;
    for (size_t to = 0; to < _historyRange; ++to) {
        const auto* stateTo = GetState(to);
        if (!stateTo || !stateFrom) {
            break;
        }
        totalDelta += stateTo->wheelDelta;
        stateFrom = stateTo;
    }
    return totalDelta / static_cast<int32_t>(_historyRange);
}

bool OriGine::MouseInput::WasWheelMovedInHistory(size_t _historyRange) const {
    for (size_t i = 0; i < _historyRange; ++i) {
        const auto* state = GetState(i);
        if (!state) {
            break;
        }
        if (state->wheelDelta != 0) {
            return true;
        }
    }
    return false;
}

bool OriGine::MouseInput::WasWheelUpInHistory(size_t _historyRange) const {
    for (size_t i = 0; i < _historyRange; ++i) {
        const auto* state = GetState(i);
        if (!state) {
            break;
        }
        if (state->wheelDelta > 0) {
            return true;
        }
    }
    return false;
}

bool OriGine::MouseInput::WasWheelDownInHistory(size_t _historyRange) const {
    for (size_t i = 0; i < _historyRange; ++i) {
        const auto* state = GetState(i);
        if (!state) {
            break;
        }
        if (state->wheelDelta < 0) {
            return true;
        }
    }
    return false;
}

void MouseInput::ShowCursor(bool _show) {
    // Windows APIのShowCursorはカウンタ管理
    // 現在の表示状態と同じ要求は無視する
    // 同じ要求を繰り返すとカウンタが増減し続けてしまうため
    if (isCursorVisible_ == _show) {
        return;
    }

    ::ShowCursor(_show ? TRUE : FALSE);
    isCursorVisible_ = _show;
}
