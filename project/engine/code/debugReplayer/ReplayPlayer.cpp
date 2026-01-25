#include "ReplayPlayer.h"

/// stl
#include <filesystem>
#include <fstream>

/// engine
#include "scene/SceneManager.h"
// log
#include "logger/Logger.h"

// input
#include "input/GamepadInput.h"
#include "input/KeyboardInput.h"
#include "input/MouseInput.h"

using namespace OriGine;

/// <summary>
/// 指定されたリプレイファイルの読み込みと再生準備を行う.
/// </summary>
void ReplayPlayer::Initialize(const std::string& _filepath, SceneManager* _sceneManager) {
    if (isActive_) {
        LOG_WARN("already initialized.");
        return;
    }

    fileData_.Initialize();
    isActive_ = LoadFromFile(_filepath);

    // 読み込みに成功した場合、記録開始時のシーンへの遷移を要求する
    if (_sceneManager) {
        _sceneManager->ChangeScene(fileData_.header.startScene);
        // 最初のフレーム入力をあらかじめ適用しておく
        if (isActive_) {
            Apply(_sceneManager->keyInput_, _sceneManager->mouseInput_, _sceneManager->padInput_);
        }
    }
}

/// <summary> 再生終了処理. リソースの解放とフラグのリセットを行う. </summary>
void ReplayPlayer::Finalize() {
    if (!isActive_) {
        LOG_WARN("not initialized.");
        return;
    }

    isActive_ = false;
    fileData_.Finalize();
    currentFrameIndex_ = 0;
    filepath_.clear();
}

/// <summary>
/// バイナリ形式のリプレイファイルを解析してメモリにロードする.
/// </summary>
bool ReplayPlayer::LoadFromFile(const std::string& _filepath) {
    isActive_ = false;
    std::ifstream ifs(_filepath, std::ios::binary);
    if (!ifs.is_open()) {

#ifdef _DEBUG
        MessageBoxA(nullptr, ("Failed to open replay file: " + _filepath).c_str(), "Error", MB_OK);
#endif // _DEBUG

        LOG_CRITICAL("Failed to open replay file for reading: {}", _filepath);
        return false;
    }

    filepath_ = _filepath;

    // ===== ヘッダーのデシリアライズ =====
    {
        // 開始シーン名
        size_t length = 0;
        ifs.read(reinterpret_cast<char*>(&length), sizeof(size_t));
        fileData_.header.startScene.resize(length);
        ifs.read(fileData_.header.startScene.data(), length);

        // バージョン情報
        ifs.read(reinterpret_cast<char*>(&fileData_.header.version.major), sizeof(uint32_t));
        ifs.read(reinterpret_cast<char*>(&fileData_.header.version.minor), sizeof(uint32_t));
        ifs.read(reinterpret_cast<char*>(&fileData_.header.version.patch), sizeof(uint32_t));

        // 総フレーム数
        uint32_t frameCount = 0;
        ifs.read(reinterpret_cast<char*>(&frameCount), sizeof(uint32_t));
        fileData_.frameData.resize(frameCount);
    }

    // ===== 各フレームのデシリアライズ =====
    for (auto& frame : fileData_.frameData) {
        // 経過時間
        ifs.read(reinterpret_cast<char*>(&frame.deltaTime), sizeof(float));

        // キーボード入力（BitArray の復元）
        {
            size_t size = 0;
            ifs.read(reinterpret_cast<char*>(&size), sizeof(size));

            size_t blockCount = 0;
            ifs.read(reinterpret_cast<char*>(&blockCount), sizeof(blockCount));

            std::vector<uint32_t> blocks(blockCount);
            ifs.read(reinterpret_cast<char*>(blocks.data()),
                static_cast<std::streamsize>(blockCount * sizeof(uint32_t)));

            frame.keyInputData = BitArray<uint32_t>(size);
            for (size_t i = 0; i < blockCount; ++i) {
                frame.keyInputData.SetBlock(i, blocks[i]);
            }
        }

        // マウス入力
        {
            ifs.read(reinterpret_cast<char*>(&frame.mouseData.mousePos[X]), sizeof(float));
            ifs.read(reinterpret_cast<char*>(&frame.mouseData.mousePos[Y]), sizeof(float));
            ifs.read(reinterpret_cast<char*>(&frame.mouseData.wheelDelta), sizeof(int32_t));
            ifs.read(reinterpret_cast<char*>(&frame.mouseData.buttonData), sizeof(uint32_t));
        }

        // ゲームパッド入力
        {
            ifs.read(reinterpret_cast<char*>(&frame.padData.padState.lStick[X]), sizeof(float));
            ifs.read(reinterpret_cast<char*>(&frame.padData.padState.lStick[Y]), sizeof(float));
            ifs.read(reinterpret_cast<char*>(&frame.padData.padState.rStick[X]), sizeof(float));
            ifs.read(reinterpret_cast<char*>(&frame.padData.padState.rStick[Y]), sizeof(float));
            ifs.read(reinterpret_cast<char*>(&frame.padData.padState.lTrigger), sizeof(float));
            ifs.read(reinterpret_cast<char*>(&frame.padData.padState.rTrigger), sizeof(float));
            ifs.read(reinterpret_cast<char*>(&frame.padData.padState.buttonMask), sizeof(uint32_t));
            ifs.read(reinterpret_cast<char*>(&frame.padData.isActive), sizeof(bool));
        }
    }

    ifs.close();
    LOG_INFO("Replay file loaded successfully: {}", _filepath);

    isActive_ = true;
    return true;
}

