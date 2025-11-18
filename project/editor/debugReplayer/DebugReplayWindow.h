#pragma once

#ifdef _DEBUG

/// interface
#include "editor/IEditor.h"

/// engine
#include "scene/SceneManager.h"
// input
#include "input/GamePadInput.h"
#include "input/KeyboardInput.h"
#include "input/MouseInput.h"
// debug
#include "debugReplayer/ReplayPlayer.h"

/// <summary>
/// デバッグリプレイウィンドウ
/// </summary>
class DebugReplayWindow
    : public Editor::Window {
public:
    DebugReplayWindow();
    ~DebugReplayWindow() override;

    void Initialize();
    void DrawGui();
    void Finalize();

    /// <summary>
    /// Replayerによる入力を適用してシーンマネージャーを更新する
    /// </summary>
    void UpdateSceneManager();

private:
    std::unique_ptr<SceneManager> sceneManager_ = nullptr;
    std::unique_ptr<ReplayPlayer> replayPlayer_ = nullptr;
    size_t replayFrameIndex_                    = 0;

    // input
    std::unique_ptr<KeyboardInput> keyboardInput_ = nullptr;
    std::unique_ptr<MouseInput> mouseInput_       = nullptr;
    std::unique_ptr<GamePadInput> gamePadInput_   = nullptr;

    bool isLoadReplayFile_ = false;

public:
    bool GetIsLoadReplayFile() const {
        return isLoadReplayFile_;
    }
    void SetIsLoadReplayFile(bool isLoad) {
        isLoadReplayFile_ = isLoad;
    }

    SceneManager* GetSceneManager() {
        return sceneManager_.get();
    }
    ReplayPlayer* GetReplayPlayer() {
        return replayPlayer_.get();
    }

    size_t GetReplayFrameIndex() const {
        return replayFrameIndex_;
    }
    void SetReplayFrameIndex(size_t index) {
        replayFrameIndex_ = index;
    }
};

class DebugReplayViewArea
    : public Editor::Area {
public:
    DebugReplayViewArea(DebugReplayWindow* _parent);
    ~DebugReplayViewArea() override;

    void Initialize() override;
    void DrawGui() override;

private:
    bool isResizing_           = true;
    DebugReplayWindow* parent_ = nullptr;
};

#endif // _DEBUG
