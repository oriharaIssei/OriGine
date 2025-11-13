#pragma once

/// stl
#include <string>
#include <vector>

/// engine
class SceneManager;
// input
class KeyboardInput;
class MouseInput;
class GamePadInput;

/// default data
#include "base/ReplayData.h"

/// <summary>
/// 記録データを読み込み、フレーム単位で入力を再生する
/// </summary>
class ReplayPlayer {
public:
    /// <summary>
    /// 読み込み等、初期化処理
    /// </summary>
    /// <param name="_path"></param>
    void Initialize(const std::string& _path, SceneManager* _sceneManager);
    void Finalize();

    /// <summary>
    /// 指定したファイルから記録データを読み込む
    /// </summary>
    /// <param name="_path">ファイルパス(directory / filename)</param>
    bool LoadFromFile(const std::string& _path);

    /// <summary>
    /// 現在のフレームの入力を適用する
    /// </summary>
    /// <param name="key">キーボード入力</param>
    /// <param name="mouse">マウス入力</param>
    /// <param name="pad">ゲームパッド入力</param>
    /// <returns>フレームの経過時間</returns>
    float Apply(KeyboardInput* key, MouseInput* mouse, GamePadInput* pad);

    /// <summary>
    /// 任意のフレームにジャンプする
    /// </summary>
    bool Seek(size_t frameIndex);

private:
    ReplayFile fileData_      = {};
    std::string filepath_     = "";
    size_t currentFrameIndex_ = 0;

    bool isActive_ = false;

public:
    bool getIsActive() const { return isActive_; }

    const std::string& getFilepath() const { return filepath_; }
    const std::string& getStartSceneName() const { return fileData_.header.startScene; }

    size_t getTotalFrameCount() const { return fileData_.frameData.size(); }
    size_t getCurrentFrameIndex() const { return currentFrameIndex_; }
    void setCurrentFrameIndex(size_t index) { currentFrameIndex_ = index; }

    const ReplayFrameData& getCurrentFrameData() const { return fileData_.frameData[currentFrameIndex_]; }
};
