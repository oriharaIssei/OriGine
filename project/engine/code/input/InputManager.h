#pragma once

/// input
#include "GamePadInput.h"
#include "KeyboardInput.h"
#include "MouseInput.h"

// math
#include <math/Vector2.h>

namespace OriGine {

/// <summary>
/// 入力管理クラス
/// </summary>
class InputManager {
public:
    static InputManager* GetInstance();

    void Initialize(HWND _hwnd);
    void Update();
    void Finalize();

    KeyboardInput* GetKeyboard() { return &keyboard_; }
    MouseInput* GetMouse() { return &mouse_; }
    GamePadInput* GetGamePad() { return &gamepad_; }

private:
    Microsoft::WRL::ComPtr<IDirectInput8> directInput_;

    HWND hwnd_ = nullptr;

    KeyboardInput keyboard_;
    MouseInput mouse_;
    GamePadInput gamepad_;
};

} // namespace OriGine
