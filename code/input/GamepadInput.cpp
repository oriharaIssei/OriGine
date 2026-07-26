#include "GamepadInput.h"

/// engine
#include "EngineConfig.h"

/// math
#include "math/MathEnv.h"
#include <cmath>

using namespace OriGine;

/// <summary>
/// ゲームパッドの初期化
/// </summary>
void GamepadInput::Initialize() {
    // 初期入力を取得しておく
    Update();

    if (!inputHistory_.empty()) {
        inputHistory_.clear();
    }
}

/// <summary>
/// 状態更新
/// </summary>
void GamepadInput::Update() {
    // XInput更新
    // XInputGetState の第1引数はコントローラーのスロット番号(0~3)。
    // このエンジンでは常に 0 番（1P固定）のみを扱う設計になっている。
    // 戻り値が ERROR_SUCCESS でない場合はそのスロットにコントローラーが接続されていないことを意味するため、
    // isActive_ の判定に使う
    XINPUT_STATE state{};
    isActive_ = (XInputGetState(0, &state) == ERROR_SUCCESS);

    GamepadState currentState{};
    // 非アクティブ時は空の状態を履歴に追加して終了
    if (!isActive_) {
        inputHistory_.push_back(currentState);
        return;
    }

    // デジタルボタン
    currentState.buttonMask |= state.Gamepad.wButtons;

    // アナログトリガーを正規化
    // デッドゾーンを引いたあと、引いた分だけ狭まった範囲(triggerMax)で割り直すのが要点。
    // 単に引くだけだと最大まで押し込んでも1.0に届かず、入力の上限が失われてしまう。
    // max(...,0)で下限を切っているのは、デッドゾーン未満の微小な入力を完全に0にするため
    // (この処理が無いと、指を離していてもスティックのわずかな傾きで動き続ける)
    float triggerDeadZoneVal = *triggerDeadZone_.GetValue();
    float triggerMax         = kTriggerMax - triggerDeadZoneVal;
    currentState.lTrigger    = (std::max)(static_cast<float>(state.Gamepad.bLeftTrigger) - triggerDeadZoneVal, 0.f) / triggerMax;
    currentState.rTrigger    = (std::max)(static_cast<float>(state.Gamepad.bRightTrigger) - triggerDeadZoneVal, 0.f) / triggerMax;

    UpdateVirtualTriggerButtons(currentState);
    UpdateStickValues(state, currentState);
    UpdateVirtualStickButtons(currentState);

    // 入力履歴に追加
    inputHistory_.push_front(currentState);
    // 履歴が多すぎたら削除
    if (inputHistory_.size() > Config::Input::kHistoryCount) {
        inputHistory_.pop_back();
    }
}

/// <summary>
/// 解放処理. 入力履歴をクリアする.
/// </summary>
void GamepadInput::Finalize() {
    if (!inputHistory_.empty()) {
        inputHistory_.clear();
    }
}

/// <summary>
/// 履歴をクリアする.
/// シーン切り替え等で「前フレームの入力」が新しい文脈と無関係になるタイミングに呼び出すことで、
/// 直後の IsTrigger/IsRelease が古い状態と比較して誤判定するのを防ぐ.
/// クリア後に空状態を1つ積んでおくことで、GetState(0) が即座に有効な値を返せるようにしている.
/// </summary>
void OriGine::GamepadInput::ClearHistory() {
    // 履歴を全てクリアし、空の状態を1つ積んでおく
    if (!inputHistory_.empty()) {
        inputHistory_.clear();
    }
    GamepadState emptyState{};
    inputHistory_.push_front(emptyState);
}

/// <summary>
/// スティックの値を正規化して更新
/// </summary>
void GamepadInput::UpdateStickValues(XINPUT_STATE _state, GamepadState& _currentState) {
    // deadZoneとdeadZoneを除去したあとの最大値を計算
    float deadZone = *deadZone_.GetValue();
    // 0 ~ 1の範囲で考える そこからdeadZone分を引いた値が最大値になる
    float stickMax = 1.f - deadZone;

    // deadZoneを考慮して正規化するラムダ関数
    auto normalizeStick = [this, deadZone, stickMax](SHORT _x, SHORT _y) -> Vec2f {
        float realX = static_cast<float>(_x) / kStickMax;
        float realY = static_cast<float>(_y) / kStickMax;

        // スティックは中立を0として±に振れるため、絶対値でデッドゾーン処理をしてから
        // 元の符号を掛け戻す。符号ごと計算すると負側でデッドゾーンが逆向きに働いてしまう
        float signX = realX >= 0.f ? 1.0f : -1.0f;
        float signY = realY >= 0.f ? 1.0f : -1.0f;

        Vec2f result = Vec2f(0.0f, 0.0f);

        result[X] = signX * (std::max)(std::abs(realX) - deadZone, 0.f);
        result[X] /= stickMax;

        result[Y] = signY * (std::max)(std::abs(realY) - deadZone, 0.f);
        result[Y] /= stickMax;

        return result;
    };

    _currentState.lStick = normalizeStick(_state.Gamepad.sThumbLX, _state.Gamepad.sThumbLY);
    _currentState.rStick = normalizeStick(_state.Gamepad.sThumbRX, _state.Gamepad.sThumbRY);
}

