#include "ReplayPlayer.h"

/// stl
#include <filesystem>
#include <fstream>

/// engine
#include "scene/SceneManager.h"
// log
#include "logger/Logger.h"

// input
#include "input/GamePadInput.h"
#include "input/KeyboardInput.h"
#include "input/MouseInput.h"

namespace OriGine {

void ReplayPlayer::Initialize(const std::string& filepath, SceneManager* _sceneManager) {
    // すでに初期化されている場合は何もしない
    if (isActive_) {
        LOG_WARN("already initialized.");
        return;
    }

    fileData_.Initialize();
    isActive_ = LoadFromFile(filepath);

    // シーンマネージャーに開始シーンをセット
    _sceneManager->ChangeScene(fileData_.header.startScene);

    if (isActive_) {
        Apply(_sceneManager->keyInput_, _sceneManager->mouseInput_, _sceneManager->padInput_);
    }
}

void ReplayPlayer::Finalize() {
    // 未初期化の場合は何もしない
    if (!isActive_) {
        LOG_WARN("not initialized.");
        return;
    }

    isActive_ = false;
    fileData_.Finalize();
    currentFrameIndex_ = 0;
    filepath_.clear();
}

bool ReplayPlayer::LoadFromFile(const std::string& filepath) {
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs.is_open()) {
        MessageBoxA(nullptr, ("Failed to open replay file: " + filepath).c_str(), "Error", MB_OK);
        LOG_CRITICAL("Failed to open replay file for reading: {}", filepath);
        return false;
    }

    filepath_ = filepath;
    // 読み込み準備
    fileData_.Initialize();

    // ===== ヘッダーの読み込み =====
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

        // フレーム数
        uint32_t frameCount = 0;
        ifs.read(reinterpret_cast<char*>(&frameCount), sizeof(uint32_t));
        fileData_.frameData.resize(frameCount);
    }

    // ===== フレームデータの読み込み =====
    for (auto& frame : fileData_.frameData) {
        // deltaTime
        ifs.read(reinterpret_cast<char*>(&frame.deltaTime), sizeof(float));

        // キーボード入力
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
        }
    }

    ifs.close();

    LOG_INFO("Replay file loaded successfully: {}", filepath);
    return true;
}

float ReplayPlayer::Apply(KeyboardInput* _keyInput, MouseInput* _mouseInput, GamePadInput* _padInput) {
    auto& frameData = fileData_.frameData[currentFrameIndex_];

    /// prev の更新
    if (currentFrameIndex_ == 0) {
        /// 初期化
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
        // key入力
        _keyInput->prevKeys_ = _keyInput->keys_;

        // mouse入力
        _mouseInput->prevPos_          = _mouseInput->pos_;
        _mouseInput->prevWheelDelta_   = _mouseInput->currentWheelDelta_;
        _mouseInput->prevButtonStates_ = _mouseInput->currentButtonStates_;

        // pad入力
        _padInput->prevButtonMask_ = _padInput->buttonMask_;
    }

    /// keyboard
    // current を更新
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

    return frameData.deltaTime;
}

bool ReplayPlayer::Seek(size_t frameIndex) {
    if (frameIndex < fileData_.frameData.size()) {
        bool result        = (currentFrameIndex_ != frameIndex);
        currentFrameIndex_ = frameIndex;
        return result;
    }
    return false;
}

} // namespace OriGine
