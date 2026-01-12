#pragma once

/// include
#include "include/IncludeInputAPI.h"

/// input
#include "GamepadInput.h"
#include "KeyboardInput.h"
#include "MouseInput.h"

namespace OriGine {

/// <summary>
/// 入力全般を管理するシングルトンクラス.
/// DirectInput と XInput を併用して キーボード・マウス・ゲームパッドの状態を集中管理する.
/// </summary>
class InputManager {
public:
    /// <summary>
    /// シングルトンインスタンスを取得する.
    /// </summary>
    /// <returns>インスタンスのポインタ</returns>
    static InputManager* GetInstance();

    /// <summary>
    /// 直近の初期化を行う. DirectInput 8 の初期化と各デバイスのセットアップを行う.
    /// </summary>
    /// <param name="_hwnd">アプリケーションのウィンドウハンドル</param>
    void Initialize(HWND _hwnd);

    /// <summary>
    /// 全入力デバイスの状態を更新する. 毎フレーム呼び出す必要がある.
    /// </summary>
    void Update();

    /// <summary>
    /// 終了処理を行い、DirectInput オブジェクト等を解放する.
    /// </summary>
    void Finalize();

    /// <summary>
    /// キーボード入力管理オブジェクトを取得する.
    /// </summary>
    /// <returns>キーボード入力管理オブジェクト</returns>
    KeyboardInput* GetKeyboard() { return &keyboard_; }

    /// <summary>
    /// マウス入力管理オブジェクトを取得する.
    /// </summary>
    /// <returns>マウス入力管理オブジェクト</returns>
    MouseInput* GetMouse() { return &mouse_; }

    /// <summary>
    /// ゲームパッド入力管理オブジェクトを取得する.
    /// </summary>
    /// <returns>ゲームパッド入力管理オブジェクト</returns>
    GamepadInput* GetGamePad() { return &gamepad_; }

private:
    /// <summary>DirectInput8 インターフェース</summary>
    Microsoft::WRL::ComPtr<IDirectInput8> directInput_;

    /// <summary>ウィンドウハンドル</summary>
    HWND hwnd_ = nullptr;

    /// <summary>キーボード入力管理</summary>
    KeyboardInput keyboard_;
    /// <summary>マウス入力管理</summary>
    MouseInput mouse_;
    /// <summary>ゲームパッド入力管理</summary>
    GamepadInput gamepad_;
};

} // namespace OriGine
