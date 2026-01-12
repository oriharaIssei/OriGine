#pragma once

/// input API
#include "include/IncludeInputAPI.h"

/// windows
#include <Windows.h>

/// stl
#include <array>
#include <map>
#include <string>

/// math
#include "math/Vector2.h"
#include <cstdint>

namespace OriGine {

/// <summary>
/// マウスボタンの列挙型.
/// </summary>
enum class MouseButton : uint32_t {
    /// <summary>左ボタン</summary>
    LEFT = 0,
    /// <summary>右ボタン</summary>
    RIGHT = 1,
    /// <summary>中央（ホイール）ボタン</summary>
    MIDDLE = 2,
};

/// <summary>
/// マウスボタン名変換マップ
/// </summary>
static ::std::map<MouseButton, ::std::string> mouseButtonName = {
    {MouseButton::LEFT, "LEFT"},
    {MouseButton::RIGHT, "RIGHT"},
    {MouseButton::MIDDLE, "MIDDLE"},
};

/// <summary>DirectInput が扱う最大マウスボタン数</summary>
constexpr uint32_t MOUSE_BUTTON_COUNT = 8;

/// <summary>
/// DirectInput によるマウス入力を管理するクラス.
/// </summary>
class MouseInput {
    friend class ReplayPlayer;

public:
    MouseInput()  = default;
    ~MouseInput() = default;

    MouseInput(const MouseInput&)            = delete;
    MouseInput& operator=(const MouseInput&) = delete;

    /// <summary>
    /// デバイスの初期化を行う.
    /// </summary>
    /// <param name="directInput">DirectInput8 インターフェース</param>
    /// <param name="hwnd">ウィンドウハンドル</param>
    void Initialize(IDirectInput8* directInput, HWND hwnd);

    /// <summary>
    /// 毎フレームのデバイス入力状態をポーリングして更新する.
    /// </summary>
    void Update();

    /// <summary>
    /// デバイスの終了処理を行う.
    /// </summary>
    void Finalize();

    /// <summary>
    /// 現在のボタン状態をビットマスクに変換して取得する.
    /// </summary>
    /// <returns>ボタン状態のビットマスク</returns>
    uint32_t ButtonStateToBitmask() const;

    /// <summary>
    /// ボタン状態バッファをクリアする.
    /// </summary>
    void ClearButtonStates() {
        currentButtonStates_.fill(0);
        prevButtonStates_.fill(0);
    }

    /// <summary>
    /// ホイールの変化量をリセットする.
    /// </summary>
    void ResetWheelDelta() {
        currentWheelDelta_ = 0;
        prevWheelDelta_    = 0;
    }

    /// <summary>
    /// マウスの各座標情報をリセットする.
    /// </summary>
    void ResetPosition() {
        pos_        = Vec2f(0.0f, 0.0f);
        prevPos_    = Vec2f(0.0f, 0.0f);
        virtualPos_ = Vec2f(0.0f, 0.0f);
        velocity_   = Vec2f(0.0f, 0.0f);
    }

private:
    /// <summary>マウスデバイス</summary>
    Microsoft::WRL::ComPtr<IDirectInputDevice8> mouse_ = nullptr;
    /// <summary>ウィンドウハンドル</summary>
    HWND hwnd_ = nullptr;

    /// <summary>現在のボタン状態</summary>
    ::std::array<BYTE, MOUSE_BUTTON_COUNT> currentButtonStates_{};
    /// <summary>前回フレームのボタン状態</summary>
    ::std::array<BYTE, MOUSE_BUTTON_COUNT> prevButtonStates_{};

    /// <summary>現在のホイール回転量</summary>
    int32_t currentWheelDelta_ = 0;
    /// <summary>前回フレームのホイール回転量</summary>
    int32_t prevWheelDelta_ = 0;

    /// <summary>ユーザー制御用の仮想座標</summary>
    Vec2f virtualPos_{};
    /// <summary>クライアント領域内での現在の位置</summary>
    Vec2f pos_{};
    /// <summary>前回フレームの位置</summary>
    Vec2f prevPos_{};
    /// <summary>フレーム間の移動量</summary>
    Vec2f velocity_{};

    /// <summary>カーソルを表示中か</summary>
    bool isCursorVisible_ = true;

public:
    /// <summary>
    /// 現在の全ボタンの状態を取得する.
    /// </summary>
    /// <returns>現在のボタン状態配列</returns>
    const ::std::array<BYTE, MOUSE_BUTTON_COUNT>& GetCurrentButtonState() const { return currentButtonStates_; }

    /// <summary>
    /// 前回フレームの全ボタンの状態を取得する.
    /// </summary>
    /// <returns>前回フレームのボタン状態配列</returns>
    const ::std::array<BYTE, MOUSE_BUTTON_COUNT>& GetPrevButtonState() const { return prevButtonStates_; }

    /// <summary>
    /// 指定されたボタンが現在押されているか判定する.
    /// </summary>
    /// <param name="button">ボタン番号</param>
    /// <returns>押されていれば true</returns>
    bool IsPress(uint32_t button) const { return currentButtonStates_[button]; }

