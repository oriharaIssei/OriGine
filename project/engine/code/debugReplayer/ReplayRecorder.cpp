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

using namespace OriGine;

ReplayRecorder::ReplayRecorder() {}
ReplayRecorder::~ReplayRecorder() {}

/// <summary> 記録の初期化. 過去のフレームデータを消去し、ヘッダに開始シーンを設定する. </summary>
void ReplayRecorder::Initialize(const std::string& _startSceneName) {
    header_.startScene = _startSceneName;
    frames_.clear();
}

/// <summary> 各種入力デバイスから現在の状態を取得し、リプレイ用フレームデータとして保存する. </summary>
void ReplayRecorder::RecordFrame(float _deltaTime, KeyboardInput* _keyInput, MouseInput* _mouseInput, GamepadInput* _padInput) {
    ReplayFrameData frameData = {};
    frameData.deltaTime       = _deltaTime;

    // キーボード入力の記録（全 256 キーの状態を BitArray に格納）
    if (_keyInput) {
        int32_t keyIndex = 0;
        for (auto keyData : _keyInput->GetKeyStates()) {
            frameData.keyInputData.Set(keyIndex, keyData);
            ++keyIndex;
        }
    }

    // マウス入力の記録（座標、ホイール、ボタン）
    if (_mouseInput) {
        frameData.mouseData.mousePos   = _mouseInput->GetPosition();
        frameData.mouseData.wheelDelta = _mouseInput->GetWheelDelta();

        frameData.mouseData.buttonData = _mouseInput->ButtonStateToBitmask();
    }

    // ゲームパッド入力の記録（スティック、トリガー、ボタン、接続状態）
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

/// <summary>
/// 蓄積したデータをバイナリファイルとして書き出す.
/// </summary>
bool ReplayRecorder::SaveToFile(const std::string& _directory, const std::string& _filename) {
    // 保存先フォルダの存在を保証
    myfs::CreateFolder(_directory);

    // .rpd ファイルパスの組み立て
    std::string path = _directory + "/" + _filename + '.' + kReplayFileExtension;

    std::ofstream ofs;
    ofs.open(path, std::ios::binary);
    // ファイルオープン失敗時のエラーハンドリング
    if (!ofs.is_open()) {
        MessageBoxA(nullptr, ("Failed to open file: " + path).c_str(), "Error", MB_OK);
        LOG_CRITICAL("Failed to open replay file for writing: {}", path);
        return false;
    }

    // ヘッダー情報の書き込み
    WriteHeader(ofs);

    // 各フレームデータの書き込み
    for (size_t i = 0; i < header_.frameCount; ++i) {
        WriteFrameData(ofs, i);
    }

    ofs.close();

    LOG_INFO("Replay file saved: {}", path);
    return true;
}

/// <summary> ヘッダ情報（シーン名、バージョン、フレーム数）をバイナリ出力する. </summary>
void ReplayRecorder::WriteHeader(std::ofstream& _ofs) {
    // 1. 開始シーン名（文字列長 + 文字列本体）
    {
        size_t length = header_.startScene.size();
        _ofs.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
        _ofs.write(header_.startScene.c_str(), length);
    }
    // 2. バージョン情報（Major, Minor, Patch）
    {
        _ofs.write(reinterpret_cast<const char*>(&header_.version.major), sizeof(uint32_t));
        _ofs.write(reinterpret_cast<const char*>(&header_.version.minor), sizeof(uint32_t));
        _ofs.write(reinterpret_cast<const char*>(&header_.version.patch), sizeof(uint32_t));
    }
    // 3. 総フレーム数
    {
        header_.frameCount = static_cast<uint32_t>(frames_.size());
        _ofs.write(reinterpret_cast<const char*>(&header_.frameCount), sizeof(uint32_t));
    }
}

/// <summary> 指定したインデックスの 1 フレーム分の全入力をバイナリ出力する. </summary>
void ReplayRecorder::WriteFrameData(std::ofstream& _ofs, size_t _frameIndex) {
    const auto& frame = frames_[_frameIndex];

    // フレームの経過時間
    _ofs.write(reinterpret_cast<const char*>(&frame.deltaTime), sizeof(float));

    // キーボード入力データ（BitArray のシリアライズ）
    {
        // 1. BitArray のビット数を書き出す
        size_t size = frame.keyInputData.size();
        _ofs.write(reinterpret_cast<const char*>(&size), sizeof(size));

        // 2. 内部保持している raw データのブロック数（uint32_t の数）を書き出す
        const auto& data  = frame.keyInputData.GetData();
        size_t blockCount = data.size();
        _ofs.write(reinterpret_cast<const char*>(&blockCount), sizeof(blockCount));

        // 3. 実際の 32bit ブロック配列をバイナリ書き出し
        _ofs.write(reinterpret_cast<const char*>(data.data()),
            static_cast<std::streamsize>(blockCount * sizeof(uint32_t)));
    }
    // マウス入力データ
    {
        // マウス座標 (Vector2f -> 2 floats)
        _ofs.write(reinterpret_cast<const char*>(&frame.mouseData.mousePos[X]), sizeof(float));
        _ofs.write(reinterpret_cast<const char*>(&frame.mouseData.mousePos[Y]), sizeof(float));
        // ホイールの変化量
        _ofs.write(reinterpret_cast<const char*>(&frame.mouseData.wheelDelta), sizeof(int32_t));

        // ボタンのビットマスク
        _ofs.write(reinterpret_cast<const char*>(&frame.mouseData.buttonData), sizeof(uint32_t));
    }
    // ゲームパッド入力データ
    {
        // スティック入力 (L/R)
        _ofs.write(reinterpret_cast<const char*>(&frame.padData.lStick[X]), sizeof(float));
        _ofs.write(reinterpret_cast<const char*>(&frame.padData.lStick[Y]), sizeof(float));
        _ofs.write(reinterpret_cast<const char*>(&frame.padData.rStick[X]), sizeof(float));
        _ofs.write(reinterpret_cast<const char*>(&frame.padData.rStick[Y]), sizeof(float));
        // トリガー入力 (L/R)
        _ofs.write(reinterpret_cast<const char*>(&frame.padData.lTrigger), sizeof(float));
        _ofs.write(reinterpret_cast<const char*>(&frame.padData.rTrigger), sizeof(float));
        // ボタンのビットマスク
        _ofs.write(reinterpret_cast<const char*>(&frame.padData.buttonData), sizeof(uint32_t));
        // パッド接続状態
        _ofs.write(reinterpret_cast<const char*>(&frame.padData.isActive), sizeof(bool));
    }
}
