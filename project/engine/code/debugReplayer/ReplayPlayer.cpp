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
            ifs.read(reinterpret_cast<char*>(&frame.padData.lStick[X]), sizeof(float));
            ifs.read(reinterpret_cast<char*>(&frame.padData.lStick[Y]), sizeof(float));
            ifs.read(reinterpret_cast<char*>(&frame.padData.rStick[X]), sizeof(float));
            ifs.read(reinterpret_cast<char*>(&frame.padData.rStick[Y]), sizeof(float));
            ifs.read(reinterpret_cast<char*>(&frame.padData.lTrigger), sizeof(float));
            ifs.read(reinterpret_cast<char*>(&frame.padData.rTrigger), sizeof(float));
            ifs.read(reinterpret_cast<char*>(&frame.padData.buttonData), sizeof(uint32_t));
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

    // 入力履歴（prev 状態）の復元
    // これにより、再生中も Trigger/Release 判定が正しく機能する
    if (currentFrameIndex_ == 0) {
        // 再生開始時：現在の値をそのまま prev にセット
        // key入力
        for (size_t keyIndex = 0; keyIndex < KEY_COUNT; ++keyIndex) {
            bool isPressed                 = frameData.keyInputData.Get(keyIndex);
            _keyInput->prevKeys_[keyIndex] = isPressed ? 0x80 : 0x00;
        }

        // mouse入力
        _mouseInput->prevPos_        = frameData.mouseData.mousePos;
        _mouseInput->prevWheelDelta_ = frameData.mouseData.wheelDelta;
        for (size_t mouseButtonIndex = 0; mouseButtonIndex < MOUSE_BUTTON_COUNT; ++mouseButtonIndex) {
            _mouseInput->prevButtonStates_[mouseButtonIndex] = (frameData.mouseData.buttonData >> mouseButtonIndex) & 1u;
        }

        // pad入力
        _padInput->prevButtonMask_ = frameData.padData.buttonData;

    } else {
        // 2フレーム目以降：前回の current 値を prev へ移動
        // key入力
        _keyInput->prevKeys_ = _keyInput->keys_;

        // mouse入力
        _mouseInput->prevPos_          = _mouseInput->pos_;
        _mouseInput->prevWheelDelta_   = _mouseInput->currentWheelDelta_;
        _mouseInput->prevButtonStates_ = _mouseInput->currentButtonStates_;

        // pad入力
        _padInput->prevButtonMask_ = _padInput->buttonMask_;
    }

    // 最新状態（current）の注入
    /// keyboard
    if (frameData.keyInputData.size() != 0) {
        for (size_t keyIndex = 0; keyIndex < KEY_COUNT; ++keyIndex) {
            bool isPressed             = frameData.keyInputData.Get(keyIndex);
            _keyInput->keys_[keyIndex] = isPressed ? 0x80 : 0x00;
        }
    }

    /// mouse
    _mouseInput->pos_               = frameData.mouseData.mousePos;
    _mouseInput->virtualPos_        = _mouseInput->pos_;
    _mouseInput->currentWheelDelta_ = frameData.mouseData.wheelDelta;

    // 速度（移動量）の計算
    _mouseInput->velocity_ = _mouseInput->pos_ - _mouseInput->prevPos_;

    for (size_t mouseButtonIndex = 0; mouseButtonIndex < MOUSE_BUTTON_COUNT; ++mouseButtonIndex) {
        _mouseInput->currentButtonStates_[mouseButtonIndex] = (frameData.mouseData.buttonData >> mouseButtonIndex) & 1u;
    }

    /// padInput
    _padInput->lStick_ = frameData.padData.lStick;
    _padInput->rStick_ = frameData.padData.rStick;

    _padInput->lTrigger_ = frameData.padData.lTrigger;
    _padInput->rTrigger_ = frameData.padData.rTrigger;

    _padInput->buttonMask_ = frameData.padData.buttonData;
    _padInput->isActive_   = frameData.padData.isActive;

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
