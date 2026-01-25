#pragma once

/// stl
#include <chrono>

#include <deque>
#include <unordered_map>

/// <summary>
/// DeltaTimerの計測をするクラス
/// </summary>
class DeltaTimer {
    static constexpr size_t kMaxHistorySize = 60; // 直近60フレームの平均を取る
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

    // --- 平均計測用に追加 ---
    std::deque<float> frameHistory_; // 履歴
    float totalHistoryTime_ = 0.0f;

public:
    /// <summary>
    /// 前フレームからの経過時間を取得(秒)
    /// </summary>
    /// <returns>デルタタイム</returns>
    float GetDeltaTime() const { return deltaTime_; }

    /// <summary>
    /// 履歴全体の平均DeltaTimeを取得
    /// </summary>
    float GetAverageDeltaTime() const;

    /// <summary>
    /// 直近 指定フレーム数 の平均DeltaTimeを取得
    /// </summary>
    float GetAverageDeltaTime(size_t _frameCount) const;

    /// <summary>
    /// 履歴全体の平均FPSを取得
    /// </summary>
    /// <returns></returns>
    size_t GetAverageFPS() const;
    /// <summary>
    /// 直近 指定フレーム数 の平均FPSを取得
    /// </summary>
    /// <param name="_frameCount"></param>
    /// <returns></returns>
    size_t GetAverageFPS(size_t _frameCount) const;

    /// <summary>
    /// タイムスケールを適用した経過時間を取得
    /// </summary>
    /// <param name="key">タイムスケールのキー</param>
    /// <returns>スケール後のデルタタイム</returns>
    float GetScaledDeltaTime(const std::string& _key) const;

    /// <summary>
    /// デルタタイムを直接設定
    /// </summary>
    /// <param name="dt">デルタタイム</param>
    void SetDeltaTimer(float _dt) { deltaTime_ = _dt; }
    /// <summary>
    /// タイムスケールを設定
    /// </summary>
    /// <param name="key">キー</param>
    /// <param name="scale">スケール値</param>
    void SetTimeScale(const std::string& _key, float _scale);
};
