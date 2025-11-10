#pragma once

/// input
#include "GamePadInput.h"
#include "KeyboardInput.h"
#include "MouseInput.h"

// math
#include <math/Vector2.h>

/// <summary>
/// 入力管理クラス
/// </summary>
class InputManager {
public:
    static InputManager* getInstance();

    void Initialize(HWND _hwnd);
    void Update();
    void Finalize();

    KeyboardInput* getKeyboard() { return &keyboard_; }
    MouseInput* getMouse() { return &mouse_; }
    GamePadInput* getGamePad() { return &gamepad_; }

private:
    Microsoft::WRL::ComPtr<IDirectInput8> directInput_;

    HWND hwnd_ = nullptr;

    KeyboardInput keyboard_;
    MouseInput mouse_;
    GamePadInput gamepad_;
};
