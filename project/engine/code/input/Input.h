#pragma once

#define DIRECTINPUT_VERSION 0x0800
/// stl
#include <array>
#include <map>
#include <stdint.h>
#include <string>

/// microsoft
#include <wrl.h>
/// input API
#include <dinput.h>
#include <Xinput.h>


#include "util/globalVariables/SerializedField.h"
// math
#include <math/Vector2.h>

static const float stickMax_ = static_cast<float>(SHORT_MAX);
static const float stickMin_ = static_cast<float>(-SHORT_MAX);

enum class PadButton : uint32_t {
    UP         = XINPUT_GAMEPAD_DPAD_UP,
    DOWN       = XINPUT_GAMEPAD_DPAD_DOWN,
    LEFT       = XINPUT_GAMEPAD_DPAD_LEFT,
    RIGHT      = XINPUT_GAMEPAD_DPAD_RIGHT,
    START      = XINPUT_GAMEPAD_START,
    BACK       = XINPUT_GAMEPAD_BACK,
    L_THUMB    = XINPUT_GAMEPAD_LEFT_THUMB,
    R_THUMB    = XINPUT_GAMEPAD_RIGHT_THUMB,
    L_SHOULDER = XINPUT_GAMEPAD_LEFT_SHOULDER,
    R_SHOULDER = XINPUT_GAMEPAD_RIGHT_SHOULDER,
    A          = XINPUT_GAMEPAD_A,
    B          = XINPUT_GAMEPAD_B,
    X          = XINPUT_GAMEPAD_X,
    Y          = XINPUT_GAMEPAD_Y,
};
static std::map<PadButton, std::string> padButtonNameMap = {
    {PadButton::UP, "UP"},
    {PadButton::DOWN, "DOWN"},
    {PadButton::LEFT, "LEFT"},
    {PadButton::RIGHT, "RIGHT"},
    {PadButton::START, "START"},
    {PadButton::BACK, "BACK"},
    {PadButton::L_THUMB, "L_THUMB"},
    {PadButton::R_THUMB, "R_THUMB"},
    {PadButton::L_SHOULDER, "L_SHOULDER"},
    {PadButton::R_SHOULDER, "R_SHOULDER"},
    {PadButton::A, "A"},
    {PadButton::B, "B"},
    {PadButton::X, "X"},
    {PadButton::Y, "Y"}};

enum class Key : uint32_t {
    ONE   = DIK_1,
    TWO   = DIK_2,
    THREE = DIK_3,
    FOUR  = DIK_4,
    FIVE  = DIK_5,
    SIX   = DIK_6,
    SEVEN = DIK_7,
    EIGHT = DIK_8,
    NINE  = DIK_9,
    ZERO  = DIK_0,

    A = DIK_A,
    B = DIK_B,
    C = DIK_C,
    D = DIK_D,
    E = DIK_E,
    F = DIK_F,
    G = DIK_G,
    H = DIK_H,
    I = DIK_I,
    J = DIK_J,
    K = DIK_K,
    L = DIK_L,
    M = DIK_M,
    N = DIK_N,
    O = DIK_O,
    P = DIK_P,
    Q = DIK_Q,
    R = DIK_R,
    S = DIK_S,
    T = DIK_T,
    U = DIK_U,
    V = DIK_V,
    W = DIK_W,
    X = DIK_X,
    Y = DIK_Y,
    Z = DIK_Z,

    F1  = DIK_F1,
    F2  = DIK_F2,
    F3  = DIK_F3,
    F4  = DIK_F4,
    F5  = DIK_F5,
    F6  = DIK_F6,
    F7  = DIK_F7,
    F8  = DIK_F8,
    F9  = DIK_F9,
    F10 = DIK_F10,
    F11 = DIK_F11,
    F12 = DIK_F12,

    UP    = DIK_UP,
    DOWN  = DIK_DOWN,
    LEFT  = DIK_LEFT,
    RIGHT = DIK_RIGHT,

