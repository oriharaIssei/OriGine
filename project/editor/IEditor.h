#pragma once

#ifdef _DEBUG

/// stl
#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/// editor
#include <imgui/imgui.h>
// logger
#include "logger/Logger.h"

/// util
#include "util/DiffValue.h"

/// math
#include <Vector2.h>

namespace Editor {
// Blenderを参考
// https://colorful-pico.net/introduction-to-addon-development-in-blender/2.7/html/chapter_99/00_Glossary.html
// https://docs.blender.org/manual/ja/dev/interface/window_system/index.html

/// <summary>
/// GuiWindowの最小単位.ボタンやテキストフィールドなどのUI要素を持つところ.
/// </summary>
class Region {
public:
    Region(const std::string& _name) : name_(_name) {}
    virtual ~Region() {}
    virtual void Initialize() = 0;
    virtual void DrawGui()    = 0;
    virtual void Finalize()   = 0;

protected:
    std::string name_;

public:
    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }
};

/// <summary>
/// MenuItem.メニューの項目を表す.
/// </summary>
class MenuItem {
public:
    MenuItem(const std::string& _name) : name_(_name) {}
    virtual ~MenuItem()       = default;
    virtual void Initialize() = 0;
    virtual void DrawGui()    = 0;
    virtual void Finalize()   = 0;

protected:
    std::string name_           = "Unknown"; // MenuItemの名前
    DiffValue<bool> isSelected_ = false; // MenuItemが開いているかどうか
public:
    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

    DiffValue<bool> isSelected() const { return isSelected_; }
    DiffValue<bool>& isSelected() { return isSelected_; }

    void setSelected(bool _selected, bool _sync = false) {
        isSelected_.set(_selected);
        if (_sync) {
            isSelected_.sync();
        }
    }
};

/// <summary>
/// Menu.メニューアイテムのコンテナ.
/// </summary>
class Menu {
public:
    Menu(const std::string& _name) : name_(_name) {}
    virtual ~Menu() {}

    virtual void Initialize() = 0;
    virtual void DrawGui();
    virtual void Finalize() = 0;

protected:
    std::string name_          = "Unknown"; // Menuの名前
    DiffValue<bool> isOpen_    = false; // Menuが開いているかどうか
    DiffValue<bool> isFocused_ = false; // Menuがフォーカスされているかどうか
    std::unordered_map<std::string, std::shared_ptr<MenuItem>> menuItems_; // Menuに含まれるMenuItem
public:
    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

    DiffValue<bool> isOpen() const { return isOpen_; }
    DiffValue<bool>& isOpenRef() { return isOpen_; }
    void setOpen(bool open, bool sync = false) {
        isOpen_.set(open);
        if (sync) {
            isOpen_.sync();
        }
    }

    DiffValue<bool> isFocused() const { return isFocused_; }
    void setFocused(bool focused, bool sync = false) {
        isFocused_.set(focused);
        if (sync) {
            isFocused_.sync();
        }
    }

    const std::unordered_map<std::string, std::shared_ptr<MenuItem>>& getMenuItems() const { return menuItems_; }
    std::unordered_map<std::string, std::shared_ptr<MenuItem>>& getMenuItemsRef() { return menuItems_; }
    void addMenuItem(std::shared_ptr<MenuItem> item, bool _isInit = true) {
        if (!item) {
            return; // nullptrチェック
        }
        if (menuItems_.find(item->getName()) != menuItems_.end()) {
            LOG_INFO("MenuItem with name '{}' already exists in this menu.", item->getName());
            return;
        }

        if (_isInit) {
            item->Initialize();
        }

        menuItems_[item->getName()] = item;
    }
};

/// <summary>
/// View,特定の空間を表示するクラス.
/// </summary>
class Area {
public:
    Area(const std::string& _name) : name_(_name) {}
    virtual ~Area() {}

    virtual void Initialize() {};
    virtual void DrawGui();
    virtual void Finalize();

protected:
    void UpdateFocusAndOpenState();

protected:
    std::string name_ = "Unknown";
    Vec2f areaSize_   = Vec2f(1280.f, 720.f); // Areaのサイズ

    DiffValue<bool> isOpen_    = true; // Areaが開いているかどうか
    DiffValue<bool> isFocused_ = false; // Areaがフォーカスされているかどうか

