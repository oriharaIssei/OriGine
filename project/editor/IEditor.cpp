#include "IEditor.h"

#ifdef DEBUG
#include "EditorController.h"
#include "imGuiManager/ImGuiManager.h"
#include <imgui/imgui.h>
#endif // DEBUG

void Editor::Menu::DrawGui() {
    for (auto& [name, item] : menuItems_) {
        if (!item) {
            continue;
        }
        item->DrawGui();
    }
}

void Editor::Area::DrawGui() {
    bool isOpen = isOpen_.current();
    if (ImGui::Begin(name_.c_str(), &isOpen)) {
        areaSize_ = ImGui::GetContentRegionAvail();

        for (auto& [name, region] : regions_) {
            if (region) {
                ImGui::BeginGroup();
                region->DrawGui();
                ImGui::EndGroup();
            }
        }
    }

    isOpen_.set(isOpen);
    isFocused_.set(ImGui::IsWindowFocused());
    UpdateFocusAndOpenState();

    ImGui::End();
}

void Editor::Area::Finalize() {
    for (auto& [name, region] : regions_) {
        region->Finalize();
        region.reset();
    }
}

void Editor::Area::UpdateFocusAndOpenState() {
    if (isFocused_.isChanged()) {
        if (isFocused_.isTrigger()) {
            auto command = std::make_unique<WindowFocusCommand>(name_, &isOpen_, true);
            EditorController::getInstance()->pushCommand(std::move(command));
        } else if (isFocused_.isRelease()) {
            auto command = std::make_unique<WindowFocusCommand>(name_, &isOpen_, false);
            EditorController::getInstance()->pushCommand(std::move(command));
        }
    }
    if (isOpen_.isChanged()) {
        if (isOpen_.isTrigger()) {
            auto command = std::make_unique<WindowOpenCommand>(&isOpen_, true);
            EditorController::getInstance()->pushCommand(std::move(command));
        } else if (isOpen_.isRelease()) {
            auto command = std::make_unique<WindowOpenCommand>(&isOpen_, false);
            EditorController::getInstance()->pushCommand(std::move(command));
        }
    }
}

void Editor::Window::UpdateFocusAndOpenState() {
    if (isFocused_.isChanged()) {
        if (isFocused_.isTrigger()) {
            auto command = std::make_unique<WindowFocusCommand>(title_, &isOpen_, true);
            EditorController::getInstance()->pushCommand(std::move(command));
        } else if (isFocused_.isRelease()) {
            auto command = std::make_unique<WindowFocusCommand>(title_, &isOpen_, false);
            EditorController::getInstance()->pushCommand(std::move(command));
        }
    }
    if (isOpen_.isChanged()) {
        if (isOpen_.isTrigger()) {
            auto command = std::make_unique<WindowOpenCommand>(&isOpen_, true);
            EditorController::getInstance()->pushCommand(std::move(command));
        } else if (isOpen_.isRelease()) {
            auto command = std::make_unique<WindowOpenCommand>(&isOpen_, false);
            EditorController::getInstance()->pushCommand(std::move(command));
        }
    }
}

void Editor::Window::DrawGui() {
    ///=================================================================================================
    // Main DockSpace Window
    ///=================================================================================================
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (isMaximized_) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    } else {
        const ImGuiStyle& style = ImGui::GetStyle();
        float defaultRounding   = style.WindowRounding;
        float defaultBorderSize = style.WindowBorderSize;
        ImGui::SetNextWindowPos(windowPos_.toImVec2());
        ImGui::SetNextWindowSize(windowSize_.toImVec2());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, defaultRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, defaultBorderSize);
    }

    bool isOpen = isOpen_.current();
    ImGui::PopStyleVar(2);
    if (ImGui::Begin(title_.c_str(), &isOpen, window_flags)) {
        windowPos_  = ImGui::GetWindowPos();
        windowSize_ = ImGui::GetWindowSize();

        // DockSpaceを作成
        //! TODO : area Windowは 親ウィンドウにだけ Dockするようにしたい.
        ImGuiID dockspace_id = ImGui::GetID(title_.c_str());
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        // メニューバーの描画
        if (ImGui::BeginMenuBar()) {
            for (auto& [name, menu] : menus_) {
                if (menu) {
                    bool menuIsOpen = menu->isOpen().current();
                    if (ImGui::BeginMenu(name.c_str(), menuIsOpen)) {
                        menu->setOpen(true);
                        menu->DrawGui();
                        ImGui::EndMenu();
                    }
                    menu->setOpen(menuIsOpen);
                }
            }
            ImGui::EndMenuBar();
        }

        for (auto& [name, area] : areas_) {
            if (area) {
                area->DrawGui();
            }
        }
    }
    isOpen_.set(isOpen);
    isFocused_.set(ImGui::IsWindowFocused());

    ImGui::End();
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
    windowOpenState_->set(to_);
    windowOpenState_->sync();
}

void WindowOpenCommand::Undo() {
    if (!windowOpenState_) {
        LOG_ERROR("WindowOpenCommand: Window is null.");
        return;
    }
    windowOpenState_->set(!to_);
    windowOpenState_->sync();
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
    windowOpenState_->set(to_);
    windowOpenState_->sync();
}

void WindowFocusCommand::Undo() {
    if (!windowOpenState_) {
        LOG_ERROR("WindowFocusCommand: Window is null.");
        return;
    }
    // ウィンドウのフォーカス状態を元に戻す
    windowOpenState_->set(!to_);
    windowOpenState_->sync();
}
