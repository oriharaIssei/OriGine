#ifdef _DEBUG

#include "SettingWindow.h"

/// stl
#include <algorithm>

/// api
#include <Windows.h>

/// collision
#include "component/collision/collider/base/CollisionCategoryManager.h"

/// engine
#include "Engine.h"
#include "winApp/WinApp.h"

/// editor
#include "editor/EditorController.h"

/// util
#include "util/globalVariables/GlobalVariables.h"
#include "util/nameof.h"
#include "util/StringUtil.h"

/// externals
#include "myGui/MyGui.h"

using namespace OriGine;

const std::string SettingWindow::kGlobalVariablesSceneName       = "Settings";
const std::string SettingWindowRegion::kGlobalVariablesGroupName = "Window";
const std::string SettingWindowRegion::kGlobalVariablesWindowStateGroupName = "WindowState";

SettingWindow::SettingWindow()
    : Editor::Window(nameof<SettingWindow>()) {}
SettingWindow::~SettingWindow() {}

void SettingWindow::Initialize() {
    isOpen_.Set(false);
    isOpen_.Sync();

    windowFlags_ = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;

    AddArea(std::make_shared<SettingWindowArea>());
    AddArea(std::make_shared<ProjectSettingArea>());
    AddArea(std::make_shared<CollisionSettingArea>());

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
    LoadSettings();
}

void SettingWindowRegion::LoadSettings() {
    GlobalVariables* globalVariables = GlobalVariables::GetInstance();
    // ウィンドウのタイトルとサイズを更新
    windowTitle_ = globalVariables->GetValue<std::string>(
        SettingWindow::kGlobalVariablesSceneName, SettingWindowRegion::kGlobalVariablesGroupName, "Title");
    windowSize_ = globalVariables->GetValue<Vec2f>(
        SettingWindow::kGlobalVariablesSceneName,
        SettingWindowRegion::kGlobalVariablesGroupName,
        "Size");

    windowPos_[0] = *globalVariables->AddValue<int32_t>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "PosX",
        0);
    windowPos_[1] = *globalVariables->AddValue<int32_t>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "PosY",
        0);
    windowClientSize_[0] = *globalVariables->AddValue<int32_t>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "Width",
        static_cast<int32_t>(windowSize_[X]));
    windowClientSize_[1] = *globalVariables->AddValue<int32_t>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "Height",
        static_cast<int32_t>(windowSize_[Y]));
    windowMode_ = *globalVariables->AddValue<int32_t>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "Mode",
        0);
    monitorIndex_ = *globalVariables->AddValue<int32_t>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "MonitorIndex",
        0);
    backgroundTransparent_ = *globalVariables->AddValue<bool>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "BackgroundTransparent",
        false);
    backgroundAlpha_ = *globalVariables->AddValue<int32_t>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "BackgroundAlpha",
        255);
    int32_t colorKey = *globalVariables->AddValue<int32_t>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "TransparencyColorKey",
        static_cast<int32_t>(RGB(0, 0, 0)));
    transparencyColor_[0] = GetRValue(static_cast<COLORREF>(colorKey));
    transparencyColor_[1] = GetGValue(static_cast<COLORREF>(colorKey));
    transparencyColor_[2] = GetBValue(static_cast<COLORREF>(colorKey));
    useTransparencyColorKey_ = *globalVariables->AddValue<bool>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "UseTransparencyColorKey",
        true);
}

void SettingWindowRegion::SaveSettings() {
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

    globalVariables->SetValue<int32_t>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "PosX",
        windowPos_[0]);
    globalVariables->SetValue<int32_t>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "PosY",
        windowPos_[1]);
    globalVariables->SetValue<int32_t>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "Width",
        windowClientSize_[0]);
    globalVariables->SetValue<int32_t>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "Height",
        windowClientSize_[1]);
    globalVariables->SetValue<int32_t>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "Mode",
        windowMode_);
    globalVariables->SetValue<int32_t>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "MonitorIndex",
        monitorIndex_);
    globalVariables->SetValue<bool>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "BackgroundTransparent",
        backgroundTransparent_);
    globalVariables->SetValue<int32_t>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "BackgroundAlpha",
        backgroundAlpha_);
    COLORREF colorKey = RGB(
        std::clamp(transparencyColor_[0], 0, 255),
        std::clamp(transparencyColor_[1], 0, 255),
        std::clamp(transparencyColor_[2], 0, 255));
    globalVariables->SetValue<int32_t>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "TransparencyColorKey",
        static_cast<int32_t>(colorKey));
    globalVariables->SetValue<bool>(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName,
        "UseTransparencyColorKey",
        useTransparencyColorKey_);

    globalVariables->SaveFile(
        SettingWindow::kGlobalVariablesSceneName,
        SettingWindowRegion::kGlobalVariablesGroupName);
    globalVariables->SaveFile(
        SettingWindow::kGlobalVariablesSceneName,
        kGlobalVariablesWindowStateGroupName);

    if (WinApp* winApp = Engine::GetInstance()->GetWinApp()) {
        winApp->RestoreWindowState();
    }
}

