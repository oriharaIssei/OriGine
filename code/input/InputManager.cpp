#include "InputManager.h"

/// engine
#include "Engine.h"
#include "winApp/WinApp.h"

namespace OriGine {

//////////////////////////////////////////////////////////
// InputManager
//////////////////////////////////////////////////////////
/// <summary> シングルトンインスタンスの取得. </summary>
InputManager* InputManager::GetInstance() {
    static InputManager instance;
    return &instance;
}

/// <summary>
/// 各入力デバイスの初期化.
/// DirectInput8 インターフェースを生成し、キーボード・マウスをそれに紐付けて初期化する.
/// </summary>
void InputManager::Initialize(HWND _hwnd) {
    hwnd_ = _hwnd;

    // キーボード・マウスは DirectInput 経由でデバイスを取得するため、
    // 個別デバイスの Initialize より先に DirectInput8 インターフェースを生成しておく必要がある
    DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8,
        (void**)&directInput_, nullptr);

    keyboard_.Initialize(directInput_.Get(), hwnd_);
    mouse_.Initialize(directInput_.Get(), hwnd_);
    // ゲームパッドは DirectInput ではなく XInput ベースで実装されているため、
    // directInput_ に依存せず単独で初期化できる（呼び出し順を気にする必要がない）
    gamepad_.Initialize();
}

/// <summary> 全入力デバイスの状態を更新する. 毎フレーム呼び出す必要がある. </summary>
void InputManager::Update() {
    keyboard_.Update();
    mouse_.Update();
    gamepad_.Update();
}

/// <summary> 終了処理を行い、各入力デバイスが保持するリソース・履歴を解放する. </summary>
void InputManager::Finalize() {
    keyboard_.Finalize();
    mouse_.Finalize();
    gamepad_.Finalize();
}

} // namespace OriGine
