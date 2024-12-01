#include "input/Input.h"
#include "logger/Logger.h"

#include <cassert>

#include "Engine.h"
#include <Windows.h>

Input *Input::getInstance() {
	static Input instance;
	return &instance;
}

void Input::Init() {
	WinApp *window = Engine::getInstance()->getWinApp();
	HRESULT result = 0;
	result = DirectInput8Create(
		window->getHInstance(),
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void **)&directInput_,
		nullptr
	);
	assert(SUCCEEDED(result));

	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(result));

	result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	result = keyboard_->SetCooperativeLevel(
		window->getHwnd(),
		DISCL_FOREGROUND |
		DISCL_NONEXCLUSIVE |
		DISCL_NOWINKEY
	);
	assert(SUCCEEDED(result));

	result = directInput_->CreateDevice(GUID_SysMouse, &mouse_, NULL);
	assert(SUCCEEDED(result));

	result = mouse_->SetDataFormat(&c_dfDIMouse2);
	assert(SUCCEEDED(result));

	result = mouse_->SetCooperativeLevel(
		window->getHwnd(),
		DISCL_NONEXCLUSIVE |
		DISCL_FOREGROUND
	);
	assert(SUCCEEDED(result));
}

void Input::Finalize() {
	if(keyboard_) {
		keyboard_->Unacquire();
	}
	if(mouse_) {
		mouse_->Unacquire();
	}
}

void Input::Update() {
	keyboard_->Acquire();

	memcpy(preKeys_, keys_, sizeof(keys_));
	keyboard_->GetDeviceState(sizeof(keys_), keys_);

	mouse_->Acquire();
	preMouseState_ = currentMouseState_;
	mouse_->GetDeviceState(sizeof(currentMouseState_), &currentMouseState_);

	GetCursorPos(&mousePoint_);
	preMousePos_ = currentMousePos_;
	currentMousePos_ = { (float)mousePoint_.x,(float)mousePoint_.y };
}
