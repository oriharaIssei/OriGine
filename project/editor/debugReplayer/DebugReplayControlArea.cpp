#include "DebugReplayControlArea.h"

#ifdef _DEBUG

#include "DebugReplayWindow.h"

/// util
#include "myGui/MyGui.h"

namespace OriGine {
namespace Editor {

DebugReplayControlArea::DebugReplayControlArea(DebugReplayWindow* _parent)
    : Editor::Area(nameof<DebugReplayControlArea>()), parent_(_parent) {}
DebugReplayControlArea::~DebugReplayControlArea() {}

void DebugReplayControlArea::Initialize() {
    // Regionの追加
    AddRegion(std::make_shared<DebugReplayControlRegion>(parent_));
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
    auto* replayPlayer = parent_->GetReplayPlayer();
    if (!replayPlayer || !replayPlayer->GetIsActive()) {
        ImGui::Text("No Replay Data Loaded.");
        // reset
        currentFrame_    = 0;
        totalFrameCount_ = 0;
        return;
    }

    std::string label = name_;

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
    totalFrameCount_ = static_cast<uint32_t>(replayPlayer->GetTotalFrameCount());

    // currentFrameIndex が変更されたら再生位置を変更
    label = "##InputCurrentFrame" + name_;

    constexpr float kFrameInputItemWidth = 64.f;
    ImGui::SetNextItemWidth(kFrameInputItemWidth);

    bool isEditing = InputGuiCommand<uint32_t>(label, currentFrame_, "%d", [this](uint32_t* _current) {
        auto* replayPlayer = parent_->GetReplayPlayer();
        if (replayPlayer) {
            isPlaying_ = false; // 入力で変更された場合は一時停止
            parent_->SetReplayFrameIndex(static_cast<size_t>(*_current));
        };
    });

    ImGui::SameLine();

    label = "##SliderCurrentFrame" + name_;
    // Sliderで currentFrameIndex を変更
    isEditing |= SlideGuiCommand<uint32_t>(label, currentFrame_, 0u, totalFrameCount_ > 0 ? totalFrameCount_ - 1 : 0, "%d", [this](uint32_t* _current) {
        auto* replayPlayer = parent_->GetReplayPlayer();
        if (replayPlayer) {
            isPlaying_ = false; // スライダーで変更された場合は一時停止
            parent_->SetReplayFrameIndex(static_cast<size_t>(*_current));
        };
    });

    // totalFrameCount を表示
    ImGui::SameLine();
    ImGui::Text("/ %d", totalFrameCount_);

    // 再生中の場合、currentFrameIndex を進める
    if (isPlaying_) {
        // Guiを使っているときは 再生を一時停止する
        if (isEditing) {
            isPlaying_ = false;
            return;
        }

        ++currentFrame_;
        if (currentFrame_ >= totalFrameCount_) {
            currentFrame_ = totalFrameCount_ > 0 ? totalFrameCount_ - 1 : 0;
            isPlaying_    = false; // 最後まで再生したら停止
        }
        parent_->SetReplayFrameIndex(static_cast<size_t>(currentFrame_));
    }
}

} // namespace Editor
} // namespace OriGine

#endif // _DEBUG
