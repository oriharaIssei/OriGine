#pragma once

/// stl
#include <chrono>
#include <unordered_map>

/// <summary>
/// DeltaTimerの計測をするクラス
/// </summary>
class DeltaTimer {
public:
    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize();
    /// <summary>
    /// 更新
    /// </summary>
    void Update();

private:
    float deltaTime_;
    std::unordered_map<std::string, float> deltaTimeScaleMap_;
    std::chrono::high_resolution_clock::time_point currentTime_;
    std::chrono::high_resolution_clock::time_point preTime_;

public:
    /// <summary>
    /// 前フレームからの経過時間を取得(秒)
    /// </summary>
    /// <returns>デルタタイム</returns>
    float GetDeltaTime() const { return deltaTime_; }
    /// <summary>
    /// タイムスケールを適用した経過時間を取得
    /// </summary>
    /// <param name="key">タイムスケールのキー</param>
    /// <returns>スケール後のデルタタイム</returns>
    float GetScaledDeltaTime(const std::string& key) const;
    /// <summary>
    /// デルタタイムを直接設定
    /// </summary>
    /// <param name="dt">デルタタイム</param>
    void SetDeltaTimer(float dt) { deltaTime_ = dt; }
    /// <summary>
    /// タイムスケールを設定
    /// </summary>
    /// <param name="key">キー</param>
    /// <param name="scale">スケール値</param>
    void SetTimeScale(const std::string& key, float scale);
};
