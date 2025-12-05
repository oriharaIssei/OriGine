#include "InputManager.h"

/// Engine
#include "Engine.h"

namespace OriGine {

//////////////////////////////////////////////////////////
// InputManager
//////////////////////////////////////////////////////////
InputManager* InputManager::GetInstance() {
    static InputManager instance;
    return &instance;
}

void InputManager::Initialize(HWND _hwnd) {
    hwnd_ = _hwnd;

    DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8,
        (void**)&directInput_, nullptr);

    keyboard_.Initialize(directInput_.Get(), hwnd_);
    mouse_.Initialize(directInput_.Get(), hwnd_);
    gamepad_.Initialize();
}

void InputManager::Update() {
    keyboard_.Update();
    mouse_.Update();
    gamepad_.Update();
}

void InputManager::Finalize() {
    keyboard_.Finalize();
    mouse_.Finalize();
}

} // namespace OriGine
