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

/// <summary>
/// リプレイデータ1フレーム分のマウス情報
/// </summary>
struct ReplayFrameMouseData {
    Vec2f mousePos;
    int32_t wheelDelta;

    uint32_t buttonData;
};

/// <summary>
/// リプレイデータ1フレーム分のパッド情報
/// </summary>
struct ReplayFramePadData {
    Vec2f lStick;
    Vec2f rStick;

    float lTrigger;
    float rTrigger;

    uint32_t buttonData;
};

/// <summary>
/// リプレイデータ1フレーム分
/// </summary>
struct ReplayFrameData {
    BitArray<uint32_t> keyInputData = BitArray<uint32_t>(256); // リプレイデータ1フレーム分のキーボード情報 (uint32_t * 8 = 32B)
    ReplayFrameMouseData mouseData;
    ReplayFramePadData padData;

    float deltaTime;
};

/// <summary>
/// リプレイファイルヘッダ(ReplayFileの基本情報)
/// </summary>
struct ReplayFileHeader {
    std::string startScene = "";
    Version version        = {};
    uint32_t frameCount    = 0;
};

/// <summary>
/// リプレイファイルの拡張子
/// </summary>
constexpr const char* kReplayFileExtension = "rpd";

/// <summary>
/// リプレイフォルダ名
/// </summary>
constexpr const char* kReplayFolderName = "replays";

/// <summary>
/// リプレイファイル全体
/// </summary>
struct ReplayFile {
    void Initialize() {
        header = {};
        frameData.clear();
    }
    void Finalize() {
        header = {};
        frameData.clear();
    }

    ReplayFileHeader header;
    std::vector<ReplayFrameData> frameData;
};
