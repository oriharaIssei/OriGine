#include "GamePadInput.h"

/// math
#include "math/mathEnv.h"
#include <cmath>

/// <summary>
/// ゲームパッドの初期化
/// </summary>
void GamePadInput::Initialize() {
    ZeroMemory(&state_, sizeof(XINPUT_STATE));

    // 接続確認
    isActive_ = (XInputGetState(0, &state_) == ERROR_SUCCESS);
    UpdateStickValues();
}

/// <summary>
/// 状態更新
/// </summary>
void GamePadInput::Update() {
    prevButtonMask_ = buttonMask_;

    // XInput更新
    isActive_   = (XInputGetState(0, &state_) == ERROR_SUCCESS);
    buttonMask_ = 0;

    if (isActive_) {
        // デジタルボタン
        buttonMask_ |= state_.Gamepad.wButtons;

        // アナログトリガーをボタン扱いに変換
        if (state_.Gamepad.bLeftTrigger / *triggerDeadZone_.getValue() > kEpsilon) {
            buttonMask_ |= static_cast<uint32_t>(PadButton::L_TRIGGER);
        }
        if (state_.Gamepad.bRightTrigger / *triggerDeadZone_.getValue() > kEpsilon) {
            buttonMask_ |= static_cast<uint32_t>(PadButton::R_TRIGGER);
        }

        UpdateStickValues();
    } else {
        buttonMask_ = 0;
    }
}

/// <summary>
/// スティックの値を正規化して更新
/// </summary>
void GamePadInput::UpdateStickValues() {
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

    lStick_ = normalizeStick(state_.Gamepad.sThumbLX, state_.Gamepad.sThumbLY);
    rStick_ = normalizeStick(state_.Gamepad.sThumbRX, state_.Gamepad.sThumbRY);
}
