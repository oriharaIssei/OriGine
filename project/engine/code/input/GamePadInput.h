#pragma once

/// input API
#include "include/IncludeInputAPI.h"

/// util
#include "util/globalVariables/SerializedField.h"

/// math
#include "math/Vector2.h"
#include <intsafe.h>
#include <stdint.h>

constexpr float kStickMax = static_cast<float>(SHORT_MAX);
constexpr float kStickMin = static_cast<float>(-SHORT_MAX);

enum class PadButton : uint32_t {
    UP         = XINPUT_GAMEPAD_DPAD_UP,
    DOWN       = XINPUT_GAMEPAD_DPAD_DOWN,
    LEFT       = XINPUT_GAMEPAD_DPAD_LEFT,
    RIGHT      = XINPUT_GAMEPAD_DPAD_RIGHT,
    START      = XINPUT_GAMEPAD_START,
    BACK       = XINPUT_GAMEPAD_BACK,
    L_THUMB    = XINPUT_GAMEPAD_LEFT_THUMB,
    R_THUMB    = XINPUT_GAMEPAD_RIGHT_THUMB,
    L_SHOULDER = XINPUT_GAMEPAD_LEFT_SHOULDER,
    R_SHOULDER = XINPUT_GAMEPAD_RIGHT_SHOULDER,
    A          = XINPUT_GAMEPAD_A,
    B          = XINPUT_GAMEPAD_B,
    X          = XINPUT_GAMEPAD_X,
    Y          = XINPUT_GAMEPAD_Y,

    // 仮想トリガーボタン（XInputに存在しない独自ビット）
    L_TRIGGER = 1u << 16,
    R_TRIGGER = 1u << 17,
};

static std::map<PadButton, std::string> padButtonNameMap = {
    {PadButton::UP, "UP"},
    {PadButton::DOWN, "DOWN"},
    {PadButton::LEFT, "LEFT"},
    {PadButton::RIGHT, "RIGHT"},
    {PadButton::START, "START"},
    {PadButton::BACK, "BACK"},
    {PadButton::L_THUMB, "L_THUMB"},
    {PadButton::R_THUMB, "R_THUMB"},
    {PadButton::L_SHOULDER, "L_SHOULDER"},
    {PadButton::R_SHOULDER, "R_SHOULDER"},
    {PadButton::A, "A"},
    {PadButton::B, "B"},
    {PadButton::X, "X"},
    {PadButton::Y, "Y"},
    {PadButton::L_TRIGGER, "L_TRIGGER"},
    {PadButton::R_TRIGGER, "R_TRIGGER"}};

constexpr uint32_t PAD_BUTTON_COUNT = 16;

/// <summary>
/// XInput対応ゲームパッド入力を管理するクラス
/// </summary>
class GamePadInput {
    friend class ReplayPlayer;

public:
    GamePadInput()  = default;
    ~GamePadInput() = default;

    GamePadInput(const GamePadInput&)            = delete;
    GamePadInput& operator=(const GamePadInput&) = delete;

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    void Finalize();

    /// <summary>
    /// ボタン状態をクリア
    /// </summary>
    void clearButtonStates() {
        buttonMask_     = 0;
        prevButtonMask_ = 0;
    }
    /// <summary>
    /// スティック状態をクリア
    /// </summary>
    void clearStickStates() {
        lStick_ = Vec2f();
        rStick_ = Vec2f();
    }
    /// <summary>
    /// トリガー状態をクリア
    /// </summary>
    void clearTriggerStates() {
        lTrigger_ = 0.0f;
        rTrigger_ = 0.0f;
    }

private:
    /// <summary>
    /// スティックの値を正規化して更新
    /// </summary>
    /// <param name="_state"></param>
    void UpdateStickValues(XINPUT_STATE _state);

private:
    // 仮想ボタンマスク
    uint32_t buttonMask_;
    uint32_t prevButtonMask_;

    SerializedField<float> deadZone_{"Input", "GamePad", "DeadZone"};
    SerializedField<float> triggerDeadZone_{"Input", "GamePad", "TriggerDeadZone"};

    Vec2f lStick_{};
    Vec2f rStick_{};

    float lTrigger_ = 0.0f;
    float rTrigger_ = 0.0f;

    bool isActive_ = false;

public:
    uint32_t getButtonMask() const { return buttonMask_; }
    uint32_t getPrevButtonMask() const { return prevButtonMask_; }

    /// <summary>
    /// ゲームパッドが有効か
    /// </summary>
    bool isActive() const { return isActive_; }

    /// <summary>
    /// ボタン押下状態
    /// </summary>
    bool isPress(uint32_t button) const { return (buttonMask_ & button); }
    bool isPress(PadButton button) const { return (buttonMask_ & static_cast<uint32_t>(button)); }

    /// <summary>
    /// ボタン押下瞬間
    /// </summary>
    bool isTrigger(uint32_t button) const { return (buttonMask_ & button) && !(prevButtonMask_ & button); }
    bool isTrigger(PadButton button) const { return (buttonMask_ & static_cast<uint32_t>(button)) && !(prevButtonMask_ & static_cast<uint32_t>(button)); }

    /// <summary>
    /// ボタン解放瞬間
    /// </summary>
    bool isRelease(uint32_t button) const { return !(buttonMask_ & button) && (prevButtonMask_ & button); }
    bool isRelease(PadButton button) const { return !(buttonMask_ & static_cast<uint32_t>(button)) && (prevButtonMask_ & static_cast<uint32_t>(button)); }

    /// <summary>
    /// 左スティックの現在速度
    /// </summary>
    const Vec2f& getLStick() const { return lStick_; }

    /// <summary>
    /// 右スティックの現在速度
    /// </summary>
    const Vec2f& getRStick() const { return rStick_; }

    /// <summary>
    /// 左右トリガー値
    /// </summary>
    float getLTrigger() const { return lTrigger_; }
    float getRTrigger() const { return rTrigger_; }
};