void SettingWindowRegion::DrawGui() {
    ImGui::InputText("Window Title", &windowTitle_);
    if (!windowTitle_.empty()) {
        windowTitle_ = TrimAfterNewline(windowTitle_);
    }

    ImGui::InputFloat2("Window Size", windowSize_.v, "%4.0f");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Saved Window State");
    ImGui::InputInt2("Window Position", windowPos_);
    ImGui::InputInt2("Client Size", windowClientSize_);

    const char* windowModeItems[] = {
        "Windowed",
        "Borderless Windowed",
        "Borderless Fullscreen",
        "Exclusive Fullscreen",
    };
    constexpr int32_t kWindowModeCount = static_cast<int32_t>(sizeof(windowModeItems) / sizeof(windowModeItems[0]));
    windowMode_ = std::clamp(windowMode_, 0, kWindowModeCount - 1);
    ImGui::Combo("Window Mode", &windowMode_, windowModeItems, kWindowModeCount);
    ImGui::InputInt("Monitor Index", &monitorIndex_);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Background Transparency");
    ImGui::Checkbox("Transparent Background", &backgroundTransparent_);
    ImGui::SliderInt("Opacity", &backgroundAlpha_, 0, 255);
    ImGui::Checkbox("Use Color Key", &useTransparencyColorKey_);
    ImGui::InputInt3("Color Key RGB", transparencyColor_);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Save")) {
        SaveSettings();
    }
    if (ImGui::Button("Cancel")) {
        LoadSettings();

        SettingWindow* settingWindow = OriGine::EditorController::GetInstance()->GetWindow<SettingWindow>();
        // ウィンドウを閉じる
        settingWindow->WindowCloseMessage();
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
        settingWindow->WindowCloseMessage();
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
        settingWindow->WindowOpenMessage();
    }
}

void SettingsWindowOpen::Finalize() {}

#pragma endregion

#pragma region "CollisionSetting"

CollisionSettingArea::CollisionSettingArea()
    : Editor::Area(nameof<CollisionSettingArea>()) {}
CollisionSettingArea::~CollisionSettingArea() {}

void CollisionSettingArea::Initialize() {
    AddRegion(std::make_shared<CollisionSettingRegion>());
}

void CollisionSettingArea::Finalize() {}

CollisionSettingRegion::CollisionSettingRegion()
    : Editor::Region(nameof<CollisionSettingRegion>()) {}

CollisionSettingRegion::~CollisionSettingRegion() {}

void CollisionSettingRegion::Initialize() {
    newCategoryName_[0] = '\0';
    // カテゴリの読み込みはアプリケーション起動時に行うため、ここでは何もしない

    // CellSizeをGlobalVariablesから読み込み
    GlobalVariables* gv  = GlobalVariables::GetInstance();
    spatialHashCellSize_ = gv->GetValue<float>(
        SettingWindow::kGlobalVariablesSceneName,
        "Collision",
        "SpatialHashCellSize");
    if (spatialHashCellSize_ <= 0.0f) {
        spatialHashCellSize_ = 100.0f; // デフォルト値
    }
}

void CollisionSettingRegion::DrawGui() {
    auto* manager          = OriGine::CollisionCategoryManager::GetInstance();
    const auto& categories = manager->GetCategories();

    // SpatialHash CellSize設定
    ImGui::Text("Spatial Hash Settings:");
    ImGui::DragFloat("Cell Size", &spatialHashCellSize_, 1.0f, 1.0f, 1000.0f, "%.1f");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Collision Categories (%zu / 32)", categories.size());
    ImGui::Separator();

    // 登録済みカテゴリ一覧
    for (const auto& [name, category] : categories) {
        ImGui::BulletText("%s (bit: 0x%08X)", name.c_str(), category.GetBits());
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 新規カテゴリ追加
    ImGui::Text("Add New Category:");
    ImGui::InputText("##newCategoryName", newCategoryName_, sizeof(newCategoryName_));
    ImGui::SameLine();
    if (ImGui::Button("Add")) {
        if (strlen(newCategoryName_) > 0) {
            if (manager->RegisterCategory(newCategoryName_)) {
                newCategoryName_[0] = '\0';
            }
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 衝突マトリクス編集
    ImGui::Text("Collision Matrix:");
    ImGui::Spacing();

    // 各カテゴリのマスクをチェックボックスで編集
    for (const auto& [rowName, rowCategory] : categories) {
        if (ImGui::TreeNode(rowName.c_str())) {
            uint32_t currentMask = manager->GetCategoryMask(rowName);

            for (const auto& [colName, colCategory] : categories) {
                bool canCollide = (currentMask & colCategory.GetBits()) != 0;
                if (ImGui::Checkbox((colName + "##mask_" + rowName).c_str(), &canCollide)) {
                    if (canCollide) {
                        currentMask |= colCategory.GetBits();
                    } else {
                        currentMask &= ~colCategory.GetBits();
                    }
                    manager->SetCategoryMask(rowName, currentMask);
                }
            }
            ImGui::TreePop();
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 保存ボタン
    if (ImGui::Button("Save Categories")) {
        manager->SaveToGlobalVariables();

        // CellSizeも保存
        GlobalVariables* gv = GlobalVariables::GetInstance();
        gv->SetValue<float>(
            SettingWindow::kGlobalVariablesSceneName,
            "Collision",
            "SpatialHashCellSize",
            spatialHashCellSize_);
        gv->SaveFile(
            SettingWindow::kGlobalVariablesSceneName,
            "Collision");
    }
}

void CollisionSettingRegion::Finalize() {}

#pragma endregion

#endif // _DEBUG
