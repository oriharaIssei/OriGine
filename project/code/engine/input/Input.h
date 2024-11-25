#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <wrl.h>

#include <minwindef.h>
#include <stdint.h>
#include <Vector2.h>

class Input {
public:
	static Input *getInstance();

	Input() = default;
	~Input() = default;
	Input(const Input &) = delete;
	const Input &operator=(const Input &) = delete;

	void Init();
	void Finalize();
	void Update();
private:
	Microsoft::WRL::ComPtr<IDirectInput8> directInput_ = nullptr;

	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_ = nullptr;
	BYTE keys_[256] = {};
	BYTE preKeys_[256] = {};

	Microsoft::WRL::ComPtr <IDirectInputDevice8> mouse_ = nullptr;
	DIMOUSESTATE2 currentMouseState_;
	DIMOUSESTATE2 preMouseState_;
	POINT mousePoint_;
	Vector2 currentMousePos_;
	Vector2 preMousePos_;
public:
	bool isPressKey(const uint32_t &key)const { return keys_[key]; };
	bool isTriggerKey(const uint32_t &key)const { return (keys_[key] && preKeys_[key]); }
	bool isReleaseKey(const uint32_t &key)const { return (keys_[key] && !preKeys_[key]); }

	bool isPressMouseButton(const uint32_t &key)const { return currentMouseState_.rgbButtons[key]; }
	bool isTriggerMouseButton(const uint32_t &key)const { return currentMouseState_.rgbButtons[key] && !preMouseState_.rgbButtons[key]; }
	bool isReleaseMouseButton(const uint32_t &key)const { return !currentMouseState_.rgbButtons[key] && preMouseState_.rgbButtons[key]; }

	bool isWheel()const { return currentMouseState_.lZ != 0; }
	bool isPreWheel()const { return preMouseState_.lZ != 0; }
	int getWheel()const { return static_cast<int>(currentMouseState_.lZ); }
	int getPreWheel()const { return static_cast<int>(preMouseState_.lZ); }

	const Vector2 &getCurrentMousePos()const { return currentMousePos_; }
	const Vector2 &getPreMousePos()const { return preMousePos_; }
	 Vector2 getMouseVelocity()const { return Vector2(currentMousePos_ - preMousePos_); }
};