    ESCAPE     = DIK_ESCAPE,
    MINUS      = DIK_MINUS,
    EQUALS     = DIK_EQUALS,
    BACKSPACE  = DIK_BACK,
    TAB        = DIK_TAB,
    LBRACKET   = DIK_LBRACKET,
    RBRACKET   = DIK_RBRACKET,
    ENTER      = DIK_RETURN,
    LCTRL      = DIK_LCONTROL,
    SEMICOLON  = DIK_SEMICOLON,
    APOSTROPHE = DIK_APOSTROPHE,
    GRAVE      = DIK_GRAVE,
    LSHIFT     = DIK_LSHIFT,
    BACKSLASH  = DIK_BACKSLASH,
    COMMA      = DIK_COMMA,
    PERIOD     = DIK_PERIOD,
    SLASH      = DIK_SLASH,
    RSHIFT     = DIK_RSHIFT,
    MULTIPLY   = DIK_MULTIPLY,
    LALT       = DIK_LMENU,
    SPACE      = DIK_SPACE,
    CAPITAL    = DIK_CAPITAL,
    NUMLOCK    = DIK_NUMLOCK,
    SCROLL     = DIK_SCROLL,
    NUMPAD7    = DIK_NUMPAD7,
    NUMPAD8    = DIK_NUMPAD8,
    NUMPAD9    = DIK_NUMPAD9,
    SUBTRACT   = DIK_SUBTRACT,
    NUMPAD4    = DIK_NUMPAD4,
    NUMPAD5    = DIK_NUMPAD5,
    NUMPAD6    = DIK_NUMPAD6,
    ADD        = DIK_ADD,
    NUMPAD1    = DIK_NUMPAD1,
    NUMPAD2    = DIK_NUMPAD2,
    NUMPAD3    = DIK_NUMPAD3,
    NUMPAD0    = DIK_NUMPAD0,
    DECIMAL    = DIK_DECIMAL,

    // エイリアス
    NUM_1         = ONE,
    NUM_2         = TWO,
    NUM_3         = THREE,
    NUM_4         = FOUR,
    NUM_5         = FIVE,
    NUM_6         = SIX,
    NUM_7         = SEVEN,
    NUM_8         = EIGHT,
    NUM_9         = NINE,
    NUM_0         = ZERO,
    LEFT_BRACKET  = LBRACKET,
    L_BRACKET     = LBRACKET,
    RIGHT_BRACKET = RBRACKET,
    R_BRACKET     = RBRACKET,
    LEFT_CONTROL  = LCTRL,
    L_CTRL        = LCTRL,
    LEFT_SHIFT    = LSHIFT,
    L_SHIFT       = LSHIFT,
    RIGHT_SHIFT   = RSHIFT,
    R_SHIFT       = RSHIFT,
    LEFT_ALT      = LALT,
    L_ALT         = LALT,
};
static std::map<Key, std::string> keyNameMap = {
    {Key::ONE, "ONE"},
    {Key::TWO, "TWO"},
    {Key::THREE, "THREE"},
    {Key::FOUR, "FOUR"},
    {Key::FIVE, "FIVE"},
    {Key::SIX, "SIX"},
    {Key::SEVEN, "SEVEN"},
    {Key::EIGHT, "EIGHT"},
    {Key::NINE, "NINE"},
    {Key::ZERO, "ZERO"},

    {Key::A, "A"},
    {Key::B, "B"},
    {Key::C, "C"},
    {Key::D, "D"},
    {Key::E, "E"},
    {Key::F, "F"},
    {Key::G, "G"},
    {Key::H, "H"},
    {Key::I, "I"},
    {Key::J, "J"},
    {Key::K, "K"},
    {Key::L, "L"},
    {Key::M, "M"},
    {Key::N, "N"},
    {Key::O, "O"},
    {Key::P, "P"},
    {Key::Q, "Q"},
    {Key::R, "R"},
    {Key::S, "S"},
    {Key::T, "T"},
    {Key::U, "U"},
    {Key::V, "V"},
    {Key::W, "W"},
    {Key::X, "X"},
    {Key::Y, "Y"},
    {Key::Z, "Z"},

    {Key::ESCAPE, "ESCAPE"},
    {Key::MINUS, "MINUS"},
    {Key::EQUALS, "EQUALS"},
    {Key::BACKSPACE, "BACKSPACE"},
    {Key::TAB, "TAB"},
    {Key::L_BRACKET, "L_BRACKET"},
    {Key::R_BRACKET, "R_BRACKET"},
    {Key::ENTER, "ENTER"},
    {Key::L_CTRL, "L_CTRL"},
    {Key::SEMICOLON, "SEMICOLON"},
    {Key::APOSTROPHE, "APOSTROPHE"},
    {Key::GRAVE, "GRAVE"},
    {Key::L_SHIFT, "L_SHIFT"},
    {Key::BACKSLASH, "BACKSLASH"},
    {Key::COMMA, "COMMA"},
    {Key::PERIOD, "PERIOD"},
    {Key::SLASH, "SLASH"},
    {Key::R_SHIFT, "R_SHIFT"},
    {Key::MULTIPLY, "MULTIPLY"},
    {Key::L_ALT, "L_ALT"},
    {Key::SPACE, "SPACE"},
    {Key::CAPITAL, "CAPITAL"},
    {Key::F1, "F1"},
    {Key::F2, "F2"},
    {Key::F3, "F3"},
    {Key::F4, "F4"},
    {Key::F5, "F5"},
    {Key::F6, "F6"},
    {Key::F7, "F7"},
    {Key::F8, "F8"},
    {Key::F9, "F9"},
    {Key::F10, "F10"},
    {Key::F11, "F11"},
    {Key::F12, "F12"},
    {Key::NUMLOCK, "NUMLOCK"},
    {Key::SCROLL, "SCROLL"},
    {Key::NUMPAD7, "NUMPAD7"},
    {Key::NUMPAD8, "NUMPAD8"},
    {Key::NUMPAD9, "NUMPAD9"},
    {Key::SUBTRACT, "SUBTRACT"},
    {Key::NUMPAD4, "NUMPAD4"},
    {Key::NUMPAD5, "NUMPAD5"},
    {Key::NUMPAD6, "NUMPAD6"},
    {Key::ADD, "ADD"},
    {Key::NUMPAD1, "NUMPAD1"},
    {Key::NUMPAD2, "NUMPAD2"},
    {Key::NUMPAD3, "NUMPAD3"},
    {Key::NUMPAD0, "NUMPAD0"},
    {Key::DECIMAL, "DECIMAL"}};