/// <summary>
/// 仮想スティックボタンの状態をスティックの値から更新する.
/// アナログ入力であるスティックの傾きを、IsPress/IsTrigger 等のデジタルボタン判定APIでも
/// 扱えるようにするため、閾値(kEpsilon)を超えた傾きを仮想ボタンのビットマスクへ変換する.
/// </summary>
void GamepadInput::UpdateVirtualStickButtons(GamepadState& _currentState) {
    // 仮想左スティックボタン
    if (_currentState.lStick[Y] > kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::L_STICK_UP);
    }
    if (_currentState.lStick[Y] < -kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::L_STICK_DOWN);
    }
    if (_currentState.lStick[X] < -kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::L_STICK_LEFT);
    }
    if (_currentState.lStick[X] > kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::L_STICK_RIGHT);
    }
    // 仮想右スティックボタン
    if (_currentState.rStick[Y] > kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::R_STICK_UP);
    }
    if (_currentState.rStick[Y] < -kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::R_STICK_DOWN);
    }
    if (_currentState.rStick[X] < -kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::R_STICK_LEFT);
    }
    if (_currentState.rStick[X] > kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::R_STICK_RIGHT);
    }
}

/// <summary>
/// 仮想トリガーボタンの状態をトリガーの値から更新する.
/// アナログトリガーの押し込み量が正規化後に 0 より大きければ「押されている」とみなし、
/// L_TRIGGER/R_TRIGGER の仮想ボタンビットを立てる.
/// </summary>
void OriGine::GamepadInput::UpdateVirtualTriggerButtons(GamepadState& _currentState) {
    // アナログトリガーをボタン扱いに変換
    if (_currentState.lTrigger > kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::L_TRIGGER);
    }
    if (_currentState.rTrigger > kEpsilon) {
        _currentState.buttonMask |= static_cast<uint32_t>(GamepadButton::R_TRIGGER);
    }
}

// ==========================================
// 1. 基本的な状態取得
// ==========================================

/// <summary>
/// 最新フレーム（履歴インデックス0）の生状態を取得する.
/// 履歴が空（未初期化・未接続で一度も Update されていない等）の場合は静的な空状態を返し、
/// 呼び出し側で毎回 nullptr チェックをしなくても済むようにしている.
/// </summary>
/// <returns>最新のゲームパッド状態</returns>
const GamepadState& OriGine::GamepadInput::GetCurrentState() const {
    const auto* state = GetState(0);
    if (state) {
        return *state;
    }
    static GamepadState emptyState{};
    return emptyState;
}

/// <summary>
/// ボタンが押されているか (Hold) を判定する. 現在フレームの状態のみを見る（前フレームとの比較はしない）.
/// </summary>
/// <param name="_button">判定対象のボタン</param>
/// <returns>押されていれば true</returns>
bool GamepadInput::IsPress(GamepadButton _button) const {
    // 最新のフレーム (index 0) を見る
    const auto* current = GetState(0);
    if (!current) {
        return false;
    }

    return (current->buttonMask & static_cast<uint32_t>(_button)) != 0;
}

/// <summary> 左スティックの正規化済み入力値を取得する. </summary>
/// <returns>デッドゾーン適用・正規化済みの XY 値（-1.0～1.0）</returns>
Vec2f GamepadInput::GetLeftStick() const {
    const auto* current = GetState(0);
    return current ? current->lStick : Vec2f{0.0f, 0.0f};
}

/// <summary> 右スティックの正規化済み入力値を取得する. </summary>
/// <returns>デッドゾーン適用・正規化済みの XY 値（-1.0～1.0）</returns>
Vec2f GamepadInput::GetRightStick() const {
    const auto* current = GetState(0);
    return current ? current->rStick : Vec2f{0.0f, 0.0f};
}

