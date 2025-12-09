#include "GamepadInput.h"

/// math
#include "math/mathEnv.h"
#include <cmath>

/// <summary>
/// ゲームパッドの初期化
/// </summary>
void GamepadInput::Initialize() {
    // 初期入力を取得しておく
    Update();

    prevButtonMask_ = buttonMask_;
}

/// <summary>
/// 状態更新
/// </summary>
void GamepadInput::Update() {
    prevButtonMask_ = buttonMask_;

    // XInput更新
    XINPUT_STATE state{};
    isActive_   = (XInputGetState(0, &state) == ERROR_SUCCESS);
    buttonMask_ = 0;

    if (isActive_) {
        // デジタルボタン
        buttonMask_ |= state.Gamepad.wButtons;

        // アナログトリガーをボタン扱いに変換
        if (state.Gamepad.bLeftTrigger / *triggerDeadZone_.GetValue() > kEpsilon) {
            buttonMask_ |= static_cast<uint32_t>(PadButton::L_TRIGGER);
        }
        if (state.Gamepad.bRightTrigger / *triggerDeadZone_.GetValue() > kEpsilon) {
            buttonMask_ |= static_cast<uint32_t>(PadButton::R_TRIGGER);
        }

        // スティック値更新
        lTrigger_ = static_cast<float>(state.Gamepad.bLeftTrigger) / *triggerDeadZone_.GetValue();
        rTrigger_ = static_cast<float>(state.Gamepad.bRightTrigger) / *triggerDeadZone_.GetValue();

        UpdateStickValues(state);
    } else {
        buttonMask_ = 0;
    }
}

void GamepadInput::Finalize() {
    buttonMask_     = 0;
    prevButtonMask_ = 0;

    lStick_ = Vec2f();
    rStick_ = Vec2f();

    lTrigger_ = 0.0f;
    rTrigger_ = 0.0f;

    ClearButtonStates();
    ClearStickStates();
    ClearTriggerStates();
}

/// <summary>
/// スティックの値を正規化して更新
/// </summary>
void GamepadInput::UpdateStickValues(XINPUT_STATE _state) {
    auto normalizeStick = [this](SHORT x, SHORT y) -> Vec2f {
        Vec2f stick{
            static_cast<float>(x) / kStickMax,
            static_cast<float>(y) / kStickMax,
        };

        // デッドゾーンを適用
        if (stick.lengthSq() < deadZone_ * deadZone_) {
            stick = {0.0f, 0.0f};
        }
        return stick;
    };

    lStick_ = normalizeStick(_state.Gamepad.sThumbLX, _state.Gamepad.sThumbLY);
    rStick_ = normalizeStick(_state.Gamepad.sThumbRX, _state.Gamepad.sThumbRY);
}
