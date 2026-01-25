#pragma once

/// input API
#include "include/IncludeInputAPI.h"

/// stl
#include <deque>

/// util
#include "util/globalVariables/SerializedField.h"

/// math
#include "math/Vector2.h"
#include <intsafe.h>
#include <stdint.h>

namespace OriGine {

constexpr float kStickMax   = static_cast<float>(SHORT_MAX);
constexpr float kStickMin   = static_cast<float>(-SHORT_MAX);
constexpr float kTriggerMax = static_cast<float>(BYTE_MAX);

enum class GamepadButton : uint32_t {
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

    // 仮想左スティック方向キー（ビット18-21）
    L_STICK_UP    = 1u << 18,
    L_STICK_DOWN  = 1u << 19,
    L_STICK_LEFT  = 1u << 20,
    L_STICK_RIGHT = 1u << 21,

    // 仮想右スティック方向キー（ビット22-25）
    R_STICK_UP    = 1u << 22,
    R_STICK_DOWN  = 1u << 23,
    R_STICK_LEFT  = 1u << 24,
    R_STICK_RIGHT = 1u << 25,
};

static std::map<GamepadButton, std::string> padButtonNameMap = {
    {GamepadButton::UP, "UP"},
    {GamepadButton::DOWN, "DOWN"},
    {GamepadButton::LEFT, "LEFT"},
    {GamepadButton::RIGHT, "RIGHT"},
    {GamepadButton::START, "START"},
    {GamepadButton::BACK, "BACK"},
    {GamepadButton::L_THUMB, "L_THUMB"},
    {GamepadButton::R_THUMB, "R_THUMB"},
    {GamepadButton::L_SHOULDER, "L_SHOULDER"},
    {GamepadButton::R_SHOULDER, "R_SHOULDER"},
    {GamepadButton::A, "A"},
    {GamepadButton::B, "B"},
    {GamepadButton::X, "X"},
    {GamepadButton::Y, "Y"},
    {GamepadButton::L_TRIGGER, "L_TRIGGER"},
    {GamepadButton::R_TRIGGER, "R_TRIGGER"},
    {GamepadButton::L_STICK_UP, "L_STICK_UP"},
    {GamepadButton::L_STICK_DOWN, "L_STICK_DOWN"},
    {GamepadButton::L_STICK_LEFT, "L_STICK_LEFT"},
    {GamepadButton::L_STICK_RIGHT, "L_STICK_RIGHT"},
    {GamepadButton::R_STICK_UP, "R_STICK_UP"},
    {GamepadButton::R_STICK_DOWN, "R_STICK_DOWN"},
    {GamepadButton::R_STICK_LEFT, "R_STICK_LEFT"},
    {GamepadButton::R_STICK_RIGHT, "R_STICK_RIGHT"},
};

constexpr uint32_t PAD_BUTTON_COUNT = 26;

/// <summary>
/// ゲームパッドの状態構造体
/// </summary>
struct GamepadState {
    uint32_t buttonMask;
    Vec2f lStick;
    Vec2f rStick;
    float lTrigger;
    float rTrigger;
};

/// <summary>
/// XInput対応ゲームパッド入力を管理するクラス
/// </summary>
class GamepadInput {
    friend class ReplayPlayer;

    // 入力履歴を保持する数
    static constexpr uint32_t kInputHistoryCount = 60;

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
    /// 履歴をクリア
    /// </summary>
    void ClearHistory();

private:
    /// <summary>
    /// スティックの値を正規化して更新
    /// </summary>
    /// <param name="_state">XInput状態</param>
    void UpdateStickValues(XINPUT_STATE _state, GamepadState& _currentState);

    /// <summary>
    /// 仮想スティックボタンの状態をスティックの値から更新
    /// </summary>
    void UpdateVirtualStickButtons(GamepadState& _currentState);

    /// <summary>
    /// 仮想トリガーボタンの状態をトリガーの値から更新
    /// </summary>
    /// <param name="_currentState"></param>
    void UpdateVirtualTriggerButtons(GamepadState& _currentState);

private:
    SerializedField<float> deadZone_{"Input", "GamePad", "DeadZone"};
    SerializedField<float> triggerDeadZone_{"Input", "GamePad", "TriggerDeadZone"};

    bool isActive_ = false;

    // 仮想ボタンマスク
    std::deque<GamepadState> inputHistory_{};

public:
    /// <summary>
    /// ゲームパッドが接続されているか
    /// </summary>
    /// <returns></returns>
    bool IsActive() const {
        return isActive_;
    }

