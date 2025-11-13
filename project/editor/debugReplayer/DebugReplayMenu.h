#pragma once

#ifdef _DEBUG

/// interface
#include "editor/IEditor.h"

// parent
class DebugReplayWindow;

/// <summary>
/// デバッグリプレイのファイルメニュー
/// </summary>
class DebugReplayFileMenu
    : public Editor::Menu {
public:
    DebugReplayFileMenu(DebugReplayWindow* _parent);
    ~DebugReplayFileMenu() override;

    void Initialize() override;

private:
    DebugReplayWindow* parent_ = nullptr;
};

/// <summary>
/// デバッグリプレイのロードメニューアイテム
/// </summary>
class DebugReplayLoadMenuItem
    : public Editor::MenuItem {
public:
    DebugReplayLoadMenuItem(DebugReplayWindow* _parent);
    ~DebugReplayLoadMenuItem() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    std::string menuLabel_     = "";
    DebugReplayWindow* parent_ = nullptr;
};

/// <summary>
/// デバッグリプレイのロードコマンド
/// </summary>
class DebugReplayLoadFileCommand
    : public IEditCommand {
public:
    DebugReplayLoadFileCommand(DebugReplayWindow* _parent, const std::string& _filePath);
    ~DebugReplayLoadFileCommand() override;
    void Execute() override;
    void Undo() override;

private:
    DebugReplayWindow* parent_ = nullptr;
    std::string filePath_      = "";
    std::string prevFilePath_  = "";
};

#endif // _DEBUG
