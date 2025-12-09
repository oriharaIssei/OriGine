#pragma once

/// stl
#include <string>
#include <vector>

/// engine
class SceneManager;
// input
class KeyboardInput;
class MouseInput;
class GamepadInput;

/// default data
#include "base/ReplayData.h"

/// external
#include "logger/Logger.h"

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
    float Apply(KeyboardInput* key, MouseInput* mouse, GamepadInput* pad);

    /// <summary>
    /// 任意のフレームにジャンプする
    /// </summary>
    bool Seek(size_t frameIndex);

    /// <summary>
    /// リプレイ終了したか
    /// CurrentIndexが総フレーム数以上なら終了
    /// </summary>
    /// <returns></returns>
    bool IsEnd() const { return currentFrameIndex_ >= fileData_.frameData.size(); }

    /// <summary>
    /// 指定したフレームインデックスが有効か
    /// </summary>
    /// <param name="frameIndex">フレームのインデックス</param>
    /// <returns>true = 有効、false = 失敗</returns>
    bool IsValidFrame(int32_t frameIndex) const {
        return (frameIndex >= 0 && frameIndex < static_cast<int32_t>(fileData_.frameData.size()));
    }

private:
    ReplayFile fileData_      = {};
    std::string filepath_     = "";
    size_t currentFrameIndex_ = 0;

    bool isActive_ = false;

public:
    bool GetIsActive() const { return isActive_; }

    const std::string& GetFilepath() const { return filepath_; }
    const std::string& GetStartSceneName() const { return fileData_.header.startScene; }

    size_t GetTotalFrameCount() const { return fileData_.frameData.size(); }
    size_t GetCurrentFrameIndex() const { return currentFrameIndex_; }
    void SetCurrentFrameIndex(size_t index) { currentFrameIndex_ = index; }

    const ReplayFrameData& GetCurrentFrameData() const { return fileData_.frameData[currentFrameIndex_]; }
    const ReplayFrameData& GetFrameData(int32_t _frameIndex) const {
        if (_frameIndex < 0 || _frameIndex > fileData_.frameData.size()) {
            LOG_ERROR("Frame index out of range: {}", _frameIndex);

            ReplayFrameData tmpData;
            return tmpData;
        }
        return fileData_.frameData[_frameIndex];
    }
};
