#include "DebugReplayControlArea.h"

#ifdef _DEBUG

#include "DebugReplayWindow.h"

/// util
#include "myGui/MyGui.h"

DebugReplayControlArea::DebugReplayControlArea(DebugReplayWindow* _parent)
    : Editor::Area(nameof<DebugReplayControlArea>()), parent_(_parent) {}
DebugReplayControlArea::~DebugReplayControlArea() {}

void DebugReplayControlArea::Initialize() {
    // Regionの追加
    addRegion(std::make_shared<DebugReplayControlRegion>(parent_));
}

DebugReplayControlRegion::DebugReplayControlRegion(DebugReplayWindow* _parent)
    : Editor::Region(nameof<DebugReplayControlRegion>()), parent_(_parent) {}
DebugReplayControlRegion::~DebugReplayControlRegion() {}

void DebugReplayControlRegion::Initialize() {
    // reset
    currentFrame_    = 0;
    totalFrameCount_ = 0;

    isPlaying_ = false;
}

void DebugReplayControlRegion::Finalize() {
    // reset
    currentFrame_    = 0;
    totalFrameCount_ = 0;

    isPlaying_ = false;
}

void DebugReplayControlRegion::DrawGui() {
    // リプレイデータがロードされていない場合は何も表示しない
    auto* replayPlayer = parent_->getReplayPlayer();
    if (!replayPlayer || !replayPlayer->getIsActive()) {
        ImGui::Text("No Replay Data Loaded.");
        // reset
        currentFrame_    = 0;
        totalFrameCount_ = 0;
        return;
    }

    const std::string& replayFilePath = replayPlayer->getFilepath();
    std::string label                 = replayPlayer->getFilepath();

    // play / pause ボタン
    // 再生中は pause ボタン、停止中は play ボタンを表示
    if (isPlaying_) {
        if (ImGui::Button("Pause")) {
            isPlaying_ = false;
        }
    } else {
        if (ImGui::Button("Play")) {
            isPlaying_ = true;
        }
    }

    // totalFrameCount を常時取得
    totalFrameCount_ = static_cast<uint32_t>(replayPlayer->getTotalFrameCount());

    // currentFrameIndex が変更されたら再生位置を変更
    label = "##CurrentFrame" + replayFilePath;

    constexpr float kFrameInputItemWidth = 64.f;
    ImGui::SetNextItemWidth(kFrameInputItemWidth);

    // Guiを使っているときは 再生を一時停止する
    isPlaying_ |= !InputGuiCommand<uint32_t>(label, currentFrame_, "%d", [this](uint32_t* _current) {
        auto* replayPlayer = parent_->getReplayPlayer();
        if (replayPlayer) {
            isPlaying_ = false; // 入力で変更された場合は一時停止
            parent_->setReplayFrameIndex(static_cast<size_t>(*_current));
        };
    });

    ImGui::SameLine();

    // Sliderで currentFrameIndex を変更
    // Guiを使っているときは 再生を一時停止する
    isPlaying_ |= !SlideGuiCommand<uint32_t>(label, currentFrame_, 0u, totalFrameCount_ > 0 ? totalFrameCount_ - 1 : 0, "%d", [this](uint32_t* _current) {
        auto* replayPlayer = parent_->getReplayPlayer();
        if (replayPlayer) {
            isPlaying_ = false; // スライダーで変更された場合は一時停止
            parent_->setReplayFrameIndex(static_cast<size_t>(*_current));
        };
    });

    // totalFrameCount を表示
    ImGui::SameLine();
    ImGui::Text("/ %d", totalFrameCount_);

    // 再生中の場合、currentFrameIndex を進める
    if (isPlaying_) {
        ++currentFrame_;
        if (currentFrame_ >= totalFrameCount_) {
            currentFrame_ = totalFrameCount_ > 0 ? totalFrameCount_ - 1 : 0;
            isPlaying_    = false; // 最後まで再生したら停止
        }
        parent_->setReplayFrameIndex(static_cast<size_t>(currentFrame_));
    }
}

#endif // _DEBUG
