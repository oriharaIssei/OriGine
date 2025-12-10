#ifdef _DEBUG

#include "SettingWindow.h"

/// editor
#include "editor/EditorController.h"

#include "util/globalVariables/GlobalVariables.h"
// util
#include "util/nameof.h"

using namespace OriGine;

const std::string SettingWindow::kGlobalVariablesSceneName       = "Settings";
const std::string SettingWindowRegion::kGlobalVariablesGroupName = "Window";

SettingWindow::SettingWindow()
    : Editor::Window(nameof<SettingWindow>()) {}
SettingWindow::~SettingWindow() {}

void SettingWindow::Initialize() {
    isOpen_.Set(false);
    isOpen_.Sync();

    windowFlags_ = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;

    AddArea(std::make_shared<SettingWindowArea>());
    AddArea(std::make_shared<ProjectSettingArea>());

    OriGine::EditorController::GetInstance()->AddMainMenu(
        std::make_unique<SettingsMenu>());
}

void SettingWindow::DrawGui() {
    if (isOpen_.Current()) {
        // 開いている間はずっとフォーカスする
        // Areaを含めて、どれか一つでもフォーカスされていれば良い
        bool isAnyOneFocused = isFocused_.Current();
        if (!isAnyOneFocused) {
            for (const auto& areaPair : areas_) {
                if (areaPair.second->IsFocused().Current()) {
                    isAnyOneFocused = true;
                    break;
                }
            }
        }
        if (!isAnyOneFocused) {
            ImGui::SetNextWindowFocus();
        }
    }

    Editor::Window::DrawGui();
}

void SettingWindow::Finalize() {
    Editor::Window::Finalize();
}

#pragma region "SettingWindow"

SettingWindowArea::SettingWindowArea()
    : Editor::Area(nameof<SettingWindowArea>()) {}
SettingWindowArea::~SettingWindowArea() {}

void SettingWindowArea::Initialize() {
    AddRegion(std::make_shared<SettingWindowRegion>());
}

void SettingWindowArea::Finalize() {}

SettingWindowRegion::SettingWindowRegion()
    : Editor::Region(nameof<SettingWindowRegion>()) {}
SettingWindowRegion::~SettingWindowRegion() {}

void SettingWindowRegion::Initialize() {
    GlobalVariables* globalVariables = GlobalVariables::GetInstance();
    // ウィンドウのタイトルとサイズを更新
    windowTitle_ = globalVariables->GetValue<std::string>(
        SettingWindow::kGlobalVariablesSceneName, SettingWindowRegion::kGlobalVariablesGroupName, "Title");
    windowSize_ = globalVariables->GetValue<Vec2f>(
        SettingWindow::kGlobalVariablesSceneName,
        SettingWindowRegion::kGlobalVariablesGroupName,
        "Size");
}
void SettingWindowRegion::DrawGui() {
    ImGui::InputText("Window Title", &windowTitle_[0], sizeof(char) * 256);
    ImGui::InputFloat2("Window Size", windowSize_.v, "%4.0f");

    ImGui::Spacing();

    if (ImGui::Button("Save")) {
        GlobalVariables* globalVariables = GlobalVariables::GetInstance();
        // ウィンドウのタイトルとサイズを更新
        globalVariables->SetValue<std::string>(
            SettingWindow::kGlobalVariablesSceneName,
            SettingWindowRegion::kGlobalVariablesGroupName,
            "Title",
            windowTitle_);
        globalVariables->SetValue<Vec2f>(
            SettingWindow::kGlobalVariablesSceneName,
            SettingWindowRegion::kGlobalVariablesGroupName,
            "Size",
            windowSize_);

        globalVariables->SaveFile(
            SettingWindow::kGlobalVariablesSceneName,
            SettingWindowRegion::kGlobalVariablesGroupName);
    }
    if (ImGui::Button("Cancel")) {
        // 設定をリセット
        GlobalVariables* globalVariables = GlobalVariables::GetInstance();
        // ウィンドウのタイトルとサイズを更新
        windowTitle_ = globalVariables->GetValue<std::string>(
            SettingWindow::kGlobalVariablesSceneName, SettingWindowRegion::kGlobalVariablesGroupName, "Title");
        windowSize_ = globalVariables->GetValue<Vec2f>(
            SettingWindow::kGlobalVariablesSceneName,
            SettingWindowRegion::kGlobalVariablesGroupName,
            "Size");

        SettingWindow* settingWindow = OriGine::EditorController::GetInstance()->GetWindow<SettingWindow>();
        // ウィンドウを閉じる
        settingWindow->WindowCloseMassage();
    }
}

