#pragma once

/// stl
#include <array>
#include <map>
#include <string>

/// input API
#include "include/IncludeInputAPI.h"

/// math
#include <cmath>

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
    RCTRL      = DIK_RCONTROL,
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
    RALT       = DIK_RMENU,
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
    RIGHT_CONTROL = RCTRL,
    R_CTRL        = RCTRL,
    LEFT_SHIFT    = LSHIFT,
    L_SHIFT       = LSHIFT,
    RIGHT_SHIFT   = RSHIFT,
    R_SHIFT       = RSHIFT,
    LEFT_ALT      = LALT,
    L_ALT         = LALT,
    RIGHT_ALT     = RALT,
    R_ALT         = RALT
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
    {Key::R_CTRL, "R_CTRL"},
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

    {Key::UP, "UP"},
    {Key::DOWN, "DOWN"},
    {Key::LEFT, "LEFT"},
    {Key::RIGHT, "RIGHT"},

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

constexpr uint32_t KEY_COUNT = 256;

class KeyboardInput {
    friend class ReplayPlayer;

public:
    KeyboardInput()  = default;
    ~KeyboardInput() = default;

    KeyboardInput(const KeyboardInput&)            = delete;
    KeyboardInput& operator=(const KeyboardInput&) = delete;

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="directInput">DirectInput8 インターフェース</param>
    /// <param name="hwnd">ウィンドウハンドル</param>
    void Initialize(IDirectInput8* directInput, HWND hwnd);

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 解放処理
    /// </summary>
    void Finalize();

private:
    Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_;
    std::array<BYTE, KEY_COUNT> keys_{};
    std::array<BYTE, KEY_COUNT> prevKeys_{};

public:
    const std::array<BYTE, KEY_COUNT>& GetKeyStates() const { return keys_; }
    const std::array<BYTE, KEY_COUNT>& GetPrevKeyStates() const { return prevKeys_; }

    /// <summary>
    /// キー状態をクリアする(今と前回分の状態を初期化)
    /// </summary>
    void ClearKeyStates() {
        keys_.fill(0);
        prevKeys_.fill(0);
    }

    /// <summary>
    /// キーが押されているか
    /// </summary>
    bool IsPress(uint32_t key) const { return keys_[key]; }
    bool IsPress(Key key) const { return keys_[static_cast<uint32_t>(key)]; }

    /// <summary>
    /// 押した瞬間か
    /// </summary>
    bool IsTrigger(uint32_t key) const { return (keys_[key] && !prevKeys_[key]); }
    bool IsTrigger(Key key) const {
        uint32_t keyNum = static_cast<uint32_t>(key);
        return (keys_[keyNum] && !prevKeys_[keyNum]);
    }

    /// <summary>
    /// 離した瞬間か
    /// </summary>
    bool IsRelease(uint32_t key) const { return (!keys_[key] && prevKeys_[key]); }
    bool IsRelease(Key key) const {
        uint32_t keyNum = static_cast<uint32_t>(key);
        return (!keys_[keyNum] && prevKeys_[keyNum]);
    }
};
