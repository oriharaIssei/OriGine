#pragma once

/// input API
#include "include/IncludeInputAPI.h"

/// util
#include "util/globalVariables/SerializedField.h"

/// math
#include "math/Vector2.h"
#include <intsafe.h>
#include <stdint.h>

namespace OriGine {

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
class GamepadInput {
    friend class ReplayPlayer;

public:
    GamepadInput()  = default;
    ~GamepadInput() = default;

    GamepadInput(const GamepadInput&)            = delete;
    GamepadInput& operator=(const GamepadInput&) = delete;

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 解放処理
    /// </summary>
    void Finalize();

    /// <summary>
    /// ボタン状態をクリア
    /// </summary>
    void ClearButtonStates() {
        buttonMask_     = 0;
        prevButtonMask_ = 0;
    }
    /// <summary>
    /// スティック状態をクリア
    /// </summary>
    void ClearStickStates() {
        lStick_ = Vec2f();
        rStick_ = Vec2f();
    }
    /// <summary>
    /// トリガー状態をクリア
    /// </summary>
    void ClearTriggerStates() {
        lTrigger_ = 0.0f;
        rTrigger_ = 0.0f;
    }

private:
    /// <summary>
    /// スティックの値を正規化して更新
    /// </summary>
    /// <param name="_state">XInput状態</param>
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
    /// <summary>
    /// 現在のボタンマスクを取得
    /// </summary>
    /// <returns>ボタンマスクのビット集合</returns>
    uint32_t GetButtonMask() const { return buttonMask_; }

    /// <summary>
    /// 前回フレームのボタンマスクを取得
    /// </summary>
    /// <returns>前回フレームのボタンマスクのビット集合</returns>
    uint32_t GetPrevButtonMask() const { return prevButtonMask_; }

    /// <summary>
    /// ゲームパッドが有効か
    /// </summary>
    /// <returns>有効であればtrue</returns>
    bool IsActive() const { return isActive_; }

    /// <summary>
    /// ボタン押下状態
    /// </summary>
    /// <param name="_button">ボタンビット（PadButtonの値をuint32_tにキャストしたものなど）</param>
    /// <returns>押されていればtrue</returns>
    bool IsPress(uint32_t _button) const { return (buttonMask_ & _button); }

    /// <summary>
    /// ボタン押下状態
    /// </summary>
    /// <param name="_button">ボタン種類</param>
    /// <returns>押されていればtrue</returns>
    bool IsPress(PadButton _button) const { return (buttonMask_ & static_cast<uint32_t>(_button)); }

    /// <summary>
    /// ボタン押下瞬間
    /// </summary>
    /// <param name="_button">ボタンビット</param>
    /// <returns>押された瞬間ならtrue</returns>
    bool IsTrigger(uint32_t _button) const { return (buttonMask_ & _button) && !(prevButtonMask_ & _button); }

    /// <summary>
    /// ボタン押下瞬間
    /// </summary>
    /// <param name="_button">ボタン種類</param>
    /// <returns>押された瞬間ならtrue</returns>
    bool IsTrigger(PadButton _button) const { return (buttonMask_ & static_cast<uint32_t>(_button)) && !(prevButtonMask_ & static_cast<uint32_t>(_button)); }

    /// <summary>
    /// ボタン解放瞬間
    /// </summary>
    /// <param name="_button">ボタンビット</param>
    /// <returns>離された瞬間ならtrue</returns>
    bool IsRelease(uint32_t _button) const { return !(buttonMask_ & _button) && (prevButtonMask_ & _button); }

    /// <summary>
    /// ボタン解放瞬間
    /// </summary>
    /// <param name="_button">ボタン種類</param>
    /// <returns>離された瞬間ならtrue</returns>
    bool IsRelease(PadButton _button) const { return !(buttonMask_ & static_cast<uint32_t>(_button)) && (prevButtonMask_ & static_cast<uint32_t>(_button)); }

    /// <summary>
    /// 左スティックの現在座標（-1.0〜1.0）
    /// </summary>
    /// <returns>左スティックの入力値</returns>
    const Vec2f& GetLStick() const { return lStick_; }

    /// <summary>
    /// 右スティックの現在座標を取得する.
    /// </summary>
    /// <returns>スティック入力ベクトル (-1.0 ～ 1.0)</returns>
    const Vec2f& GetRStick() const { return rStick_; }

    /// <summary>
    /// 左トリガー値（0.0〜1.0）
    /// </summary>
    /// <returns>左トリガーの入力値</returns>
    float GetLTrigger() const { return lTrigger_; }

    /// <summary>
    /// 右トリガー値（0.0〜1.0）
    /// </summary>
    /// <returns>右トリガーの入力値</returns>
    float GetRTrigger() const { return rTrigger_; }
};

} // namespace OriGine