void SettingWindowRegion::Finalize() {}

#pragma endregion

#pragma region "ProjectSetting"

ProjectSettingArea::ProjectSettingArea()
    : Editor::Area(nameof<ProjectSettingArea>()) {}
ProjectSettingArea::~ProjectSettingArea() {}

void ProjectSettingArea::Initialize() {
    AddRegion(std::make_shared<ProjectSettingRegion>());
}

void ProjectSettingArea::Finalize() {}

ProjectSettingRegion::ProjectSettingRegion()
    : Editor::Region(nameof<ProjectSettingRegion>()) {}

ProjectSettingRegion::~ProjectSettingRegion() {}

void ProjectSettingRegion::Initialize() {
    // 設定をリセット
    GlobalVariables* globalVariables = GlobalVariables::GetInstance();
    // ウィンドウのタイトルとサイズを更新
    startUpSceneName_ = globalVariables->GetValue<std::string>(
        SettingWindow::kGlobalVariablesSceneName,
        "Scene",
        "StartupSceneName");

    gravity_ = globalVariables->GetValue<float>(
        SettingWindow::kGlobalVariablesSceneName,
        "Physics",
        "Gravity");
}

void ProjectSettingRegion::DrawGui() {
    ImGui::InputText("StartupSceneName", &startUpSceneName_[0], sizeof(char) * 256);
    ImGui::Spacing();
    ImGui::DragFloat("Gravity", &gravity_, 0.01f);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Save")) {
        GlobalVariables* globalVariables = GlobalVariables::GetInstance();
        // 起動時のシーン名を更新
        globalVariables->SetValue<std::string>(
            SettingWindow::kGlobalVariablesSceneName,
            "Scene",
            "StartupSceneName",
            startUpSceneName_);
        // 重力の値を更新
        globalVariables->SetValue<float>(
            SettingWindow::kGlobalVariablesSceneName,
            "Physics",
            "Gravity",
            gravity_);

        globalVariables->SaveFile(
            SettingWindow::kGlobalVariablesSceneName,
            "Scene");
        globalVariables->SaveFile(
            SettingWindow::kGlobalVariablesSceneName,
            "Physics");
    }
    if (ImGui::Button("Cancel")) {
        // 設定をリセット
        GlobalVariables* globalVariables = GlobalVariables::GetInstance();
        // ウィンドウのタイトルとサイズを更新
        startUpSceneName_ = globalVariables->GetValue<std::string>(
            SettingWindow::kGlobalVariablesSceneName,
            "Scene",
            "StartupSceneName");

        gravity_ = globalVariables->GetValue<float>(
            SettingWindow::kGlobalVariablesSceneName,
            "Physics",
            "Gravity");

        SettingWindow* settingWindow = OriGine::EditorController::GetInstance()->GetWindow<SettingWindow>();
        // ウィンドウを閉じる
        settingWindow->WindowCloseMassage();
    }
}

void ProjectSettingRegion::Finalize() {}

#pragma endregion

#pragma region "SettingMenu"

SettingsMenu::SettingsMenu()
    : Editor::Menu(nameof<SettingsMenu>()) {}

SettingsMenu::~SettingsMenu() {}

void SettingsMenu::Initialize() {
    AddMenuItem(std::make_shared<SettingsWindowOpen>());
}

void SettingsMenu::Finalize() {}

SettingsWindowOpen::SettingsWindowOpen()
    : Editor::MenuItem("SettingsWindowOpen") {}

SettingsWindowOpen::~SettingsWindowOpen() {}

void SettingsWindowOpen::Initialize() {}

void SettingsWindowOpen::DrawGui() {
    SettingWindow* settingWindow = OriGine::EditorController::GetInstance()->GetWindow<SettingWindow>();
    bool isOpenSettingWindow     = settingWindow->IsOpen();

    if (ImGui::MenuItem("Settings", nullptr, &isOpenSettingWindow, !isOpenSettingWindow)) {
        // SettingsWindowを開く
        settingWindow->WindowOpenMassage();
    }
}

void SettingsWindowOpen::Finalize() {}

#pragma endregion

#endif // _DEBUG
