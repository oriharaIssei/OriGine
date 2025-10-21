#pragma once

#ifdef _DEBUG

#include "editor/IEditor.h"

/// editor
#include "editor/EditorController.h"
#include "myGui/MyGui.h"

/// util
#include "util/nameof.h"

/// <summary>
/// EditorWindowのメニュー
/// </summary>
class EditorWindowMenu
    : public Editor::Menu {
public:
    EditorWindowMenu();
    ~EditorWindowMenu() override;
    void Initialize() override;
    void Finalize() override;
};

/// <summary>
/// ウィンドウを開くためのメニューアイテム
/// </summary>
/// <typeparam name="EditorWindowType"></typeparam>
template <EditorWindow EditorWindowType>
class WindowItem
    : public Editor::MenuItem {
public:
    using WindowType = EditorWindowType;

    WindowItem();
    ~WindowItem() override = default;
    void Initialize() override {}
    void DrawGui() override;
    void Finalize() override {}

private:
};

template <EditorWindow EditorWindowType>
inline WindowItem<EditorWindowType>::WindowItem() : Editor::MenuItem(nameof<WindowType>()) {}

template <EditorWindow EditorWindowType>
inline void WindowItem<EditorWindowType>::DrawGui() {
    auto window         = EditorController::getInstance()->getWindow<WindowType>();
    bool windowIsOpened = window->isOpen();
    if (ImGui::MenuItem(name_.c_str(), nullptr, windowIsOpened, !windowIsOpened)) {
        window->setOpen(!windowIsOpened);
        if (!windowIsOpened) {
            window->setFocused(true);
        }
    }
}
#endif // _DEBUG
