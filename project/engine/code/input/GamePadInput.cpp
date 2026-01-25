#include "GamepadInput.h"

/// math
#include "math/mathEnv.h"
#include <cmath>

using namespace OriGine;

/// <summary>
/// ゲームパッドの初期化
/// </summary>
void GamepadInput::Initialize() {
    // 初期入力を取得しておく
    Update();

    if (!inputHistory_.empty()) {
        inputHistory_.clear();
    }
}

/// <summary>
/// 状態更新
/// </summary>
void GamepadInput::Update() {
    // XInput更新
    XINPUT_STATE state{};
    isActive_ = (XInputGetState(0, &state) == ERROR_SUCCESS);

    GamepadState currentState{};
    if (!isActive_) {
        inputHistory_.push_back(currentState);
    }
    // デジタルボタン
    currentState.buttonMask |= state.Gamepad.wButtons;

    // アナログトリガーを正規化
    float triggerDeadZoneVal = *triggerDeadZone_.GetValue();
    float triggerMax         = kTriggerMax - triggerDeadZoneVal;
    currentState.lTrigger    = (std::max)(static_cast<float>(state.Gamepad.bLeftTrigger) - triggerDeadZoneVal, 0.f) / triggerMax;
    currentState.rTrigger    = (std::max)(static_cast<float>(state.Gamepad.bRightTrigger) - triggerDeadZoneVal, 0.f) / triggerMax;

    UpdateVirtualTriggerButtons(currentState);
    UpdateStickValues(state, currentState);
    UpdateVirtualStickButtons(currentState);

    // 入力履歴に追加
    inputHistory_.push_front(currentState);
    // 履歴が多すぎたら削除
    if (inputHistory_.size() > kInputHistoryCount) {
        inputHistory_.pop_back();
    }
}

void GamepadInput::Finalize() {
    if (!inputHistory_.empty()) {
        inputHistory_.clear();
    }
}

void OriGine::GamepadInput::ClearHistory() {
    // 履歴を全てクリアし、空の状態を1つ積んでおく
    if (!inputHistory_.empty()) {
        inputHistory_.clear();
    }
    GamepadState emptyState{};
    inputHistory_.push_front(emptyState);
}

/// <summary>
/// スティックの値を正規化して更新
/// </summary>
void GamepadInput::UpdateStickValues(XINPUT_STATE _state, GamepadState& _currentState) {
    // deadZoneとdeadZoneを除去したあとの最大値を計算
    float deadZone = *deadZone_.GetValue();
    float stickMax = kStickMax - deadZone;

    // deadZoneを考慮して正規化するラムダ関数
    auto normalizeStick = [this, deadZone, stickMax](SHORT _x, SHORT _y) -> Vec2f {
        float realX = static_cast<float>(_x) / kStickMax;
        float realY = static_cast<float>(_y) / kStickMax;

        Vec2f result = Vec2f(0.0f, 0.0f);

        if (std::abs(realX) - deadZone >= kEpsilon) {
            float signX = std::signbit(realX) ? 1.0f : -1.0f;
            result[X]   = signX * (std::abs(realX) - deadZone) / stickMax;
        }
        if (std::abs(realX) - deadZone >= kEpsilon) {
            float signY = std::signbit(realY) ? 1.0f : -1.0f;
            result[Y]   = signY * (std::abs(realY) - deadZone) / stickMax;
        }

        return result;
    };

    _currentState.lStick = normalizeStick(_state.Gamepad.sThumbLX, _state.Gamepad.sThumbLY);
    _currentState.rStick = normalizeStick(_state.Gamepad.sThumbRX, _state.Gamepad.sThumbRY);
}

void GamepadInput::UpdateVirtualStickButtons(GamepadState& _currentState) {
    // 仮想左スティックボタン
    if (_currentState.lStick[Y] > kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::L_STICK_UP);
    }
    if (_currentState.lStick[Y] < -kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::L_STICK_DOWN);
    }
    if (_currentState.lStick[X] < -kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::L_STICK_LEFT);
    }
    if (_currentState.lStick[X] > kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::L_STICK_RIGHT);
    }
    // 仮想右スティックボタン
    if (_currentState.rStick[Y] > kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::R_STICK_UP);
    }
    if (_currentState.rStick[Y] < -kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::R_STICK_DOWN);
    }
    if (_currentState.rStick[X] < -kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::R_STICK_LEFT);
    }
    if (_currentState.rStick[X] > kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::R_STICK_RIGHT);
    }
}

void OriGine::GamepadInput::UpdateVirtualTriggerButtons(GamepadState& _currentState) {
    // アナログトリガーをボタン扱いに変換
    if (_currentState.lTrigger > kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::L_TRIGGER);
    }
    if (_currentState.rTrigger > kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::R_TRIGGER);
    }
}

// ==========================================
// 1. 基本的な状態取得
// ==========================================

const GamepadState& OriGine::GamepadInput::GetCurrentState() const {
    const auto* state = GetState(0);
    if (state) {
        return *state;
    }
    static GamepadState emptyState{};
    return emptyState;
}

