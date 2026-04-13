#include "DevelopControlArea.h"

#ifdef _DEBUG

/// stl
#include <format>
#include <thread>

/// engine
#include "scene/Scene.h"
#include "winApp/WinApp.h"

/// editor
#include "editor/sceneEditor/SceneEditor.h"

/// util
#include "util/nameof.h"

using namespace OriGine;

DevelopControlArea::DevelopControlArea(SceneEditorWindow* _parentWindow)
    : Editor::Area(nameof<DevelopControlArea>()), parentWindow_(_parentWindow) {}
DevelopControlArea::~DevelopControlArea() {
    // ビルドスレッドが完了するまで待機
    WaitForBuildThread();
}

void DevelopControlArea::WaitForBuildThread() {
    shouldCancelBuild_ = true;
    if (buildThread_.joinable()) {
        buildThread_.join();
    }
    shouldCancelBuild_ = false;
}

void DevelopControlArea::Initialize() {
    AddRegion(std::make_shared<ControlRegion>(this));
}

DevelopControlArea::ControlRegion::ControlRegion(DevelopControlArea* _parentArea)
    : Editor::Region(nameof<ControlRegion>()), parentArea_(_parentArea) {}

DevelopControlArea::ControlRegion::~ControlRegion() {}

void DevelopControlArea::ControlRegion::Initialize() {}

void DevelopControlArea::ControlRegion::DrawGui() {

    if (ImGui::Button("Build Develop") && !parentArea_->isBuilding_) {
        auto* currentScene = parentArea_->GetParentWindow()->GetCurrentScene();
        if (!currentScene) {
            LOG_ERROR("No current scene found.");
            return;
        }
        /// ==========================================
        // Build
        /// ==========================================
        // ビルドコマンドの作成
        std::string buildCommand =
            parentArea_->buildTool_ + " "
            + parentArea_->projectName_
            + " /p:Configuration=" + parentArea_->configuration
            + " /p:Platform=" + parentArea_->platform;
        LOG_DEBUG("Executing build command: {}", buildCommand);

        // buildThread を立てる(Build中もエディターを操作できるように)
        // 前のビルドスレッドが完了している場合は join
        if (parentArea_->buildThread_.joinable()) {
            parentArea_->buildThread_.join();
        }
        parentArea_->isBuilding_        = true;
        parentArea_->shouldCancelBuild_ = false;
        parentArea_->buildThread_       = std::thread([this, cmd = std::move(buildCommand)]() {
            RunProcessAndWait(cmd);
            parentArea_->isBuilding_ = false;
        });
    }

    ImGui::SameLine();

    if (ImGui::Button("Run") && !parentArea_->isBuilding_) {
        auto* currentScene  = parentArea_->GetParentWindow()->GetCurrentScene();
        std::string exePath = std::filesystem::current_path().string() + parentArea_->exePath_;
        LOG_DEBUG("Executing application at path: {}", exePath);

        std::string runCommand = std::format("{} {} {}", exePath, "-s", currentScene->GetName()); // 実行ファイルパスと 実行する scene を送る
        // アプリケーションの実行
        int32_t result = std::system(runCommand.c_str());
        if (result != 0) {
            LOG_ERROR("Failed to run application. Error code: {}", result);
        } else {
            LOG_DEBUG("Application executed successfully.");
        }
    }
}
void DevelopControlArea::ControlRegion::Finalize() {}

#endif // _DEBUG
