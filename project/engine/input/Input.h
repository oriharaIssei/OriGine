#pragma once

#define DIRECTINPUT_VERSION 0x0800
/// stl
#include <array>
#include <stdint.h>

/// microsoft
#include <wrl.h>
/// input API
#include <dinput.h>
#include <Xinput.h>

// lib
#include "lib/globalVariables/SerializedField.h"
// math
#include <math/Vector2.h>

static const float stickMax_ = static_cast<float>(SHORT_MAX);
static const float stickMin_ = static_cast<float>(-SHORT_MAX);

class Input {
public:
    static Input* getInstance();

    Input()                              = default;
    ~Input()                             = default;
    Input(const Input&)                  = delete;
    const Input& operator=(const Input&) = delete;

    void Initialize();
    void Finalize();
    void Update();

private:
    Microsoft::WRL::ComPtr<IDirectInput8> directInput_ = nullptr;

    Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_ = nullptr;
    BYTE keys_[256]                                       = {};
    BYTE preKeys_[256]                                    = {};

    Microsoft::WRL::ComPtr<IDirectInputDevice8> mouse_ = nullptr;
    DIMOUSESTATE2 currentMouseState_;
    DIMOUSESTATE2 preMouseState_;

    POINT mousePoint_;
    Vec2f currentMousePos_;
    Vec2f preMousePos_;

    // ゲームパッド とりあえず 一つだけ
    XINPUT_STATE padState_;
    XINPUT_STATE prePadState_;
    bool isPadActive_;
    SerializedField<float> deadZone_{"Input", "GamePad", "DeadZone"};
    Vec2f currentLStickVelocity_ = {0.0f, 0.0f};
    Vec2f preLStickVelocity_     = {0.0f, 0.0f};

    Vec2f currentRStickVelocity_ = {0.0f, 0.0f};
    Vec2f preRStickVelocity_     = {0.0f, 0.0f};

public:
    bool isPressKey(const uint32_t& key) const { return keys_[key]; };
    bool isTriggerKey(const uint32_t& key) const { return (keys_[key] && !preKeys_[key]); }
    bool isReleaseKey(const uint32_t& key) const { return (!keys_[key] && preKeys_[key]); }

    bool isPressMouseButton(const uint32_t& key) const { return currentMouseState_.rgbButtons[key]; }
    bool isTriggerMouseButton(const uint32_t& key) const { return currentMouseState_.rgbButtons[key] && !preMouseState_.rgbButtons[key]; }
    bool isReleaseMouseButton(const uint32_t& key) const { return !currentMouseState_.rgbButtons[key] && preMouseState_.rgbButtons[key]; }

    bool isWheel() const { return currentMouseState_.lZ != 0; }
    bool isPreWheel() const { return preMouseState_.lZ != 0; }
    int getWheel() const { return static_cast<int>(currentMouseState_.lZ); }
    int getPreWheel() const { return static_cast<int>(preMouseState_.lZ); }

    const Vec2f& getCurrentMousePos() const { return currentMousePos_; }
    const Vec2f& getPreMousePos() const { return preMousePos_; }
    Vec2f getMouseVelocity() const { return Vec2f(currentMousePos_ - preMousePos_); }

    const Vec2f& getLStickVelocity() const { return currentLStickVelocity_; }
    const Vec2f& getPreLStickVelocity() const { return preLStickVelocity_; }

    const Vec2f& getRStickVelocity() const { return currentRStickVelocity_; }
    const Vec2f& getPreRStickVelocity() const { return preRStickVelocity_; }

    bool isPadActive() const { return isPadActive_; }
    bool isPressButton(const uint32_t& button) const { return padState_.Gamepad.wButtons & button; }
    bool isTriggerButton(const uint32_t& button) const { return padState_.Gamepad.wButtons & button && !(prePadState_.Gamepad.wButtons & button); }
    bool isReleaseButton(const uint32_t& button) const { return !(padState_.Gamepad.wButtons & button) && prePadState_.Gamepad.wButtons & button; }
};