    std::unordered_map<std::string, std::shared_ptr<Region>> regions_; // Areaに含まれるRegion
public:
    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

    const DiffValue<bool>& isOpen() const { return isOpen_; }
    void setOpen(bool _open, bool _sync = false) {
        isOpen_.set(_open);
        if (_sync) {
            isOpen_.sync();
        }
    }
    const DiffValue<bool>& isFocused() const { return isFocused_; }
    void setFocused(bool _focused, bool _sync = false) {
        isFocused_.set(_focused);
        if (_sync) {
            isFocused_.sync();
        }
    }

    const Vec2f& getAreaSize() const { return areaSize_; }
    void setAreaSize(const Vec2f& size) { areaSize_ = size; }

    std::unordered_map<std::string, std::shared_ptr<Region>>& getRegions() { return regions_; }
    void addRegion(std::shared_ptr<Region> region, bool _isInit = true) {
        if (!region) {
            return; // nullptrチェック
        }
        if (regions_.find(region->getName()) != regions_.end()) {
            LOG_INFO("Region with name '{}' already exists in this area.", region->getName());
            return;
        }
        if (_isInit) {
            region->Initialize();
        }
        regions_[region->getName()] = region;
    }
    void removeRegion(const std::string& regionName) {
        auto it = regions_.find(regionName);
        if (it != regions_.end()) {
            regions_.erase(it);
        } else {
            LOG_INFO("Region with name '{}' not found in this area.", regionName);
        }
    }
    void clearRegions() {
        regions_.clear();
    }
};

/// <summary>
/// Window.1画面の単位.
/// </summary>
class Window {
public:
    Window(const std::string& _title)
        : title_(_title){}
    virtual ~Window() {};

    virtual void Initialize() = 0;
    virtual void DrawGui();
    virtual void Finalize();

protected:
    void UpdateFocusAndOpenState();

protected:
    std::string title_;
    DiffValue<bool> isOpen_    = true;
    DiffValue<bool> isFocused_ = false; // Windowがフォーカスされているかどうか

    bool initializedDockSpace_ = false; // DockSpaceの初期化が完了しているかどうか

    /// Window
    ImGuiWindowFlags windowFlags_ =
        ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGuiDockNodeFlags dockFlags_ = ImGuiDockNodeFlags_None; // DockSpaceのフラグ

    Vec2f windowSize_             = Vec2f(1280.f, 720.f); // Windowのサイズ
    Vec2f windowPos_              = Vec2f(0.f, 0.f); // Windowの位置
    bool isMaximized_             = false; // Windowが最大化されているかどうか
    bool isMinimized_             = false; // Windowが最小化されているかどうか

    std::unordered_map<std::string, std::shared_ptr<Area>> areas_; // Windowに含まれるArea
    std::unordered_map<std::string, std::shared_ptr<Menu>> menus_; // Windowに含まれるMenu
public:
    const std::string& getTitle() const { return title_; }
    void setTitle(const std::string& title) { title_ = title; }

    bool isOpen() const { return isOpen_.current(); }
    DiffValue<bool>& isOpenRef() { return isOpen_; }
    void setOpen(bool open, bool sync = false) {
        isOpen_ = open;
        if (sync) {
            isOpen_.sync();
        }
    }

    bool isFocused() const { return isFocused_.current(); }
    DiffValue<bool>& isFocusedRef() { return isFocused_; }
    void setFocused(bool focused, bool sync = false) {
        isFocused_.set(focused);
        if (sync) {
            isFocused_.sync();
        }
    }

    const std::unordered_map<std::string, std::shared_ptr<Area>>& getAreas() const { return areas_; }
    std::unordered_map<std::string, std::shared_ptr<Area>>& getAreasRef() { return areas_; }
    std::shared_ptr<Area> getArea(const std::string& areaName) const {
        auto it = areas_.find(areaName);
        if (it != areas_.end()) {
            return it->second;
        }
        LOG_INFO("Area with name '{}' not found in this window.", areaName);
        return nullptr;
    }
    void addArea(std::shared_ptr<Area> area, bool _isInit = true) {
        if (!area) {
            return; // nullptrチェック
        }
        if (areas_.find(area->getName()) != areas_.end()) {
            LOG_INFO("Area with name '{}' already exists in this window.", area->getName());
            return;
        }

        if (_isInit) {
            area->Initialize();
        }

        areas_[area->getName()] = area;
    }
    void removeArea(const std::string& areaName) {
        auto it = areas_.find(areaName);
        if (it != areas_.end()) {
            areas_.erase(it);
        } else {
            LOG_INFO("Area with name '{}' not found in this window.", areaName);
        }
    }
    void clearAreas() {
        areas_.clear();
    }