/// <summary>
/// 現在の再生フレームに対応する入力を、各デバイスの内部状態に上書き注入する.
/// </summary>
float ReplayPlayer::Apply(KeyboardInput* _keyInput, MouseInput* _mouseInput, GamepadInput* _padInput) {
    auto& frameData = fileData_.frameData[currentFrameIndex_];

    // ========================================================================
    // 1. KeyboardInput の復元
    // ========================================================================
    {
        KeyboardState newState{};
        // キーデータの展開 (BitSetなどから復元)
        if (frameData.keyInputData.size() != 0) {
            for (size_t keyIndex = 0; keyIndex < KEY_COUNT; ++keyIndex) {
                bool isPressed          = frameData.keyInputData.Get(keyIndex);
                newState.keys[keyIndex] = isPressed ? 0x80 : 0x00;
            }
        } else {
            newState.keys.fill(0);
        }

        // 履歴に追加
        _keyInput->inputHistory_.push_front(newState);

        // サイズ制限 (InputManagerと同じ定数を使用)
        if (_keyInput->inputHistory_.size() > KeyboardInput::kInputHistoryCount) {
            _keyInput->inputHistory_.pop_back();
        }

        // フレーム0の特別処理: prev(index 1) を current(index 0) と同じにする
        // これにより、再生開始瞬間の誤った Trigger 判定を防ぐ
        if (currentFrameIndex_ == 0 && _keyInput->inputHistory_.size() >= 2) {
            _keyInput->inputHistory_[1] = _keyInput->inputHistory_[0];
        }
    }

    // ========================================================================
    // 2. MouseInput の復元
    // ========================================================================
    {
        MouseState newState{};
        newState.mousePos   = frameData.mouseData.mousePos;
        newState.wheelDelta = frameData.mouseData.wheelDelta;

        // ボタンビットマスクの復元 (保存形式に合わせてビットシフト)
        newState.buttonData = 0;
        for (size_t i = 0; i < MOUSE_BUTTON_COUNT; ++i) {
            // frameData内のデータ構造に依存しますが、元のロジックを再現
            if ((frameData.mouseData.buttonData >> i) & 1u) {
                // MouseInputで定義したenumに対応するビットを立てる
                newState.buttonData |= (1 << i);
            }
        }

        // Velocity は MouseInput::GetVelocity() で履歴から計算されるため、
        // ここで代入する必要はない (State構造体からも削除済み)

        // 履歴に追加
        _mouseInput->inputHistory_.push_front(newState);

        // 仮想座標の復元 (ユーザー操作用変数は履歴外にある場合が多いので直接セット)
        _mouseInput->SetVirtualPosition(newState.mousePos);

        // サイズ制限
        if (_mouseInput->inputHistory_.size() > MouseInput::kInputHistoryCount) {
            _mouseInput->inputHistory_.pop_back();
        }

        // フレーム0の特別処理
        if (currentFrameIndex_ == 0 && _mouseInput->inputHistory_.size() >= 2) {
            _mouseInput->inputHistory_[1] = _mouseInput->inputHistory_[0];
        }
    }

    // ========================================================================
    // 3. GamepadInput の復元
    // ========================================================================
    {
        GamepadState newState{};
        newState             = frameData.padData.padState;
        _padInput->isActive_ = frameData.padData.isActive;

        // 履歴に追加
        _padInput->inputHistory_.push_front(newState);

        // サイズ制限
        if (_padInput->inputHistory_.size() > GamepadInput::kInputHistoryCount) {
            _padInput->inputHistory_.pop_back();
        }

        // フレーム0の特別処理
        if (currentFrameIndex_ == 0 && _padInput->inputHistory_.size() >= 2) {
            _padInput->inputHistory_[1] = _padInput->inputHistory_[0];
        }
    }

    return frameData.deltaTime;
}

bool ReplayPlayer::Seek(size_t _frameIndex) {
    if (_frameIndex < fileData_.frameData.size()) {
        bool result        = (currentFrameIndex_ != _frameIndex);
        currentFrameIndex_ = _frameIndex;
        return result;
    }
    return false;
}
