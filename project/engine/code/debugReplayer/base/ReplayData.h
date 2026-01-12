#pragma once

/// windows
#include <Windows.h>

/// stl
#include <array>
#include <string>

/// util
#include <BitArray.h>
#include <Version.h>

/// math
#include "math/Vector2.h"
#include <cstdint>

namespace OriGine {

/// <summary>
/// リプレイデータ 1 フレーム分のマウス入力情報.
/// </summary>
struct ReplayFrameMouseData {
    Vec2f mousePos; // マウスのスクリーン座標
    int32_t wheelDelta; // ホイールの回転量

    uint32_t buttonData; // マウスボタンのビットマスク状態
};

/// <summary>
/// リプレイデータ 1 フレーム分のゲームパッド入力情報.
/// </summary>
struct ReplayFramePadData {
    Vec2f lStick; // 左スティックの傾き
    Vec2f rStick; // 右スティックの傾き

    float lTrigger; // 左トリガーの押し込み量
    float rTrigger; // 右トリガーの押し込み量

    uint32_t buttonData; // パッドボタンのビットマスク状態
    bool isActive; // パッドが接続されてアクティブだったか
};

/// <summary>
/// リプレイデータ 1 フレーム分の全入力情報.
/// キーボード、マウス、パッドの状態と、そのフレームの経過時間（deltaTime）を保持する.
/// </summary>
struct ReplayFrameData {
    BitArray<uint32_t> keyInputData = BitArray<uint32_t>(256); // キーボードの全キー状態（256ビット）
    ReplayFrameMouseData mouseData; // マウス情報
    ReplayFramePadData padData; // パッド情報

    float deltaTime; // フレーム間の経過時間
};

/// <summary>
/// リプレイファイルのヘッダ情報.
/// リプレイ開始時のシーン、エンジンのバージョン、総フレーム数などを管理する.
/// </summary>
struct ReplayFileHeader {
    std::string startScene = ""; // リプレイ開始時のシーン名
    Version version        = {}; // エンジン/アプリのバージョン
    uint32_t frameCount    = 0; // 記録された総フレーム数
};

/// <summary> リプレイファイルの拡張子（"rpd" = Replay Data）. </summary>
constexpr const char* kReplayFileExtension = "rpd";

/// <summary> リプレイデータが保存されるデフォルトのフォルダ名. </summary>
constexpr const char* kReplayFolderName = "replays";

/// <summary>
/// リプレイファイル全体のデータを保持する構造体.
/// </summary>
struct ReplayFile {
    /// <summary> ファイルデータの初期化. </summary>
    void Initialize() {
        header = {};
        frameData.clear();
    }
    /// <summary> ファイルデータの解放処理. </summary>
    void Finalize() {
        header = {};
        frameData.clear();
    }

    ReplayFileHeader header; // ヘッダ情報
    std::vector<ReplayFrameData> frameData; // 各フレームの入力データリスト
};

} // namespace OriGine
