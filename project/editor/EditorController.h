#pragma once

/// stl
#include <functional>
#include <list>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>

/// engine
#include "IEditor.h"

/// lib
#include "globalVariables/SerializedField.h"

/// util
#include <util/nameof.h>

///=============================================================================
/// EditorController (Engine の 汎用的な editor郡)
///=============================================================================
class EditorController {
    friend class SceneManager;

public:
    static EditorController* getInstance();

    void Initialize();
    void Update();
    void Finalize();

    void Undo() {
        if (currentCommandItr_ != commandHistory_.begin()) {
            --currentCommandItr_;
            (*currentCommandItr_)->Undo();
        }
    }

    void Redo() {
        if (currentCommandItr_ != commandHistory_.end()) {
            (*currentCommandItr_)->Execute();
            ++currentCommandItr_;
        }
    }

    void clearCommandHistory() {
        commandHistory_.clear();
        currentCommandItr_ = commandHistory_.end();
    }

private:
    EditorController();
    EditorController(const EditorController&) = delete;
    ~EditorController();
    EditorController& operator=(const EditorController&) = delete;
    EditorController& operator=(EditorController&&)      = delete;

private:
    void ExecuteCommandRequests();

private:
    std::unordered_map<std::string, std::unique_ptr<Editor::Window>> editorWindows_;
    std::unordered_map<std::string, std::unique_ptr<Editor::Menu>> mainMenus_; // メインメニューのマップ

    const std::string defaultSerializeSceneName_ = "Settings";
    const std::string defaultSerializeGroupName_ = "Editor";
    std::unordered_map<Editor::Window*, SerializedField<bool>> editorActivity_;

    std::queue<std::unique_ptr<IEditCommand>> commandRequestQueue_;
    std::list<std::unique_ptr<IEditCommand>> commandHistory_;
    std::list<std::unique_ptr<IEditCommand>>::iterator currentCommandItr_ = commandHistory_.end();

public:
    template <EditorWindow EditorWindowClass>
    EditorWindowClass* getWindow() {
        std::string windowName = nameof<EditorWindowClass>();
        auto itr               = editorWindows_.find(windowName);
        if (itr == editorWindows_.end()) {
            return nullptr;
        }
        return dynamic_cast<EditorWindowClass*>(itr->second.get());
    }

    template <EditorWindow EditorWindowClass>
    void addEditor(std::unique_ptr<EditorWindowClass>&& editor) {
        std::string name                            = nameof<EditorWindowClass>();
        editorWindows_[name]                        = std::move(editor);
        editorActivity_[editorWindows_[name].get()] = SerializedField<bool>(defaultSerializeSceneName_, defaultSerializeGroupName_, name, false);
    }

    template <EditorMenu EditorMenuClass>
    void addMainMenu(std::unique_ptr<EditorMenuClass>&& menu) {
        std::string name = nameof<EditorMenuClass>();
        if (mainMenus_.find(name) != mainMenus_.end()) {
            LOG_WARN("Main menu with name '{}' already exists.", name);
            return;
        }
        mainMenus_[name] = std::move(menu);
    }
    template <EditorMenu EditorMenuClass>
    EditorMenuClass* getMainMenu() {
        std::string name = nameof<EditorMenuClass>();
        auto itr         = mainMenus_.find(name);
        if (itr == mainMenus_.end()) {
            return nullptr;
        }
        return dynamic_cast<EditorMenuClass*>(itr->second.get());
    }

    void pushCommand(std::unique_ptr<IEditCommand>&& command) {
        commandRequestQueue_.push(std::move(command));
    }
};
