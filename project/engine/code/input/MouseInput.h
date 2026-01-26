#pragma once

/// input API
#include "include/IncludeInputAPI.h"

/// windows
#include <Windows.h>

/// stl
#include <deque>
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
    LEFT = 1u << 1,
    /// <summary>右ボタン</summary>
    RIGHT = 1u << 2,
    /// <summary>中央（ホイール）ボタン</summary>
    MIDDLE = 1u << 3,

    BTN_3 = 1u << 4,
    BTN_4 = 1u << 5,
    BTN_5 = 1u << 6,
    BTN_6 = 1u << 7,
    BTN_7 = 1u << 8,
};

/// <summary>
/// マウスボタン名変換マップ
/// </summary>
static ::std::map<MouseButton, ::std::string> mouseButtonName = {
    {MouseButton::LEFT, "LEFT"},
    {MouseButton::RIGHT, "RIGHT"},
    {MouseButton::MIDDLE, "MIDDLE"},
    {MouseButton::BTN_3, "BTN_3"},
    {MouseButton::BTN_4, "BTN_4"},
    {MouseButton::BTN_5, "BTN_5"},
    {MouseButton::BTN_6, "BTN_6"},
    {MouseButton::BTN_7, "BTN_7"},
};

/// <summary>
/// 1 フレーム分のマウス入力情報.
/// </summary>
struct MouseState {
    Vec2f mousePos; // マウスのスクリーン座標
    Vec2f velocity; // マウスの移動量 マウスの座標が後から書き換えられる可能性があるので、フレームの開始時の位置との差分を保持しておく
    int32_t wheelDelta; // ホイールの回転量
    uint32_t buttonData; // マウスボタンのビットマスク状態
};

/// <summary>DirectInput が扱う最大マウスボタン数</summary>
static constexpr uint32_t MOUSE_BUTTON_COUNT = 8;

class MouseInput {
    friend class ReplayPlayer;

    static constexpr uint32_t kInputHistoryCount = 60;

public:
    MouseInput()  = default;
    ~MouseInput() = default;

    MouseInput(const MouseInput&)            = delete;
    MouseInput& operator=(const MouseInput&) = delete;

    void Initialize(IDirectInput8* _directInput, HWND _hwnd);
    void Update();
    void Finalize();

    /// <summary>
    /// 履歴をクリアする
    /// </summary>
    void ClearHistory();

private:
    // ヘルパー: 履歴への安全なアクセス
    const MouseState* GetState(size_t _historyIndex) const;
    MouseState* GetStateRef(size_t _historyIndex);

    // 現在のボタン状態をビットマスクに変換 (Update内で使用)
    uint32_t CreateButtonBitmask(const DIMOUSESTATE2& _diState) const;

private:
    Microsoft::WRL::ComPtr<IDirectInputDevice8> mouse_ = nullptr;
    HWND hwnd_                                         = nullptr;

    // 履歴管理
    std::deque<MouseState> inputHistory_{};

    // ユーザー制御用の仮想座標 (履歴とは別に保持が必要)
    Vec2f virtualPos_{};

    // カーソル表示フラグ
    bool isCursorVisible_ = true;

public:
    /// <summary>
    /// マウスデバイスが存在するか
    /// このインスタンスが有効かどうか
    /// </summary>
    /// <returns></returns>
    bool HasMouse() const { return mouse_ != nullptr; }

    // ==========================================
    // アクセッサ (履歴を利用)
    // ==========================================

    /// <summary>
    /// 現在のマウス状態を取得
    /// </summary>
    /// <returns></returns>
    const MouseState& GetCurrentState() const;

    /// <summary>
    /// 指定したボタンが押されているか (Press/Hold)
    /// </summary>
    /// <param name="_button"></param>
    /// <returns></returns>
    bool IsPress(MouseButton _button) const;

    /// <summary>
    /// 指定したボタンが押された瞬間か (Press/Down)
    /// </summary>
    /// <param name="_button"></param>
    /// <returns></returns>
    bool IsTrigger(MouseButton _button) const;

    /// <summary>
    /// 指定したボタンが離された瞬間か (Release/Up)
    /// </summary>
    /// <param name="_button"></param>
    /// <returns></returns>
    bool IsRelease(MouseButton _button) const;

    /// <summary>
    /// マウスの位置を取得
    /// </summary>
    /// <returns></returns>
    Vec2f GetPosition() const;
    /// <summary>
    /// マウスの位置を設定
    /// </summary>
    /// <param name="_pos"></param>
    void SetPosition(const Vec2f& _pos);

    /// <summary>
    /// 現在の速度を取得
    /// </summary>
    /// <returns></returns>
    Vec2f GetVelocity() const;

    /// <summary>
    /// 指定した2フレーム間の速度を計算
    /// </summary>
    /// <param name="_recentIdx">新しいFrame</param>
    /// <param name="_oldIdx">古いFrame</param>
    /// <returns></returns>
    Vec2f CalculateVelocityBetween(size_t _recentIdx, size_t _oldIdx) const;

    /// <summary>
    /// 指定したフレームから過去に遡って履歴サイズ分の平均速度を計算
    /// </summary>
    /// <param name="_from">指定するフレーム(0 == 最新)</param>
    /// <param name="_historySize">遡る履歴の範囲</param>
    /// <returns></returns>
    Vec2f GetAverageVelocity(size_t _from, size_t _historyRange) const;

    // ホイールの回転量を取得
    int32_t GetWheelDelta() const;

    /// <summary>
    /// 指定したフレームから過去に遡って履歴サイズ分のホイール回転量の平均を計算
    /// </summary>
    /// <param name="_from"></param>
    /// <param name="_historyRange"></param>
    /// <returns></returns>
    int32_t WheelAverageDelta(size_t _from, size_t _historyRange) const;

    /// <summary>
    /// ホイールが回転したか
    /// </summary>
    /// <returns></returns>
    bool IsWheel() const { return GetWheelDelta() != 0; }
    /// <summary>
    /// ホイールが上回転したか
    /// </summary>
    /// <returns></returns>
    bool IsWheelUp() const { return GetWheelDelta() > 0; }
    /// <summary>
    /// ホイールが下回転したか
    /// </summary>
    /// <returns></returns>
    bool IsWheelDown() const { return GetWheelDelta() < 0; }

    /// <summary>
    /// 指定した履歴範囲内でホイールが回転したか
    /// </summary>
    /// <param name="_historyRange"></param>
    /// <returns></returns>
    bool WasWheelMovedInHistory(size_t _historyRange) const;
    /// <summary>
    /// 指定した履歴範囲内でホイールが上回転したか
    /// </summary>
    /// <param name="_historyRange"></param>
    /// <returns></returns>
    bool WasWheelUpInHistory(size_t _historyRange) const;
    /// <summary>
    /// 指定した履歴範囲内でホイールが下回転したか
    /// </summary>
    /// <param name="_historyRange"></param>
    /// <returns></returns>
    bool WasWheelDownInHistory(size_t _historyRange) const;

    // 仮想座標の設定 (エディタ操作などで使用)
    void SetVirtualPosition(const Vec2f& _pos) { virtualPos_ = _pos; }
    // 仮想座標の取得
    Vec2f GetVirtualPosition() const { return virtualPos_; }

    // カーソル表示切替
    void ShowCursor(bool _show);
};

} // namespace OriGine
