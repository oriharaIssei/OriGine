#pragma once

/// stl
#include <list>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>

/// engine
#include "module/editor/IEditor.h"

/// util
#include <util/nameof.h>

///=============================================================================
/// EditorGroup (Engine の 汎用的な editor郡)
///=============================================================================
class EditorGroup {
    friend class SceneManager;

public:
    static EditorGroup* getInstance();

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

private:
    EditorGroup();
    EditorGroup(const EditorGroup&) = delete;
    ~EditorGroup();
    EditorGroup& operator=(const EditorGroup&) = delete;
    EditorGroup& operator=(EditorGroup&&)      = delete;

private:
    void ExecuteCommandRequests();

private:
    std::unordered_map<std::string, std::unique_ptr<IEditor>> editors_;
    std::unordered_map<IEditor*, bool> editorActive_;

    std::queue<std::unique_ptr<IEditCommand>> commandRequestQueue_;
    std::list<std::unique_ptr<IEditCommand>> commandHistory_;
    std::list<std::unique_ptr<IEditCommand>>::iterator currentCommandItr_ = commandHistory_.end();

public:
    template <IsEditor EditorClass>
    void addEditor(std::unique_ptr<EditorClass>&& editor) {
        std::string name                    = nameof<EditorClass>();
        editors_[name]                      = std::move(editor);
        editorActive_[editors_[name].get()] = false;
    }

    void pushCommand(std::unique_ptr<IEditCommand>&& command) {
        commandRequestQueue_.push(std::move(command));
    }
};