    /// <summary>
    /// 指定されたボタンが現在押されているか判定する.
    /// </summary>
    /// <param name="button">ボタン種類</param>
    /// <returns>押されていれば true</returns>
    bool IsPress(MouseButton button) const { return currentButtonStates_[static_cast<uint32_t>(button)]; }

    /// <summary>
    /// 指定されたボタンがこのフレームで押されたか判定する.
    /// </summary>
    /// <param name="button">ボタン番号</param>
    /// <returns>押された瞬間なら true</returns>
    bool IsTrigger(uint32_t button) const { return currentButtonStates_[button] && !prevButtonStates_[button]; }

    /// <summary>
    /// 指定されたボタンがこのフレームで押されたか判定する.
    /// </summary>
    /// <param name="button">ボタン種類</param>
    /// <returns>押された瞬間なら true</returns>
    bool IsTrigger(MouseButton button) const {
        return currentButtonStates_[static_cast<uint32_t>(button)] && !prevButtonStates_[static_cast<uint32_t>(button)];
    }

    /// <summary>
    /// 指定されたボタンがこのフレームで離されたか判定する.
    /// </summary>
    /// <param name="button">ボタン番号</param>
    /// <returns>離された瞬間なら true</returns>
    bool IsRelease(uint32_t button) const { return !currentButtonStates_[button] && prevButtonStates_[button]; }

    /// <summary>
    /// 指定されたボタンがこのフレームで離されたか判定する.
    /// </summary>
    /// <param name="button">ボタン種類</param>
    /// <returns>離された瞬間なら true</returns>
    bool IsRelease(MouseButton button) const {
        return !currentButtonStates_[static_cast<uint32_t>(button)] && prevButtonStates_[static_cast<uint32_t>(button)];
    }

    /// <summary>
    /// 現在のホイール回転量を取得する.
    /// </summary>
    /// <returns>ホイールの変化量 (通常 120 の倍数)</returns>
    int32_t GetWheelDelta() const { return static_cast<int32_t>(currentWheelDelta_); }

    /// <summary>
    /// 前回フレームのホイール回転量を取得する.
    /// </summary>
    /// <returns>前フレームのホイールの変化量</returns>
    int32_t GetPrevWheelDelta() const { return prevWheelDelta_; }

    /// <summary>
    /// 今フレームでホイールが回転したか判定する.
    /// </summary>
    /// <returns>回転していれば true</returns>
    bool IsWheel() const { return currentWheelDelta_ != 0; }

    /// <summary>
    /// ホイールが奥（画面上方向）に回転したか判定する.
    /// </summary>
    /// <returns>奥に回転していれば true</returns>
    bool IsWheelUp() const { return currentWheelDelta_ > 0; }

    /// <summary>
    /// ホイールが手前（画面下方向）に回転したか判定する.
    /// </summary>
    /// <returns>手前に回転していれば true</returns>
    bool IsWheelDown() const { return currentWheelDelta_ < 0; }

    /// <summary>
    /// 前回フレームでホイールが回転していたか判定する.
    /// </summary>
    /// <returns>回転していれば true</returns>
    bool IsPrevWheel() const { return prevWheelDelta_ != 0; }

    /// <summary>
    /// 停止状態からホイールが回転し始めた瞬間か判定する.
    /// </summary>
    /// <returns>回転し始めた瞬間なら true</returns>
    bool IsTriggerWheel() const {
        return (currentWheelDelta_ != 0) && (prevWheelDelta_ == 0);
    }

    /// <summary>
    /// 回転していたホイールが停止した瞬間か判定する.
    /// </summary>
    /// <returns>停止した瞬間なら true</returns>
    bool IsReleaseWheel() const {
        return (currentWheelDelta_ == 0) && (prevWheelDelta_ != 0);
    }

    /// <summary>
    /// 現在のマウス座標を取得する.
    /// </summary>
    /// <returns>現在のマウス座標</returns>
    const Vec2f& GetPosition() const { return pos_; }

    /// <summary>
    /// ユーザー定義の仮想座標を取得する.
    /// </summary>
    /// <returns>現在の仮想マウス座標</returns>
    const Vec2f& GetVirtualPosition() const { return virtualPos_; }

    /// <summary>
    /// ユーザー定義の仮想座標を設定する.
    /// </summary>
    /// <param name="pos">設定する仮想座標</param>
    void SetVirtualPosition(const Vec2f& pos) { virtualPos_ = pos; }

    /// <summary>
    /// 前回フレームのマウス座標を取得する.
    /// </summary>
    /// <returns>前フレームのマウス座標</returns>
    const OriGine::Vec2f& GetPrevPosition() const { return prevPos_; }

    /// <summary>
    /// フレーム間の移動量を取得する.
    /// </summary>
    /// <returns>マウスの移動量ベクトル</returns>
    const OriGine::Vec2f& GetVelocity() const { return velocity_; }

    /// <summary>
    /// システムのカーソル位置を移動させる.
    /// </summary>
    /// <param name="pos">移動先の座標</param>
    void SetPosition(const Vec2f& pos);

    /// <summary>
    /// システムのマウスカーソルの表示・非表示を切り替える.
    /// </summary>
    /// <param name="show">表示するなら true, 隠すなら false</param>
    void SetShowCursor(bool show);
};

} // namespace OriGine
