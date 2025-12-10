#pragma once

/// include
#include "include/IncludeInputAPI.h"

/// input
#include "GamepadInput.h"
#include "KeyboardInput.h"
#include "MouseInput.h"

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
    GamepadInput* GetGamePad() { return &gamepad_; }

private:
    Microsoft::WRL::ComPtr<IDirectInput8> directInput_;

    HWND hwnd_ = nullptr;

    KeyboardInput keyboard_;
    MouseInput mouse_;
    GamepadInput gamepad_;
};

} // namespace OriGine