    const std::unordered_map<std::string, std::shared_ptr<Menu>>& getMenus() const { return menus_; }
    std::unordered_map<std::string, std::shared_ptr<Menu>>& getMenusRef() { return menus_; }
    void addMenu(std::shared_ptr<Menu> menu, bool _isInit = true) {
        if (!menu) {
            return; // nullptrチェック
        }
        if (menus_.find(menu->getName()) != menus_.end()) {
            LOG_WARN("Menu with name '{}' already exists in this window.", menu->getName());
            return;
        }
        if (_isInit) {
            menu->Initialize();
        }
        menus_[menu->getName()] = menu;
    }
    void clearMenus() {
        menus_.clear();
    }
};

};

Vec2f ConvertMouseToGuiWindow(const Vec2f& _mousePos, const Vec2f& _guiWindowLT, const ImVec2& _guiWindowSize, const Vec2f& _originalResolution);

template <typename EditorWindowClass>
concept EditorWindow = std::derived_from<EditorWindowClass, Editor::Window>;

template <typename EditorMenuClass>
concept EditorMenu = std::derived_from<EditorMenuClass, Editor::Menu>;

#pragma region DefaultCommand

class IEditCommand {
public:
    IEditCommand() {}
    virtual ~IEditCommand() {}

    /// @brief コマンド実行
    virtual void Execute() = 0;

    /// @brief コマンドの取り消し
    virtual void Undo() { Execute(); };
};

class CommandCombo
    : public IEditCommand {
public:
    CommandCombo() {}
    ~CommandCombo() {}

    void Execute() {
        for (auto& command : commands_) {
            command->Execute();
        }
        if (funcOnAfterCommand_) {
            funcOnAfterCommand_();
        }
    }
    void Undo() {
        for (auto& command : commands_) {
            command->Undo();
        }
        if (funcOnAfterCommand_) {
            funcOnAfterCommand_();
        }
    }

private:
    std::vector<std::shared_ptr<IEditCommand>> commands_;
    std::function<void()> funcOnAfterCommand_;
    std::function<void()> funcOnAfterUndoCommand_;

public:
    void addCommand(std::shared_ptr<IEditCommand> command) {
        if (!command) {
            return; // nullptrチェック
        }
        commands_.emplace_back(command);
    }
    void setFuncOnAfterCommand(std::function<void()> func, bool _syncFuncOnAfterUndoCommand = false) {
        funcOnAfterCommand_ = func;
        if (_syncFuncOnAfterUndoCommand) {
            funcOnAfterUndoCommand_ = func;
        }
    }
    void setFuncOnAfterUndoCommand(std::function<void()> func) {
        funcOnAfterUndoCommand_ = func;
    }
};

template <typename T>
class SetterCommand
    : public IEditCommand {
public:
    SetterCommand(T* _target, const T& _to, std::function<void(T*)> _funcOnAfterExecute = nullptr, bool _syncFuncOnAfter = true, std::function<void(T*)> _funcOnAfterUndo = nullptr)
        : target(_target), to(_to), funcOnAfterExecute_(_funcOnAfterExecute) {
        if (_syncFuncOnAfter) {
            funcOnAfterUndoExecute_ = _funcOnAfterExecute;
        } else {
            funcOnAfterUndoExecute_ = _funcOnAfterUndo;
        }
        from = _target ? *_target : T(); // 初期値を設定
    }
    SetterCommand(T* _target, const T& _to, const T& _from, std::function<void(T*)> _funcOnAfterExecute = nullptr, bool _syncFuncOnAfter = true, std::function<void(T*)> _funcOnAfterUndo = nullptr)
        : target(_target), to(_to), from(_from), funcOnAfterExecute_(_funcOnAfterExecute) {
        if (_syncFuncOnAfter) {
            funcOnAfterUndoExecute_ = _funcOnAfterExecute;
        } else {
            funcOnAfterUndoExecute_ = _funcOnAfterUndo;
        }
    }

    void Execute() override {
        if (!target) {
            return;
        }
        *target = to;

        if (funcOnAfterExecute_) {
            funcOnAfterExecute_(target);
        }
    }
    void Undo() override {
        if (!target) {
            return;
        }
        *target = from;
        if (funcOnAfterUndoExecute_) {
            funcOnAfterUndoExecute_(target);
        }
    }

private:
    std::function<void(T*)> funcOnAfterExecute_;
    std::function<void(T*)> funcOnAfterUndoExecute_;
    T* target;
    T to;
    T from;
};

