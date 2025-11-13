#pragma once
/// input API
#include "include/IncludeInputAPI.h"

/// math
#include "math/Vector2.h"
#include <cstdint>

enum class MouseButton : uint32_t {
    LEFT   = 0,
    RIGHT  = 1,
    MIDDLE = 2,
};
static std::map<MouseButton, std::string> mouseButtonName = {
    {MouseButton::LEFT, "LEFT"},
    {MouseButton::RIGHT, "RIGHT"},
    {MouseButton::MIDDLE, "MIDDLE"},
};

constexpr uint32_t MOUSE_BUTTON_COUNT = 8;

/// <summary>
/// マウス入力を管理するクラス
/// </summary>
class MouseInput {
    friend class ReplayPlayer;

public:
    MouseInput()  = default;
    ~MouseInput() = default;

    MouseInput(const MouseInput&)            = delete;
    MouseInput& operator=(const MouseInput&) = delete;

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize(IDirectInput8* directInput, HWND hwnd);

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 解放
    /// </summary>
    void Finalize();

    /// <summary>
    /// ボタン状態をビットマスクに変換
    /// </summary>
    /// <returns></returns>
    uint32_t ButtonStateToBitmask() const;

    /// <summary>
    /// ボタン状態をクリア
    /// </summary>
    void clearButtonStates() {
        currentButtonStates_.fill(0);
        prevButtonStates_.fill(0);
    }

    /// <summary>
    /// ホイールの変化量をリセット
    /// </summary>
    void resetWheelDelta() {
        currentWheelDelta_ = 0;
        prevWheelDelta_    = 0;
    }

    /// <summary>
    /// マウス座標をリセット
    /// </summary>
    void resetPosition() {
        pos_        = Vec2f(0.0f, 0.0f);
        prevPos_    = Vec2f(0.0f, 0.0f);
        virtualPos_ = Vec2f(0.0f, 0.0f);
        velocity_   = Vec2f(0.0f, 0.0f);
    }

private:
    Microsoft::WRL::ComPtr<IDirectInputDevice8> mouse_;
    HWND hwnd_ = nullptr;

    std::array<BYTE, MOUSE_BUTTON_COUNT> currentButtonStates_{};
    std::array<BYTE, MOUSE_BUTTON_COUNT> prevButtonStates_{};

    int32_t currentWheelDelta_ = 0;
    int32_t prevWheelDelta_    = 0;

    Vec2f virtualPos_{}; // userが座標を制御するための変数
    Vec2f pos_{};
    Vec2f prevPos_{};
    Vec2f velocity_{};

    bool isCursorVisible_ = true;

public:
    const std::array<BYTE, MOUSE_BUTTON_COUNT>& getCurrentButtonState() const { return currentButtonStates_; }
    const std::array<BYTE, MOUSE_BUTTON_COUNT>& getPrevButtonState() const { return prevButtonStates_; }

    /// <summary>
    /// マウスボタンが押されているか
    /// </summary>
    bool isPress(uint32_t button) const { return currentButtonStates_[button]; }
    bool isPress(MouseButton button) const { return currentButtonStates_[static_cast<uint32_t>(button)]; }

    /// <summary>
    /// 押した瞬間か
    /// </summary>
    bool isTrigger(uint32_t button) const { return currentButtonStates_[button] && !prevButtonStates_[button]; }
    bool isTrigger(MouseButton button) const {
        return currentButtonStates_[static_cast<uint32_t>(button)] && !prevButtonStates_[static_cast<uint32_t>(button)];
    }

    /// <summary>
    /// 離した瞬間か
    /// </summary>
    bool isRelease(uint32_t button) const { return !currentButtonStates_[button] && prevButtonStates_[button]; }
    bool isRelease(MouseButton button) const {
        return !currentButtonStates_[static_cast<uint32_t>(button)] && prevButtonStates_[static_cast<uint32_t>(button)];
    }

    /// <summary>
    /// ホイールの変化量を取得
    /// </summary>
    int32_t getWheelDelta() const { return static_cast<int32_t>(currentWheelDelta_); }
    /// <summary>
    /// 前フレームのホイールの変化量を取得
    /// </summary>
    int32_t getPrevWheelDelta() const { return prevWheelDelta_; }

    /// <summary>
    /// ホイールが回転したか
    /// </summary>
    /// <returns></returns>
    bool isWheel() const { return currentWheelDelta_ != 0; }
    /// <summary>
    /// ホイールが上に回転したか
    /// </summary>
    /// <returns></returns>
    bool isWheelUp() const { return currentWheelDelta_ > 0; }
    /// <summary>
    /// ホイールが下に回転したか
    /// </summary>
    /// <returns></returns>
    bool isWheelDown() const { return currentWheelDelta_ < 0; }

    /// <summary>
    /// 前フレームのホイールが回転したか
    /// </summary>
    /// <returns></returns>
    bool isPrevWheel() const { return prevWheelDelta_ != 0; }
    /// <summary>
    /// 前フレームの前フレームのホイールが上に回転したか
    /// </summary>
    /// <returns></returns>
    bool isPrevWheelUp() const { return prevWheelDelta_ > 0; }
    /// <summary>
    /// 前フレームのホイールが下に回転したか
    /// </summary>
    /// <returns></returns>
    bool isPrevWheelDown() const { return prevWheelDelta_ < 0; }

    /// <summary>
    /// ホイールが回転した瞬間か
    /// </summary>
    bool isTriggerWheel() const {
        return (currentWheelDelta_ != 0) && (prevWheelDelta_ == 0);
    }
    /// <summary>
    /// ホイールが回転を止めた瞬間か
    /// </summary>
    bool isReleaseWheel() const {
        return (currentWheelDelta_ == 0) && (prevWheelDelta_ != 0);
    }

    /// <summary>
    /// 現在の座標を取得
    /// </summary>
    const Vec2f& getPosition() const { return pos_; }

    /// <summary>
    /// 仮想座標を取得
    /// </summary>
    const Vec2f& getVirtualPosition() const { return virtualPos_; }
    /// <summary>
    /// 仮想座標を設定
    /// </summary>
    void setVirtualPosition(const Vec2f& pos) { virtualPos_ = pos; }

    /// <summary>
    /// 前フレームの座標を取得
    /// </summary>
    const Vec2f& getPrevPosition() const { return prevPos_; }

    /// <summary>
    /// フレーム間の移動量を取得
    /// </summary>
    const Vec2f& getVelocity() const { return velocity_; }

    /// <summary>
    /// カーソル位置を設定
    /// </summary>
    void setPosition(const Vec2f& pos);

    /// <summary>
    /// マウスカーソルを表示／非表示にする
    /// </summary>
    void ShowCursor(bool show);
};
