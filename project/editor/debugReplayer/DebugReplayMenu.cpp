#include "DebugReplayMenu.h"

#ifdef _DEBUG

/// parent
#include "DebugReplayWindow.h"

/// engine
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
// debugReplayer
#include "debugReplayer/base/ReplayData.h"
#include "debugReplayer/ReplayPlayer.h"

/// util
#include "myFileSystem/MyFileSystem.h"
#include "nameof.h"
// gui
#include "myGui/MyGui.h"

/// externals
#include <imgui/imgui.h>

DebugReplayFileMenu::DebugReplayFileMenu(DebugReplayWindow* _parent)
    : Editor::Menu(nameof<DebugReplayFileMenu>()), parent_(_parent) {}
DebugReplayFileMenu::~DebugReplayFileMenu() {}

void DebugReplayFileMenu::Initialize() {
    // menuItem の追加
    addMenuItem(std::make_shared<DebugReplayLoadMenuItem>(parent_));
}

DebugReplayLoadMenuItem::DebugReplayLoadMenuItem(DebugReplayWindow* _parent)
    : Editor::MenuItem(nameof<DebugReplayLoadMenuItem>()), parent_(_parent) {}
DebugReplayLoadMenuItem::~DebugReplayLoadMenuItem() {}

void DebugReplayLoadMenuItem::Initialize() {
    menuLabel_ = "Load##" + name_;
}

void DebugReplayLoadMenuItem::DrawGui() {
    if (ImGui::MenuItem(menuLabel_.c_str())) {
        // ファイルダイアログを開く
        std::string directory = kReplayFolderName;
        std::string filename  = "";

        // fileの取得に成功したら
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, filename, {kReplayFileExtension})) {
            ReplayPlayer* replayPlayer = parent_->getReplayPlayer();
            // skip
            if (!replayPlayer) {
                LOG_ERROR("ReplayPlayer is nullptr.");
                return;
            }
            // active だったら Finalize する
            if (replayPlayer->getIsActive()) {
                replayPlayer->Finalize();
            }

            // ファイルの読み込み
            std::string filePath = kApplicationResourceDirectory + "/" + directory + "/" + filename;

            // コマンドとして登録
            EditorController::getInstance()->pushCommand(std::make_unique<DebugReplayLoadFileCommand>(parent_, filePath));
        }
    }
}

void DebugReplayLoadMenuItem::Finalize() {
    parent_ = nullptr;
    menuLabel_.clear();
}

DebugReplayLoadFileCommand::DebugReplayLoadFileCommand(DebugReplayWindow* _parent, const std::string& _filePath)
    : parent_(_parent), filePath_(_filePath) {
    auto replayPlayer = parent_->getReplayPlayer();
    if (!replayPlayer || !replayPlayer->getIsActive()) {
        return;
    }
    prevFilePath_ = replayPlayer->getFilepath();
}

DebugReplayLoadFileCommand::~DebugReplayLoadFileCommand() {
    parent_ = nullptr;

    filePath_.clear();
    prevFilePath_.clear();
}

void DebugReplayLoadFileCommand::Execute() {
    auto replayPlayer = parent_->getReplayPlayer();
    if (!replayPlayer) {
        LOG_ERROR("ReplayPlayer is nullptr.");
        return;
    }

    // active だったら Finalize する
    if (replayPlayer->getIsActive()) {
        replayPlayer->Finalize();
    }
    // ファイルの読み込み
    replayPlayer->Initialize(filePath_, parent_->getSceneManager());
    parent_->setIsLoadReplayFile(true);
}

void DebugReplayLoadFileCommand::Undo() {
    auto replayPlayer = parent_->getReplayPlayer();
    if (!replayPlayer) {
        LOG_ERROR("ReplayPlayer is nullptr.");
        return;
    }
    // active だったら Finalize する
    if (replayPlayer->getIsActive()) {
        replayPlayer->Finalize();
    }
    // ファイルの読み込み
    replayPlayer->Initialize(prevFilePath_, parent_->getSceneManager());
    parent_->setIsLoadReplayFile(true);
}

#endif // _DEBUG