bool GamepadInput::IsPress(GamepadButton _button) const {
    // 最新のフレーム (index 0) を見る
    const auto* current = GetState(0);
    if (!current) {
        return false;
    }

    return (current->buttonMask & static_cast<uint32_t>(_button)) != 0;
}

Vec2f GamepadInput::GetLeftStick() const {
    const auto* current = GetState(0);
    return current ? current->lStick : Vec2f{0.0f, 0.0f};
}

Vec2f GamepadInput::GetRightStick() const {
    const auto* current = GetState(0);
    return current ? current->rStick : Vec2f{0.0f, 0.0f};
}

float GamepadInput::GetLeftTrigger() const {
    const auto* current = GetState(0);
    return current ? current->lTrigger : 0.0f;
}

float GamepadInput::GetRightTrigger() const {
    const auto* current = GetState(0);
    return current ? current->rTrigger : 0.0f;
}

// ==========================================
// 2. エッジ検出 (履歴比較)
// ==========================================

bool GamepadInput::IsTrigger(GamepadButton _button) const {
    return IsTrigger(static_cast<uint32_t>(_button));
}

bool OriGine::GamepadInput::IsTrigger(uint32_t _buttonMask) const {
    // 比較には最低2フレーム必要
    const auto* current = GetState(0);
    const auto* prev    = GetState(1);

    if (!current || !prev) {
        return false;
    }

    bool isDownNow  = (current->buttonMask & _buttonMask) != 0;
    bool isDownPrev = (prev->buttonMask & _buttonMask) != 0;

    // 「今は押されている」かつ「前は押されていない」
    if (isDownNow && !isDownPrev) {
        return true;
    }

    return false;
}

bool GamepadInput::IsRelease(GamepadButton _button) const {
    return IsRelease(static_cast<uint32_t>(_button));
}

bool OriGine::GamepadInput::IsRelease(uint32_t _buttonMask) const {
    const auto* current = GetState(0);
    const auto* prev    = GetState(1);

    if (!current || !prev) {
        return false;
    }

    bool isDownNow  = (current->buttonMask & _buttonMask) != 0;
    bool isDownPrev = (prev->buttonMask & _buttonMask) != 0;

    // 「今は押されていない」かつ「前は押されていた」
    return !isDownNow && isDownPrev;
}

// ==========================================
// 3. 履歴を利用した拡張判定
// ==========================================
bool GamepadInput::WasPressedRecently(GamepadButton _button, size_t _framesToCheck) const {
    // 履歴サイズを超えないように制限
    size_t checkCount = (std::min)(inputHistory_.size(), _framesToCheck);
    uint32_t mask  = static_cast<uint32_t>(_button);

    // 過去 N フレームを走査
    for (int i = 0; i < checkCount; ++i) {
        if ((inputHistory_[i].buttonMask & mask) != 0) {
            return true; // 押されていた瞬間が見つかった
        }
    }
    return false;
}

bool OriGine::GamepadInput::WasTriggeredRecently(GamepadButton _button, size_t _framesToCheck) const {
    // 履歴サイズを超えないように制限
    size_t checkCount = (std::min)(inputHistory_.size() - 1, _framesToCheck - 1);
    uint32_t mask  = static_cast<uint32_t>(_button);
    // 過去 N フレームを走査
    for (int i = 0; i < checkCount; ++i) {
        bool isDownNow  = (inputHistory_[i].buttonMask & mask) != 0;
        bool isDownPrev = (inputHistory_[i + 1].buttonMask & mask) != 0;
        // 「今は押されている」かつ「前は押されていない」
        if (isDownNow && !isDownPrev) {
            return true; // 押された瞬間が見つかった
        }
    }
    return false;
}

bool OriGine::GamepadInput::WasReleasedRecently(GamepadButton _button, size_t _framesToCheck) const {
    // 履歴サイズを超えないように制限
    size_t checkCount = (std::min)(inputHistory_.size() - 1, _framesToCheck - 1);
    uint32_t mask  = static_cast<uint32_t>(_button);
    // 過去 N フレームを走査
    for (int i = 0; i < checkCount; ++i) {
        bool isDownNow  = (inputHistory_[i].buttonMask & mask) != 0;
        bool isDownPrev = (inputHistory_[i + 1].buttonMask & mask) != 0;
        // 「今は押されていない」かつ「前は押されていた」
        if (!isDownNow && isDownPrev) {
            return true; // 離された瞬間が見つかった
        }
    }
    return false;
}

bool GamepadInput::IsPressedDuration(GamepadButton _button, size_t _frames) const {
    // 履歴が足りなければ false
    if (inputHistory_.size() < static_cast<size_t>(_frames)) {
        return false;
    }

    uint32_t mask = static_cast<uint32_t>(_button);

    // 指定フレームの間、ずっと押され続けているかチェック
    for (int i = 0; i < _frames; ++i) {
        if ((inputHistory_[i].buttonMask & mask) == 0) {
            return false; // 途中で離している
        }
    }
    return true;
}
