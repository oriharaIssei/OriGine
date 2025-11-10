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

/// <summary>
/// マウス入力を管理するクラス
/// </summary>
class MouseInput {
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

private:
    Microsoft::WRL::ComPtr<IDirectInputDevice8> mouse_;
    DIMOUSESTATE2 current_{};
    DIMOUSESTATE2 prev_{};

    HWND hwnd_ = nullptr;

    Vec2f virtualPos_{}; // userが座標を制御するための変数
    Vec2f pos_{};
    Vec2f prevPos_{};
    Vec2f velocity_{};

    bool isCursorVisible_ = true;

public:
    /// <summary>
    /// マウスボタンが押されているか
    /// </summary>
    bool isPress(uint32_t button) const { return current_.rgbButtons[button]; }
    bool isPress(MouseButton button) const { return current_.rgbButtons[static_cast<uint32_t>(button)]; }

    /// <summary>
    /// 押した瞬間か
    /// </summary>
    bool isTrigger(uint32_t button) const { return current_.rgbButtons[button] && !prev_.rgbButtons[button]; }
    bool isTrigger(MouseButton button) const {
        return current_.rgbButtons[static_cast<uint32_t>(button)] && !prev_.rgbButtons[static_cast<uint32_t>(button)];
    }

    /// <summary>
    /// 離した瞬間か
    /// </summary>
    bool isRelease(uint32_t button) const { return !current_.rgbButtons[button] && prev_.rgbButtons[button]; }
    bool isRelease(MouseButton button) const {
        return !current_.rgbButtons[static_cast<uint32_t>(button)] && prev_.rgbButtons[static_cast<uint32_t>(button)];
    }

    /// <summary>
    /// ホイールの変化量を取得
    /// </summary>
    int32_t getWheelDelta() const { return static_cast<int32_t>(current_.lZ); }
    /// <summary>
    /// 前フレームのホイールの変化量を取得
    /// </summary>
    int32_t getPrevWheelDelta() const { return static_cast<int32_t>(prev_.lZ); }

    /// <summary>
    /// ホイールが回転したか
    /// </summary>
    /// <returns></returns>
    bool isWheel() const { return current_.lZ != 0; }
    /// <summary>
    /// ホイールが上に回転したか
    /// </summary>
    /// <returns></returns>
    bool isWheelUp() const { return current_.lZ > 0; }
    /// <summary>
    /// ホイールが下に回転したか
    /// </summary>
    /// <returns></returns>
    bool isWheelDown() const { return current_.lZ < 0; }

    /// <summary>
    /// 前フレームのホイールが回転したか
    /// </summary>
    /// <returns></returns>
    bool isPrevWheel() const { return prev_.lZ != 0; }
    /// <summary>
    /// 前フレームの前フレームのホイールが上に回転したか
    /// </summary>
    /// <returns></returns>
    bool isPrevWheelUp() const { return prev_.lZ > 0; }
    /// <summary>
    /// 前フレームのホイールが下に回転したか
    /// </summary>
    /// <returns></returns>
    bool isPrevWheelDown() const { return prev_.lZ < 0; }

    /// <summary>
    /// ホイールが回転した瞬間か
    /// </summary>
    bool isTriggerWheel() const {
        return (current_.lZ != 0) && (prev_.lZ == 0);
    }
    /// <summary>
    /// ホイールが回転を止めた瞬間か
    /// </summary>
    bool isReleaseWheel() const {
        return (current_.lZ == 0) && (prev_.lZ != 0);
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