/// <summary> 左トリガーの正規化済み入力値を取得する. </summary>
/// <returns>デッドゾーン適用・正規化済みの押し込み量（0.0～1.0）</returns>
float GamepadInput::GetLeftTrigger() const {
    const auto* current = GetState(0);
    return current ? current->lTrigger : 0.0f;
}

/// <summary> 右トリガーの正規化済み入力値を取得する. </summary>
/// <returns>デッドゾーン適用・正規化済みの押し込み量（0.0～1.0）</returns>
float GamepadInput::GetRightTrigger() const {
    const auto* current = GetState(0);
    return current ? current->rTrigger : 0.0f;
}

// ==========================================
// 2. エッジ検出 (履歴比較)
// ==========================================

bool GamepadInput::IsTrigger(GamepadButton _button) const {
    return IsTrigger(static_cast<uint32_t>(_button));
}

bool OriGine::GamepadInput::IsTrigger(uint32_t _buttonMask) const {
    // 比較には最低2フレーム必要
    const auto* current = GetState(0);
    const auto* prev    = GetState(1);

    if (!current || !prev) {
        return false;
    }

    bool isDownNow  = (current->buttonMask & _buttonMask) != 0;
    bool isDownPrev = (prev->buttonMask & _buttonMask) != 0;

    // 「今は押されている」かつ「前は押されていない」
    if (isDownNow && !isDownPrev) {
        return true;
    }

    return false;
}

bool GamepadInput::IsRelease(GamepadButton _button) const {
    return IsRelease(static_cast<uint32_t>(_button));
}

bool OriGine::GamepadInput::IsRelease(uint32_t _buttonMask) const {
    const auto* current = GetState(0);
    const auto* prev    = GetState(1);

    if (!current || !prev) {
        return false;
    }

    bool isDownNow  = (current->buttonMask & _buttonMask) != 0;
    bool isDownPrev = (prev->buttonMask & _buttonMask) != 0;

    // 「今は押されていない」かつ「前は押されていた」
    return !isDownNow && isDownPrev;
}

// ==========================================
// 3. 履歴を利用した拡張判定
// ==========================================
bool GamepadInput::WasPressedRecently(GamepadButton _button, size_t _framesToCheck) const {
    // 履歴サイズを超えないように制限
    size_t checkCount = (std::min)(inputHistory_.size(), _framesToCheck);
    uint32_t mask     = static_cast<uint32_t>(_button);

    // 過去 N フレームを走査
    for (int i = 0; i < checkCount; ++i) {
        if ((inputHistory_[i].buttonMask & mask) != 0) {
            return true; // 押されていた瞬間が見つかった
        }
    }
    return false;
}

bool OriGine::GamepadInput::WasTriggeredRecently(GamepadButton _button, size_t _framesToCheck) const {
    // 履歴サイズを超えないように制限
    size_t checkCount = (std::min)(inputHistory_.size() - 1, _framesToCheck - 1);
    uint32_t mask     = static_cast<uint32_t>(_button);
    // 過去 N フレームを走査
    for (int i = 0; i < checkCount; ++i) {
        bool isDownNow  = (inputHistory_[i].buttonMask & mask) != 0;
        bool isDownPrev = (inputHistory_[i + 1].buttonMask & mask) != 0;
        // 「今は押されている」かつ「前は押されていない」
        if (isDownNow && !isDownPrev) {
            return true; // 押された瞬間が見つかった
        }
    }
    return false;
}

bool OriGine::GamepadInput::WasReleasedRecently(GamepadButton _button, size_t _framesToCheck) const {
    // 履歴サイズを超えないように制限
    size_t checkCount = (std::min)(inputHistory_.size() - 1, _framesToCheck - 1);
    uint32_t mask     = static_cast<uint32_t>(_button);
    // 過去 N フレームを走査
    for (int i = 0; i < checkCount; ++i) {
        bool isDownNow  = (inputHistory_[i].buttonMask & mask) != 0;
        bool isDownPrev = (inputHistory_[i + 1].buttonMask & mask) != 0;
        // 「今は押されていない」かつ「前は押されていた」
        if (!isDownNow && isDownPrev) {
            return true; // 離された瞬間が見つかった
        }
    }
    return false;
}

bool GamepadInput::IsPressedDuration(GamepadButton _button, size_t _frames) const {
    // 履歴が足りなければ false
    if (inputHistory_.size() < static_cast<size_t>(_frames)) {
        return false;
    }

    uint32_t mask = static_cast<uint32_t>(_button);

    // 指定フレームの間、ずっと押され続けているかチェック
    for (int i = 0; i < _frames; ++i) {
        if ((inputHistory_[i].buttonMask & mask) == 0) {
            return false; // 途中で離している
        }
    }
    return true;
}
