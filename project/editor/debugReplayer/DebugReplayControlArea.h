#pragma once

#ifdef _DEBUG

/// interface
#include "editor/IEditor.h"

namespace OriGine {
namespace Editor {
// parent
class DebugReplayWindow;

/// <summary>
/// デバッグのリプレイを制御するエリア
/// </summary>
class DebugReplayControlArea
    : public Area {
public:
    DebugReplayControlArea(DebugReplayWindow* _parent);
    ~DebugReplayControlArea() override;

    void Initialize();

private:
    DebugReplayWindow* parent_ = nullptr;
};

/// <summary>
/// デバッグのリプレイを制御する Guiの集まり
/// </summary>
class DebugReplayControlRegion
    : public Region {
public:
    DebugReplayControlRegion(DebugReplayWindow* _parent);
    ~DebugReplayControlRegion() override;

    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    DebugReplayWindow* parent_ = nullptr;

    uint32_t totalFrameCount_ = 0;
    uint32_t currentFrame_    = 0;
    bool isPlaying_           = false;
};

} // namespace Editor
} // namespace OriGine

#endif // _DEBUG
