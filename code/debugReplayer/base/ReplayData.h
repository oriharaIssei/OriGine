#pragma once

/// windows
#include <Windows.h>

/// stl
#include <array>
#include <string>

/// engine
// input
#include "input/GamepadInput.h"
#include "input/KeyboardInput.h"
#include "input/MouseInput.h"

/// util
#include <BitArray.h>
#include <Version.h>

/// math
#include "math/Vector2.h"
#include <cstdint>

namespace OriGine {

/// <summary>
/// リプレイデータ 1 フレーム分のパッド入力情報.
/// </summary>
struct ReplayFramePadData {
    GamepadState padState; // パッドの状態
    bool isActive; // パッドが接続されているかどうか
};

/// <summary>
/// リプレイデータ 1 フレーム分の全入力情報.
/// キーボード、マウス、パッドの状態と、そのフレームの経過時間（deltaTime）を保持する.
/// </summary>
struct ReplayFrameData {
    BitArray<uint32_t> keyInputData = BitArray<uint32_t>(256); // キーボードの全キー状態（256ビット）
    MouseState mouseData; // マウス情報
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