enum class MouseButton : uint32_t {
    LEFT   = 0,
    RIGHT  = 1,
    MIDDLE = 2,
};
static std::map<MouseButton, std::string> mouseButtonName = {
    {MouseButton::LEFT, "LEFT"},
    {MouseButton::RIGHT, "RIGHT"},
    {MouseButton::MIDDLE, "MIDDLE"},
};

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

    /// マウス
    Microsoft::WRL::ComPtr<IDirectInputDevice8> mouse_ = nullptr;
    DIMOUSESTATE2 currentMouseState_;
    DIMOUSESTATE2 preMouseState_;
    // 座標
    POINT mousePoint_;
    Vec2f currentMousePos_;
    Vec2f preMousePos_;
    bool virtualMouseIsSynchronizedWithClientMouse_ = false;
    Vec2f virtualMousePos_;
    // マウスカーソルを隠すかどうか
    bool isHideMouseCursor_ = false;
    // マウス座標を固定するかどうか
    bool isMousePosFixed_ = false;

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
    /// キーボード
    bool isPressKey(const uint32_t& key) const { return keys_[key]; }
    bool isPressKey(Key key) const { return keys_[static_cast<int32_t>(key)]; }
    bool isTriggerKey(const uint32_t& key) const { return (keys_[key] && !preKeys_[key]); }
    bool isTriggerKey(Key key) const {
        int32_t keyNum = static_cast<int32_t>(key);
        return (keys_[keyNum] && !preKeys_[keyNum]);
    }
    bool isReleaseKey(const uint32_t& key) const { return (!keys_[key] && preKeys_[key]); }
    bool isReleaseKey(Key key) const {
        int32_t keyNum = static_cast<int32_t>(key);
        return (!keys_[keyNum] && preKeys_[keyNum]);
    }

    /// マウス
    bool isPressMouseButton(const uint32_t& _mouse) const { return currentMouseState_.rgbButtons[_mouse]; }
    bool isPressMouseButton(MouseButton mouse) const { return currentMouseState_.rgbButtons[static_cast<int32_t>(mouse)]; }
    bool isTriggerMouseButton(const uint32_t& _mouse) const { return currentMouseState_.rgbButtons[_mouse] && !preMouseState_.rgbButtons[_mouse]; }
    bool isTriggerMouseButton(MouseButton mouse) const {
        int32_t mouseNum = static_cast<int32_t>(mouse);
        return currentMouseState_.rgbButtons[mouseNum] && !preMouseState_.rgbButtons[mouseNum];
    }
    bool isReleaseMouseButton(const uint32_t& _mouse) const { return !currentMouseState_.rgbButtons[_mouse] && preMouseState_.rgbButtons[_mouse]; }
    bool isReleaseMouseButton(MouseButton mouse) const {
        int32_t mouseNum = static_cast<int32_t>(mouse);
        return !currentMouseState_.rgbButtons[mouseNum] && preMouseState_.rgbButtons[mouseNum];
    }

    bool isWheel() const { return currentMouseState_.lZ != 0; }
    bool isPreWheel() const { return preMouseState_.lZ != 0; }
    int getWheel() const { return static_cast<int>(currentMouseState_.lZ); }
    int getPreWheel() const { return static_cast<int>(preMouseState_.lZ); }

    bool synchronizeVirtualMouseWithClientMouse() const { return virtualMouseIsSynchronizedWithClientMouse_; }
    const Vec2f& getCurrentMousePos() const { return currentMousePos_; }
    const Vec2f& getPreMousePos() const { return preMousePos_; }
    const Vec2f& getVirtualMousePos() const { return virtualMousePos_; }
    /// <summary>
    /// マウスの座標を固定する
    /// </summary>
    /// <param name="_fixedPos">固定座標(スクリーン座標系)</param>
    void FixMousePos(const Vec2f& _fixedPos);
    void FixMousePos(const Vec2d& _fixedPos);

    /// <summary>
    /// マウスの座標の固定を解除する
    /// </summary>
    void ReleaseMousePos() {
        isMousePosFixed_ = false;
        ClipCursor(NULL);
    }

    void setVirtualMousePos(const Vec2f& pos) { virtualMousePos_ = pos; }
    Vec2f getMouseVelocity() const { return currentMousePos_ - preMousePos_; }

    bool isHideMouseCursor() const { return isHideMouseCursor_; }
    void setIsHideMouseCursor(bool _isHideCursor) {
        isHideMouseCursor_ = _isHideCursor;
        while (ShowCursor(BOOL(isHideMouseCursor_)) >= 0) {}
    }

    /// gamePad
    const Vec2f& getLStickVelocity() const { return currentLStickVelocity_; }
    const Vec2f& getPreLStickVelocity() const { return preLStickVelocity_; }

    const Vec2f& getRStickVelocity() const { return currentRStickVelocity_; }
    const Vec2f& getPreRStickVelocity() const { return preRStickVelocity_; }

    bool isPadActive() const { return isPadActive_; }
    bool isPressButton(const uint32_t& button) const { return padState_.Gamepad.wButtons & button; }
    bool isPressButton(PadButton button) const { return padState_.Gamepad.wButtons & static_cast<uint32_t>(button); }
    bool isTriggerButton(const uint32_t& button) const { return padState_.Gamepad.wButtons & button && !(prePadState_.Gamepad.wButtons & button); }
    bool isTriggerButton(PadButton button) const {
        int32_t buttonNum = static_cast<int32_t>(button);
        return padState_.Gamepad.wButtons & buttonNum && !(prePadState_.Gamepad.wButtons & buttonNum);
    }
    bool isReleaseButton(const uint32_t& button) const { return !(padState_.Gamepad.wButtons & button) && prePadState_.Gamepad.wButtons & button; }
    bool isReleaseButton(PadButton button) const {
        int32_t buttonNum = static_cast<int32_t>(button);
        return !(padState_.Gamepad.wButtons & buttonNum) && prePadState_.Gamepad.wButtons & buttonNum;
    }
};
