#include "ReplayRecorder.h"

/// engine
// input
#include "input/GamepadInput.h"
#include "input/KeyboardInput.h"
#include "input/MouseInput.h"
// logger
#include "logger/Logger.h"

/// util
#include "binaryIO/BinaryIO.h"
#include "myFileSystem/MyFileSystem.h"

ReplayRecorder::ReplayRecorder() {}
ReplayRecorder::~ReplayRecorder() {}

void ReplayRecorder::Initialize(const std::string& _startSceneName) {
    header_.startScene = _startSceneName;
    frames_.clear();
}

void ReplayRecorder::RecordFrame(float deltaTime, KeyboardInput* _keyInput, MouseInput* _mouseInput, GamepadInput* _padInput) {
    ReplayFrameData frameData = {};
    frameData.deltaTime       = deltaTime;

    // キーボード入力の記録
    if (_keyInput) {
        int32_t keyIndex = 0;
        for (auto keyData : _keyInput->GetKeyStates()) {
            frameData.keyInputData.Set(keyIndex, keyData);
            ++keyIndex;
        }
    }

    // マウス入力の記録
    if (_mouseInput) {
        frameData.mouseData.mousePos   = _mouseInput->GetPosition();
        frameData.mouseData.wheelDelta = _mouseInput->GetWheelDelta();

        frameData.mouseData.buttonData = _mouseInput->ButtonStateToBitmask();
    }

    // ゲームパッド入力の記録
    if (_padInput) {
        frameData.padData.lStick     = _padInput->GetLStick();
        frameData.padData.rStick     = _padInput->GetRStick();
        frameData.padData.lTrigger   = _padInput->GetLTrigger();
        frameData.padData.rTrigger   = _padInput->GetRTrigger();
        frameData.padData.buttonData = _padInput->GetButtonMask();
        frameData.padData.isActive   = _padInput->IsActive();
    }

    frames_.push_back(frameData);
    header_.frameCount = static_cast<uint32_t>(frames_.size());
}

bool ReplayRecorder::SaveToFile(const std::string& _directory, const std::string& _filename) {
    // _directory を 念のため作成しておく
    myfs::CreateFolder(_directory);

    // ファイルを開く
    // rpd = Replay debug
    std::string path = _directory + "/" + _filename + '.' + kReplayFileExtension;

    std::ofstream ofs;
    ofs.open(path, std::ios::binary);
    // 失敗したらエラー表示
    if (!ofs.is_open()) {
        MessageBoxA(nullptr, ("Failed to open file: " + path).c_str(), "Error", MB_OK);
        LOG_CRITICAL("Failed to open replay file for writing: {}", path);
        return false;
    }

    // ヘッダー書き込み
    WriteHeader(ofs);

    // フレームデータ書き込み
    for (size_t i = 0; i < header_.frameCount; ++i) {
        WriteFrameData(ofs, i);
    }

    // ファイルを閉じる
    ofs.close();

    LOG_INFO("Replay file saved: {}", path);
    return true;
}

void ReplayRecorder::WriteHeader(std::ofstream& _ofs) {
    // 開始シーン名
    {
        size_t length = header_.startScene.size();
        _ofs.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
        _ofs.write(header_.startScene.c_str(), length);
    }
    // バージョン情報
    {
        _ofs.write(reinterpret_cast<const char*>(&header_.version.major), sizeof(uint32_t));
        _ofs.write(reinterpret_cast<const char*>(&header_.version.minor), sizeof(uint32_t));
        _ofs.write(reinterpret_cast<const char*>(&header_.version.patch), sizeof(uint32_t));
    }
    // フレーム数
    {
        header_.frameCount = static_cast<uint32_t>(frames_.size());
        _ofs.write(reinterpret_cast<const char*>(&header_.frameCount), sizeof(uint32_t));
    }
}

void ReplayRecorder::WriteFrameData(std::ofstream& _ofs, size_t _frameIndex) {
    const auto& frame = frames_[_frameIndex];

    // デルタタイム
    _ofs.write(reinterpret_cast<const char*>(&frame.deltaTime), sizeof(float));

    // キーボード入力データ
    {
        // 1. サイズを書き出す
        size_t size = frame.keyInputData.size();
        _ofs.write(reinterpret_cast<const char*>(&size), sizeof(size));

        // 2. データの要素数を書き出す（復元に必要）
        const auto& data  = frame.keyInputData.GetData();
        size_t blockCount = data.size();
        _ofs.write(reinterpret_cast<const char*>(&blockCount), sizeof(blockCount));

        // 3. 実際のデータを書き出す
        _ofs.write(reinterpret_cast<const char*>(data.data()),
            static_cast<std::streamsize>(blockCount * sizeof(uint32_t)));
    }
    // マウス入力データ
    {
        // マウス位置
        _ofs.write(reinterpret_cast<const char*>(&frame.mouseData.mousePos[X]), sizeof(float));
        _ofs.write(reinterpret_cast<const char*>(&frame.mouseData.mousePos[Y]), sizeof(float));
        // ホイールデルタ
        _ofs.write(reinterpret_cast<const char*>(&frame.mouseData.wheelDelta), sizeof(int32_t));

        // ボタンデータ
        _ofs.write(reinterpret_cast<const char*>(&frame.mouseData.buttonData), sizeof(uint32_t));
    }
    // ゲームパッド入力データ
    {
        // 左スティック
        _ofs.write(reinterpret_cast<const char*>(&frame.padData.lStick[X]), sizeof(float));
        _ofs.write(reinterpret_cast<const char*>(&frame.padData.lStick[Y]), sizeof(float));
        // 右スティック
        _ofs.write(reinterpret_cast<const char*>(&frame.padData.rStick[X]), sizeof(float));
        _ofs.write(reinterpret_cast<const char*>(&frame.padData.rStick[Y]), sizeof(float));
        // トリガー
        _ofs.write(reinterpret_cast<const char*>(&frame.padData.lTrigger), sizeof(float));
        _ofs.write(reinterpret_cast<const char*>(&frame.padData.rTrigger), sizeof(float));
        // ボタンデータ
        _ofs.write(reinterpret_cast<const char*>(&frame.padData.buttonData), sizeof(uint32_t));
        // アクティブ状態
        _ofs.write(reinterpret_cast<const char*>(&frame.padData.isActive), sizeof(bool));
    }
}