    /// <summary>
    /// 履歴から指定したインデックスの生状態を取得
    /// </summary>
    /// <param name="_historyIndex"></param>
    /// <returns></returns>
    const GamepadState* GetState(size_t _historyIndex) const {
        if (_historyIndex >= inputHistory_.size()) {
            return nullptr;
        }
        return &inputHistory_[_historyIndex];
    }

    const GamepadState& GetCurrentState() const;

    /// <summary>
    /// 履歴のサイズを取得
    /// </summary>
    /// <returns></returns>
    size_t GetHistorySize() const {
        return static_cast<int>(inputHistory_.size());
    }

    /// <summary>
    /// 何かしらのボタンが押されているか
    /// </summary>
    /// <returns></returns>
    bool IsPressAny() const {
        const auto& state = GetCurrentState();
        return state.buttonMask != 0;
    }

    /// <summary>
    /// ボタンが押されているか (Hold)
    /// </summary>
    /// <param name="_buttonMask"></param>
    /// <returns></returns>
    bool IsPress(uint32_t _buttonMask) const {
        const auto& state = GetCurrentState();
        return (state.buttonMask & _buttonMask) != 0;
    }
    /// <summary>
    /// ボタンが押されているか (Hold)
    /// </summary>
    /// <param name="_button"></param>
    /// <returns></returns>
    bool IsPress(GamepadButton _button) const;

    /// <summary>
    /// 左スティックの取得
    /// </summary>
    /// <returns></returns>
    Vec2f GetLeftStick() const;

    /// <summary>
    /// 右スティックの取得
    /// </summary>
    /// <returns></returns>
    Vec2f GetRightStick() const;

    /// <summary>
    /// 左トリガー (L2)
    /// </summary>
    /// <returns></returns>
    float GetLeftTrigger() const;

    /// <summary>
    /// 右トリガー (R2)
    /// </summary>
    /// <returns></returns>
    float GetRightTrigger() const;

    // ==========================================
    // 2. エッジ検出 (履歴比較)
    // ==========================================

    /// <summary>
    /// ボタンが押された瞬間か (Just Pressed / Trigger)
    /// </summary>
    /// <param name="_buttonMask"></param>
    /// <returns></returns>
    bool IsTrigger(uint32_t _buttonMask) const;

    /// <summary>
    ///  押された瞬間か (Just Pressed / Trigger)
    /// </summary>
    /// <param name="_button"></param>
    /// <returns></returns>
    bool IsTrigger(GamepadButton _button) const;

    /// <summary>
    /// ボタンが離された瞬間か (Just Released)
    /// </summary>
    /// <param name="_buttonMask"></param>
    /// <returns></returns>
    bool IsRelease(uint32_t _buttonMask) const;

    /// <summary>
    /// 離された瞬間か (Just Released)
    /// </summary>
    /// <param name="_button"></param>
    /// <returns></returns>
    bool IsRelease(GamepadButton _button) const;

    // ==========================================
    // 3. 履歴を利用した拡張判定
    // ==========================================

    /// <summary>
    /// 直近 N フレーム以内に押されたか？ (先行入力用)
    /// </summary>
    /// <param name="_button"></param>
    /// <param name="_framesToCheck"></param>
    /// <returns></returns>
    bool WasPressedRecently(GamepadButton _button, size_t _framesToCheck = 5) const;

    /// <summary>
    /// 直近 N フレーム以内に入力を開始したか？ (先行入力用)
    /// </summary>
    /// <param name="_button"></param>
    /// <param name="_framesToCheck"></param>
    /// <returns></returns>
    bool WasTriggeredRecently(GamepadButton _button, size_t _framesToCheck = 5) const;

    /// <summary>
    /// 直近 N フレーム以内に離されたか？ (キャンセル入力用)
    /// </summary>
    /// <param name="_button"></param>
    /// <param name="_framesToCheck"></param>
    /// <returns></returns>
    bool WasReleasedRecently(GamepadButton _button, size_t _framesToCheck = 5) const;

    /// <summary>
    /// 指定したフレーム数、ボタンが押し続けられているか？ (溜め判定用)
    /// </summary>
    /// <param name="_button"></param>
    /// <param name="_frames"></param>
    /// <returns></returns>
    bool IsPressedDuration(GamepadButton _button, size_t _frames) const;
};

} // namespace OriGine
