#pragma once

#ifdef _DEBUG

/// stl
#include <string>

/// interface
#include "editor/IEditor.h"

namespace OriGine {
namespace Editor {
// parent
class DebugReplayWindow;

/// <summary>
/// デバッグリプレイのファイルメニュー
/// </summary>
class DebugReplayFileMenu
    : public Menu {
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
    : public MenuItem {
public:
    DebugReplayLoadMenuItem(DebugReplayWindow* _parent);
    ~DebugReplayLoadMenuItem() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    ::std::string menuLabel_   = "";
    DebugReplayWindow* parent_ = nullptr;
};

/// <summary>
/// デバッグリプレイのロードコマンド
/// </summary>
class DebugReplayLoadFileCommand
    : public IEditCommand {
public:
    DebugReplayLoadFileCommand(DebugReplayWindow* _parent, const ::std::string& _filePath);
    ~DebugReplayLoadFileCommand() override;
    void Execute() override;
    void Undo() override;

private:
    DebugReplayWindow* parent_  = nullptr;
    ::std::string filePath_     = "";
    ::std::string prevFilePath_ = "";
};

} // namespace Editor
} // namespace OriGine

#endif // _DEBUG
