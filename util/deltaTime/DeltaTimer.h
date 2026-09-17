#pragma once

/// stl
#include <chrono>

#include <deque>
#include <unordered_map>

/// engine
#include "EngineConfig.h"

/// <summary>
/// DeltaTimerの計測をするクラス
/// </summary>
class DeltaTimer {
    static constexpr size_t kMaxHistorySize = OriGine::Config::Time::kFpsHistorySize; // 直近の平均を取る
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

    // ベンチマーク用の固定デルタタイムモード。有効な間はUpdate()が実クロックを
    // 読まず、deltaTime_ を常に同じ値にする。GetScaledDeltaTime()もdeltaTime_を
    // 元に計算しているため、ここを固定するだけで両方の取得経路が固定値になる
    bool fixedDeltaTimeEnabled_   = false;
    float fixedDeltaTimeSeconds_ = 0.0f;

    // --- 平均計測用に追加 ---
    std::deque<float> frameHistory_; // 履歴
    float totalHistoryTime_ = 0.0f;

public:
    const std::unordered_map<std::string, float>& GetDeltaTimeScaleMap() const { return deltaTimeScaleMap_; }

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
    float GetAverageDeltaTime(size_t frameCount) const;

    /// <summary>
    /// 履歴全体の平均FPSを取得
    /// </summary>
    /// <returns></returns>
    size_t GetAverageFPS() const;
    /// <summary>
    /// 直近 指定フレーム数 の平均FPSを取得
    /// </summary>
    /// <param name="frameCount"></param>
    /// <returns></returns>
    size_t GetAverageFPS(size_t frameCount) const;

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
    void SetDeltaTime(float dt) { deltaTime_ = dt; }
    /// <summary>
    /// タイムスケールを設定
    /// </summary>
    /// <param name="key">キー</param>
    /// <param name="scale">スケール値</param>
    void SetTimeScale(const std::string& key, float scale);

    /// <summary>
    /// 固定デルタタイムモードを有効化する(ベンチマーク専用).
    /// </summary>
    /// <remarks>
    /// マシンの実行速度やOSのスケジューリングジッターでフレーム時間が変わると、
    /// それに比例して移動量や衝突判定の回数まで回ごとに変わってしまい、
    /// 「同じ負荷を計測している」という前提が崩れる。有効中は Update() が
    /// 実クロックを読まずこの値をそのまま使うため、通常実行(ゲーム・エディタ)では
    /// 呼ばないこと。
    /// </remarks>
    /// <param name="_fixedSeconds">固定するデルタタイム[秒]</param>
    void EnableFixedDeltaTime(float _fixedSeconds);

    /// <summary>
    /// 固定デルタタイムモードを解除し、実時間計測に戻す.
    /// </summary>
    void DisableFixedDeltaTime();

    /// <summary>
    /// 固定デルタタイムモードが有効かどうか.
    /// </summary>
    bool IsFixedDeltaTimeEnabled() const { return fixedDeltaTimeEnabled_; }
};
