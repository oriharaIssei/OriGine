#include "IEditor.h"

#ifdef DEBUG
#include "EditorController.h"
#include "imGuiManager/ImGuiManager.h"
#include <imgui/imgui_internal.h>

namespace OriGine {

namespace Editor {

void Menu::DrawGui() {
    for (auto& [name, item] : menuItems_) {
        if (!item) {
            continue;
        }
        item->DrawGui();
    }
}

void Area::DrawGui() {
    bool isOpen = isOpen_.Current();

    if (!isOpen) {
        isOpen_.Set(isOpen);
        isFocused_.Set(ImGui::IsWindowFocused());
        UpdateFocusAndOpenState();
        return; // ウィンドウが開いていない場合はここで終了
    }

    if (ImGui::Begin(name_.c_str(), &isOpen)) {
        areaSize_ = ImGui::GetContentRegionAvail();

        // regions_ 描画
        for (auto& [name, region] : regions_) {
            if (region) {
                ImGui::BeginGroup();
                region->DrawGui();
                ImGui::EndGroup();
            }
        }
    }

    isOpen_.Set(isOpen);
    isFocused_.Set(ImGui::IsWindowFocused());
    UpdateFocusAndOpenState();
    ImGui::End();
}

void Area::Finalize() {
    for (auto& [name, region] : regions_) {
        region->Finalize();
        region.reset();
    }
}

void Area::UpdateFocusAndOpenState() {
    if (isOpen_.IsChanged()) {
        if (isOpen_.IsTrigger()) {
            auto command = std::make_unique<WindowOpenCommand>(&isOpen_, true);
            EditorController::GetInstance()->PushCommand(std::move(command));
        } else if (isOpen_.IsRelease()) {
            auto command = std::make_unique<WindowOpenCommand>(&isOpen_, false);
            EditorController::GetInstance()->PushCommand(std::move(command));
        }
    }
}

void Window::UpdateFocusAndOpenState() {
    if (isFocused_.IsChanged()) {
        if (isFocused_.IsTrigger()) {
            auto command = std::make_unique<WindowFocusCommand>(title_, &isFocused_, true);
            EditorController::GetInstance()->PushCommand(std::move(command));
        } else if (isFocused_.IsRelease()) {
            auto command = std::make_unique<WindowFocusCommand>(title_, &isFocused_, false);
            EditorController::GetInstance()->PushCommand(std::move(command));
        }
    }
    if (isOpen_.IsChanged()) {
        if (isOpen_.IsTrigger()) {
            auto command = std::make_unique<WindowOpenCommand>(&isOpen_, true);
            EditorController::GetInstance()->PushCommand(std::move(command));
        } else if (isOpen_.IsRelease()) {
            auto command = std::make_unique<WindowOpenCommand>(&isOpen_, false);
            EditorController::GetInstance()->PushCommand(std::move(command));
        }
    }
}

void Window::DrawGui() {
    ///=================================================================================================
    // Main DockSpace Window
    ///=================================================================================================

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    if (isMaximized_) {
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
    }

    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PopStyleVar(2);

    bool isOpen = isOpen_.Current();
    if (!isOpen) {
        return; // ウィンドウが開いていない場合はここで終了
    }

    if (ImGui::Begin(title_.c_str(), &isOpen, windowFlags_)) {

        windowPos_  = ImGui::GetWindowPos();
        windowSize_ = ImGui::GetWindowSize();

        // 固有のDockSpace ID
        ImGuiID dockspaceID = ImGui::GetID((title_ + "_DockSpace").c_str());

        ImGui::DockSpace(dockspaceID, ImVec2(0, 0), dockFlags_);

        // メニュー
        if (ImGui::BeginMenuBar()) {
            for (auto& [name, menu] : menus_) {
                if (menu && ImGui::BeginMenu(name.c_str())) {
                    menu->SetOpen(true);
                    menu->DrawGui();
                    ImGui::EndMenu();
                }
            }
            ImGui::EndMenuBar();
        }

        // 子ウィンドウ描画
        for (auto& [name, area] : areas_) {
            if (area) {
                // 子ウィンドウに親DockSpace IDを渡す
                area->DrawGui();
            }
        }
    }

    isOpen_.Set(isOpen);
    isFocused_.Set(ImGui::IsWindowFocused());
    UpdateFocusAndOpenState();

    ImGui::End();
}

void Window::Finalize() {
    // エリアの終了処理
    for (auto& [name, area] : areas_) {
        area->Finalize();
    }
    areas_.clear();

    // メニューの終了処理
    for (auto& [name, menu] : menus_) {
        menu->Finalize();
    }
    menus_.clear();
}

void Window::WindowOpenMassage() {
    auto command = std::make_unique<WindowOpenCommand>(&isOpen_, true);
    EditorController::GetInstance()->PushCommand(std::move(command));
}

void Window::WindowCloseMassage() {
    auto command = std::make_unique<WindowOpenCommand>(&isOpen_, false);
    EditorController::GetInstance()->PushCommand(std::move(command));
}

void Menu::Finalize() {
    for (auto& [name, item] : menuItems_) {
        item->Finalize();
        item.reset();
    }
}

Vec2f ConvertMouseToGuiWindow(const Vec2f& _mousePos, const Vec2f& _guiWindowLT, const ImVec2& _guiWindowSize, const Vec2f& _originalResolution) {
    // SceneView 内での相対的なマウス座標を計算
    Vec2f relative = _mousePos - _guiWindowLT;

    // SceneView のスケールを計算
    float scaleX = _originalResolution[X] / _guiWindowSize[X];
    float scaleY = _originalResolution[Y] / _guiWindowSize[Y];

    // ゲーム内の座標に変換
    Vec2f gamePos;
    gamePos[X] = relative[X] * scaleX;
    gamePos[Y] = relative[Y] * scaleY;

    return gamePos;
}

WindowOpenCommand::WindowOpenCommand(
    DiffValue<bool>* _windowOpenState, bool _to)
    : windowOpenState_(_windowOpenState), to_(_to) {}
WindowOpenCommand::~WindowOpenCommand() {}

void WindowOpenCommand::Execute() {
    if (!windowOpenState_) {
        LOG_ERROR("WindowOpenCommand: Window is null.");
        return;
    }
    windowOpenState_->Set(to_);
    windowOpenState_->Sync();
}

void WindowOpenCommand::Undo() {
    if (!windowOpenState_) {
        LOG_ERROR("WindowOpenCommand: Window is null.");
        return;
    }
    windowOpenState_->Set(!to_);
    windowOpenState_->Sync();
}

WindowFocusCommand::WindowFocusCommand(
    const std::string& _windowName, DiffValue<bool>* _windowOpenState, bool _to)
    : windowName_(_windowName), windowOpenState_(_windowOpenState), to_(_to) {
}
WindowFocusCommand::~WindowFocusCommand() {}

void WindowFocusCommand::Execute() {
    if (!windowOpenState_) {
        LOG_ERROR("WindowFocusCommand: Window is null.");
        return;
    }
    // ウィンドウのフォーカス状態を設定
    windowOpenState_->Set(to_);
    windowOpenState_->Sync();
}

void WindowFocusCommand::Undo() {
    if (!windowOpenState_) {
        LOG_ERROR("WindowFocusCommand: Window is null.");
        return;
    }
    // ウィンドウのフォーカス状態を元に戻す
    windowOpenState_->Set(!to_);
    windowOpenState_->Sync();
}

} // namespace Editor
} // namespace OriGine

#endif // DEBUG
