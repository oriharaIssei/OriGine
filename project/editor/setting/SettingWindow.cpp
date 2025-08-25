#ifdef _DEBUG

#include "SettingWindow.h"

/// editor
#include "editor/EditorController.h"

#include "util/globalVariables/GlobalVariables.h"
// util
#include "util/nameof.h"

const std::string SettingWindow::kGlobalVariablesSceneName       = "Settings";
const std::string WindowSettingRegion::kGlobalVariablesGroupName = "Window";

SettingWindow::SettingWindow()
    : Editor::Window(nameof<SettingWindow>()) {}
SettingWindow::~SettingWindow() {}

void SettingWindow::Initialize() {
    isOpen_.set(false);
    isOpen_.sync();

    windowFlags_ = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDocking;

    addArea(std::make_shared<WindowSettingArea>());
    addArea(std::make_shared<ProjectSettingArea>());

    EditorController::getInstance()->addMainMenu(
        std::make_unique<SettingsMenu>());
}

void SettingWindow::Finalize() {
    Editor::Window::Finalize();
}

#pragma region "WindowSetting"

WindowSettingArea::WindowSettingArea()
    : Editor::Area(nameof<WindowSettingArea>()) {}
WindowSettingArea::~WindowSettingArea() {}

void WindowSettingArea::Initialize() {
    addRegion(std::make_shared<WindowSettingRegion>());
}

void WindowSettingArea::Finalize() {}

WindowSettingRegion::WindowSettingRegion()
    : Editor::Region(nameof<WindowSettingRegion>()) {}
WindowSettingRegion::~WindowSettingRegion() {}

void WindowSettingRegion::Initialize() {
    GlobalVariables* globalVariables = GlobalVariables::getInstance();
    // ウィンドウのタイトルとサイズを更新
    windowTitle_ = globalVariables->getValue<std::string>(
        SettingWindow::kGlobalVariablesSceneName, WindowSettingRegion::kGlobalVariablesGroupName, "Title");
    windowSize_ = globalVariables->getValue<Vec2f>(
        SettingWindow::kGlobalVariablesSceneName,
        WindowSettingRegion::kGlobalVariablesGroupName,
        "Size");
}
void WindowSettingRegion::DrawGui() {
    ImGui::InputText("Window Title", &windowTitle_[0], sizeof(char) * 256);
    ImGui::InputFloat2("Window Size", windowSize_.v, "%4.0f");

    ImGui::Spacing();

    if (ImGui::Button("Save")) {
        GlobalVariables* globalVariables = GlobalVariables::getInstance();
        // ウィンドウのタイトルとサイズを更新
        globalVariables->setValue<std::string>(
            SettingWindow::kGlobalVariablesSceneName,
            WindowSettingRegion::kGlobalVariablesGroupName,
            "Title",
            windowTitle_);
        globalVariables->setValue<Vec2f>(
            SettingWindow::kGlobalVariablesSceneName,
            WindowSettingRegion::kGlobalVariablesGroupName,
            "Size",
            windowSize_);

        globalVariables->SaveFile(
            SettingWindow::kGlobalVariablesSceneName,
            WindowSettingRegion::kGlobalVariablesGroupName);
    }
    if (ImGui::Button("Cancel")) {
        // 設定をリセット
        GlobalVariables* globalVariables = GlobalVariables::getInstance();
        // ウィンドウのタイトルとサイズを更新
        windowTitle_ = globalVariables->getValue<std::string>(
            SettingWindow::kGlobalVariablesSceneName, WindowSettingRegion::kGlobalVariablesGroupName, "Title");
        windowSize_ = globalVariables->getValue<Vec2f>(
            SettingWindow::kGlobalVariablesSceneName,
            WindowSettingRegion::kGlobalVariablesGroupName,
            "Size");

        SettingWindow* settingWindow = EditorController::getInstance()->getWindow<SettingWindow>();
        // ウィンドウの状態を更新
        settingWindow->setOpen(false);
    }
}

void WindowSettingRegion::Finalize() {}

#pragma endregion

#pragma region "ProjectSetting"

ProjectSettingArea::ProjectSettingArea()
    : Editor::Area(nameof<ProjectSettingArea>()) {}
ProjectSettingArea::~ProjectSettingArea() {}

void ProjectSettingArea::Initialize() {
    addRegion(std::make_shared<ProjectSettingRegion>());
}

void ProjectSettingArea::Finalize() {}

ProjectSettingRegion::ProjectSettingRegion()
    : Editor::Region(nameof<ProjectSettingRegion>()) {}

ProjectSettingRegion::~ProjectSettingRegion() {}

void ProjectSettingRegion::Initialize() {
    // 設定をリセット
    GlobalVariables* globalVariables = GlobalVariables::getInstance();
    // ウィンドウのタイトルとサイズを更新
    startUpSceneName_ = globalVariables->getValue<std::string>(
        SettingWindow::kGlobalVariablesSceneName,
        "Scene",
        "StartupSceneName");

    gravity_ = globalVariables->getValue<float>(
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
        GlobalVariables* globalVariables = GlobalVariables::getInstance();
        // 起動時のシーン名を更新
        globalVariables->setValue<std::string>(
            SettingWindow::kGlobalVariablesSceneName,
            "Scene",
            "StartupSceneName",
            startUpSceneName_);
        // 重力の値を更新
        globalVariables->setValue<float>(
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
        GlobalVariables* globalVariables = GlobalVariables::getInstance();
        // ウィンドウのタイトルとサイズを更新
        startUpSceneName_ = globalVariables->getValue<std::string>(
            SettingWindow::kGlobalVariablesSceneName,
            "Scene",
            "StartupSceneName");

        gravity_ = globalVariables->getValue<float>(
            SettingWindow::kGlobalVariablesSceneName,
            "Physics",
            "Gravity");

        SettingWindow* settingWindow = EditorController::getInstance()->getWindow<SettingWindow>();
        // ウィンドウの状態を更新
        settingWindow->setOpen(false);
    }
}

void ProjectSettingRegion::Finalize() {}

#pragma endregion

#pragma region "SettingMenu"

SettingsMenu::SettingsMenu()
    : Editor::Menu(nameof<SettingsMenu>()) {}

SettingsMenu::~SettingsMenu() {}

void SettingsMenu::Initialize() {
    addMenuItem(std::make_shared<SettingsWindowOpen>());
}

void SettingsMenu::Finalize() {}

SettingsWindowOpen::SettingsWindowOpen()
    : Editor::MenuItem("SettingsWindowOpen") {}

SettingsWindowOpen::~SettingsWindowOpen() {}

void SettingsWindowOpen::Initialize() {}

void SettingsWindowOpen::DrawGui() {
    SettingWindow* settingWindow = EditorController::getInstance()->getWindow<SettingWindow>();
    bool isOpenSettingWindow     = settingWindow->isOpen();

    if (ImGui::MenuItem("Settings", nullptr, &isOpenSettingWindow, !isOpenSettingWindow)) {
        // SettingsWindowを開く
        settingWindow->setOpen(true);
        settingWindow->setFocused(true);
    }
}

void SettingsWindowOpen::Finalize() {}

#pragma endregion

#endif // _DEBUG
