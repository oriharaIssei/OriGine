#pragma once

/// stl
#include <array>
#include <cstdint>
#include <string>
#include <vector>

/// engine
#include "EngineConfig.h"
#include "ProfileEventStream.h"
#include "ProfileTree.h"

namespace OriGine {

/// <summary>
/// フレームタイムの統計値
/// </summary>
struct FrameTimeStats {
    float averageMs_ = 0.0f; // 直近履歴の平均
    float p99Ms_      = 0.0f; // 直近履歴の99パーセンタイル(スパイクの目安)
    float maxMs_      = 0.0f; // 直近履歴の最大値
};

/// <summary>
/// 階層プロファイラの統括クラス (シングルトン).
/// フレーム境界の管理、スレッドごとの計測ストリームの収集、
/// フレームタイム/アロケーション履歴の保持を行う.
/// 実際の計測点は PROFILE_SCOPE マクロ (Profiling::ScopedEvent) が担当する.
///
/// 使い方:
///   void SomeSystem::Update() {
///       PROFILE_SCOPE("SomeSystem::Update");
///       ...
///   }
///   // 毎フレーム1回、エンジンのフレーム開始時に呼ぶ
///   OriGine::Profiler::GetInstance()->BeginFrame();
/// </summary>
class Profiler {
public:
    /// <summary> シングルトンインスタンスを取得する </summary>
    static Profiler* GetInstance();

    /// <summary>
    /// フレーム境界処理. Engine::BeginFrame() から1フレームに1回呼び出すこと.
    /// 各スレッドの計測バッファのスワップ、フレームタイム履歴の更新、
    /// アロケーションカウンタのフレーム集計を行う.
    /// Releaseビルドでは実質何もしない(呼び出し自体は安全).
    /// </summary>
    void BeginFrame();

    /// <summary>
    /// 現在の実行スレッドの表示名を設定する(未設定時は"Thread-<ID>"になる)
    /// </summary>
    /// <param name="_name">スレッドの表示名</param>
    void SetCurrentThreadName(const std::string& _name);

    /// <summary>
    /// 直前フレームの階層プロファイル木を、登録されている全スレッド分構築して返す.
    /// Editor描画時にのみ呼び出すことを想定している(このメソッド自体は動的確保を伴う).
    /// </summary>
    /// <returns>スレッドごとの階層プロファイル木</returns>
    std::vector<Profiling::ProfileThreadTree> BuildLastFrameTrees() const;

    /// <summary>
    /// フレームタイムの統計値(平均/99パーセンタイル/最大)を直近履歴から計算する
    /// </summary>
    /// <returns>フレームタイム統計</returns>
    FrameTimeStats GetFrameTimeStats() const;

    /// <summary>
    /// フレームタイム履歴(ミリ秒, リングバッファ)を取得する
    /// </summary>
    const std::array<float, OriGine::Config::Profiler::kFrameHistorySize>& GetFrameTimeHistory() const { return frameTimeHistory_; }

    /// <summary>
    /// フレームタイム履歴における最新値の格納位置(ImGui::PlotLinesのvalues_offsetに使用)
    /// </summary>
    size_t GetFrameTimeHistoryCursor() const { return historyCursor_; }

private:
    Profiler()  = default;
    ~Profiler() = default;
    Profiler(const Profiler&)            = delete;
    Profiler& operator=(const Profiler&) = delete;

private:
    std::array<float, OriGine::Config::Profiler::kFrameHistorySize> frameTimeHistory_{};
    size_t historyCursor_    = 0;
    int64_t lastFrameTicks_  = 0;
    bool hasLastFrameTicks_  = false;
};

} // namespace OriGine

// ============================================================================
// PROFILE_SCOPE マクロ
// リリースビルド(_RELEASE定義時)では完全に消える(空展開)。
// ============================================================================
#if defined(_RELEASE)
#define PROFILE_SCOPE(_name) ((void)0)
#else
#define OriGine_PROFILE_CONCAT_INNER(a, b) a##b
#define OriGine_PROFILE_CONCAT(a, b) OriGine_PROFILE_CONCAT_INNER(a, b)
/// <summary>
/// 現在のスコープの計測を開始する(スコープを抜けると自動的に終了イベントが記録される).
/// 例: PROFILE_SCOPE("CollisionCheckSystem::Update");
/// </summary>
#define PROFILE_SCOPE(_name) \
    ::OriGine::Profiling::ScopedEvent OriGine_PROFILE_CONCAT(origine_profileScope_, __LINE__)(_name)
#endif