template <typename T>
concept HasPushBack = requires(T a, typename T::value_type v) {
    { a.push_back(v) };
};
template <typename T>
concept HasPushFront = requires(T a, typename T::value_type v) {
    { a.push_front(v) };
};

template <typename T>
concept HasClear = requires(T a) {
    { a.clear() };
};
template <typename T>
concept HasErase = requires(T a, typename T::iterator it) {
    { a.erase(it) };
};

template <typename Container>
    requires HasPushBack<Container>
class AddElementCommand : public IEditCommand {
public:
    using ContainerType = Container;
    using Iterator      = typename Container::iterator;
    using ValueType     = typename Container::value_type;

    AddElementCommand(Container* container, const ValueType& value)
        : container_(container), value_(value) {}

    void Execute() override {
        container_->push_back(value_);
    }

    void Undo() override {
        if (!container_->empty()) {
            container_->pop_back();
        }
    }

private:
    Container* container_;
    ValueType value_;
};

// erase対応コンテナ用コマンド
template <typename Container>
    requires HasErase<Container>
class EraseElementCommand : public IEditCommand {
public:
    using ContainerType = Container;
    using Iterator      = typename Container::iterator;

    EraseElementCommand(Container* container, Iterator it)
        : container_(container), it_(it) {}

    void Execute() override {
        if (container_ && it_ != container_->end()) {
            backup_ = *it_;
            it_     = container_->erase(it_);
        }
    }

    void Undo() override {
        it_ = container_->insert(it_, backup_);
    }

private:
    Container* container_;
    Iterator it_;
    typename Container::value_type backup_;
};

template <typename Container>
class SortCommand : public IEditCommand {
public:
    using ValueType = typename Container::value_type;
    using Compare   = std::function<bool(const ValueType&, const ValueType&)>;

    SortCommand(Container* container, Compare comp)
        : container_(container), comp_(comp) {}

    void Execute() override {
        // 元の順序を保存
        backup_.assign(container_->begin(), container_->end());
        std::sort(container_->begin(), container_->end(), comp_);
    }

    void Undo() override {
        // 元の順序に戻す
        if (container_) {
            container_->assign(backup_.begin(), backup_.end());
        }
    }

private:
    Container* container_;
    Compare comp_;
    std::vector<ValueType> backup_;
};

template <typename Container>
    requires HasClear<Container>
class ClearCommand : public IEditCommand {
public:
    using ValueType = typename Container::value_type;

    ClearCommand(Container* container)
        : container_(container) {}
    void Execute() override {
        // 元の順序を保存
        backup_.assign(container_->begin(), container_->end());
        container_->clear();
    }
    void Undo() override {
        // 元の順序に戻す
        if (container_) {
            container_->assign(backup_.begin(), backup_.end());
        }
    }

private:
    Container* container_;
    std::vector<typename Container::value_type> backup_;
};

class WindowOpenCommand
    : public IEditCommand {
public:
    WindowOpenCommand(DiffValue<bool>* _windowOpenState, bool _to = true);
    ~WindowOpenCommand() override;

    void Execute() override;
    void Undo() override;

private:
    DiffValue<bool>* windowOpenState_ = nullptr; // 開くウィンドウの状態へのポインタ

    bool to_ = true; // 開いた後の状態
};

class WindowFocusCommand
    : public IEditCommand {
public:
    WindowFocusCommand(const std::string& _windowName, DiffValue<bool>* _windowOpenState, bool _to = true);
    ~WindowFocusCommand() override;
    void Execute() override;
    void Undo() override;

private:
    std::string windowName_;
    DiffValue<bool>* windowOpenState_ = nullptr; // 開くウィンドウの状態へのポインタ

    bool to_ = true; // フォーカス後の状態
};
#pragma endregion DefaultCommand

#endif // _DEBUG
