#pragma once

#ifdef _DEBUG

#include "editor/IEditor.h"

/// editor
#include "editor/EditorController.h"

namespace OriGine {
namespace Editor {

/// <summary>
/// EditorWindowのメニュー
/// </summary>
class EditorWindowMenu
    : public Menu {
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
    : public MenuItem {
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
    auto window         = EditorController::GetInstance()->GetWindow<WindowType>();
    bool windowIsOpened = window->IsOpen();
    if (ImGui::MenuItem(name_.c_str(), nullptr, windowIsOpened, !windowIsOpened)) {
        window->SetOpen(!windowIsOpened);
        if (!windowIsOpened) {
            window->SetFocused(true);
        }
    }
}

} // namespace Editor
} // namespace OriGine

#endif // _DEBUG
