#include "ReplayPlayer.h"

/// stl
#include <filesystem>
#include <fstream>

/// engine
// input
#include "input/GamePadInput.h"
#include "input/KeyboardInput.h"
#include "input/MouseInput.h"

bool ReplayPlayer::LoadFromFile(const std::string& filepath) {
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs.is_open()) {
        MessageBoxA(nullptr, ("Failed to open replay file: " + filepath).c_str(), "Error", MB_OK);
        LOG_CRITICAL("Failed to open replay file for reading: {}", filepath);
        return false;
    }

    filepath_ = filepath;
    // 読み込み準備
    frames_.clear();
    currentFrameIndex_ = 0;

    // ===== ヘッダーの読み込み =====
    {
        // 開始シーン名
        size_t length = 0;
        ifs.read(reinterpret_cast<char*>(&length), sizeof(size_t));
        header_.startScene.resize(length);
        ifs.read(header_.startScene.data(), length);

        // バージョン情報
        ifs.read(reinterpret_cast<char*>(&header_.version.major), sizeof(uint32_t));
        ifs.read(reinterpret_cast<char*>(&header_.version.minor), sizeof(uint32_t));
        ifs.read(reinterpret_cast<char*>(&header_.version.patch), sizeof(uint32_t));

        // フレーム数
        uint32_t frameCount = 0;
        ifs.read(reinterpret_cast<char*>(&frameCount), sizeof(uint32_t));
        frames_.resize(frameCount);
    }

    // ===== フレームデータの読み込み =====
    for (auto& frame : frames_) {
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
                frame.keyInputData.setBlock(i, blocks[i]);
            }
        }

        // マウス入力
        {
            ifs.read(reinterpret_cast<char*>(&frame.mouseData.mousePos[X]), sizeof(float));
            ifs.read(reinterpret_cast<char*>(&frame.mouseData.mousePos[Y]), sizeof(float));
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
    auto& frameData = frames_[currentFrameIndex_];

    // keyboard
    for (size_t keyIndex = 0; keyIndex < KEY_COUNT; ++keyIndex) {
        bool isPressed             = frameData.keyInputData.get(keyIndex);
        _keyInput->keys_[keyIndex] = isPressed ? 0x80 : 0x00;
    }

    // mouse
    _mouseInput->pos_      = frameData.mouseData.mousePos;
    _mouseInput->velocity_ = _mouseInput->pos_ - _mouseInput->prevPos_;

    for (size_t mouseButtonIndex = 0; mouseButtonIndex < MOUSE_BUTTON_COUNT; ++mouseButtonIndex) {
        _mouseInput->currentButtonStates_[mouseButtonIndex] = (frameData.mouseData.buttonData >> mouseButtonIndex) & 1u;
    }

    // padInput
    _padInput->lStick_ = frameData.padData.lStick;
    _padInput->rStick_ = frameData.padData.rStick;

    _padInput->lTrigger_ = frameData.padData.lTrigger;
    _padInput->rTrigger_ = frameData.padData.rTrigger;

    _padInput->buttonMask_ = frameData.padData.buttonData;

    return frameData.deltaTime;
}

void ReplayPlayer::StepFrame() {
    if (isReverse_) {
        if (currentFrameIndex_ > 0) {
            --currentFrameIndex_;
        }
    } else {
        if (currentFrameIndex_ + 1 < frames_.size()) {
            ++currentFrameIndex_;
        }
    }
}

void ReplayPlayer::Seek(size_t frameIndex) {
    if (frameIndex < frames_.size()) {
        currentFrameIndex_ = frameIndex;
    }
}
