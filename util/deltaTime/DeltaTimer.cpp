#include "DeltaTimer.h"

#include <logger/Logger.h>

void DeltaTimer::Initialize() {
    currentTime_ = std::chrono::high_resolution_clock::now();
    preTime_     = currentTime_;

    deltaTime_        = 0.0f;
    totalHistoryTime_ = 0.0f;
    frameHistory_.clear();
}

void DeltaTimer::Update() {
    if (fixedDeltaTimeEnabled_) {
        // ベンチマーク中は実クロックを読まず固定値を使う(EnableFixedDeltaTime参照)。
        // currentTime_/preTime_ は更新しない。DisableFixedDeltaTime() が
        // 実時間計測に戻す際にまとめて基準を合わせ直すため、ここで半端に触ると
        // 解除直後の1フレームが巨大なデルタタイムになってしまう
        deltaTime_ = fixedDeltaTimeSeconds_;
    } else {
        preTime_     = currentTime_;
        currentTime_ = std::chrono::high_resolution_clock::now();
        deltaTime_   = static_cast<float>(std::chrono::duration<float>(currentTime_ - preTime_).count());
    }

    // --- 平均計測用に追加 ---
    // 直近kMaxHistorySize frame分だけを保持するリングバッファ。
    // 合計値も同時に更新しておくことで、平均を求めるたびに履歴を全走査せずに済む
    // (押し出す要素の分を引き、追加する分を足すだけで合計が保たれる)
    frameHistory_.push_back(deltaTime_);
    totalHistoryTime_ += deltaTime_;
    if (frameHistory_.size() > kMaxHistorySize) {
        totalHistoryTime_ -= frameHistory_.front();
        frameHistory_.pop_front();
    }
}

float DeltaTimer::GetAverageDeltaTime() const {
    if (frameHistory_.empty()) {
        return 0.0f;
    }
    return totalHistoryTime_ / static_cast<float>(frameHistory_.size());
}

float DeltaTimer::GetAverageDeltaTime(size_t frameCount) const {
    if (frameHistory_.empty() || frameCount == 0) {
        return 0.0f;
    }
    size_t count = (std::min)(frameCount, frameHistory_.size());
    float total  = 0.0f;
    for (size_t i = frameHistory_.size() - count; i < frameHistory_.size(); ++i) {
        total += frameHistory_[i];
    }
    return total / static_cast<float>(count);
}

size_t DeltaTimer::GetAverageFPS() const {
    if (frameHistory_.empty()) {
        return 0;
    }
    return static_cast<size_t>(frameHistory_.size() / totalHistoryTime_);
}

size_t DeltaTimer::GetAverageFPS(size_t frameCount) const {
    if (frameHistory_.empty() || frameCount == 0) {
        return 0;
    }
    size_t count = (std::min)(frameCount, frameHistory_.size());
    float total  = 0.0f;
    for (size_t i = frameHistory_.size() - count; i < frameHistory_.size(); ++i) {
        total += frameHistory_[i];
    }
    return static_cast<size_t>(count / total);
}

/// <summary>
/// 指定したキーのタイムスケールを掛けたdeltaTimeを返す。
/// </summary>
/// <remarks>
/// スロー演出やヒットストップを、ゲーム全体ではなく特定の対象にだけ掛けるための仕組み。
/// 例えば"Camera"だけ等速のままプレイヤーをスローにする、といった使い分けができる。
/// キーが未登録の場合はスケールなしのdeltaTimeを返すため、
/// SetTimeScale()を呼び忘れても動作は止まらないが、演出は効かない。
/// </remarks>
float DeltaTimer::GetScaledDeltaTime(const std::string& key) const {
    auto itr = deltaTimeScaleMap_.find(key);
    if (itr != deltaTimeScaleMap_.end()) {
        return deltaTime_ * itr->second;
    }
    LOG_DEBUG("Key '{}' not found in deltaTimeScaleMap_. Returning unscaled deltaTime.", key);
    return deltaTime_;
}

void DeltaTimer::SetTimeScale(const std::string& key, float scale) {
    deltaTimeScaleMap_[key] = scale;
}

void DeltaTimer::EnableFixedDeltaTime(float _fixedSeconds) {
    fixedDeltaTimeEnabled_ = true;
    fixedDeltaTimeSeconds_ = _fixedSeconds;
    deltaTime_             = _fixedSeconds;
}

void DeltaTimer::DisableFixedDeltaTime() {
    fixedDeltaTimeEnabled_ = false;
    // 固定モード中は currentTime_/preTime_ を止めていたので、そのまま実時間計測に
    // 戻すと次のUpdate()が「固定モードに入る前」からの経過時間を巨大なデルタタイムとして
    // 計上してしまう。基準を今に合わせ直してから戻す
    currentTime_ = std::chrono::high_resolution_clock::now();
    preTime_     = currentTime_;
}
