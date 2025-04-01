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
/// EngineEditor (Engine の 汎用的な editor郡)
///=============================================================================
class EngineEditor {
public:
    static EngineEditor* getInstance();

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
    EngineEditor();
    EngineEditor(const EngineEditor&) = delete;
    ~EngineEditor();
    EngineEditor& operator=(const EngineEditor&) = delete;
    EngineEditor& operator=(EngineEditor&&)      = delete;

private:
    void ExecuteCommandRequests();

private:
    bool isActive_ = true;

    std::unordered_map<std::string, std::unique_ptr<IEditor>> editors_;
    std::unordered_map<IEditor*, bool> editorActive_;

    std::queue<std::unique_ptr<IEditCommand>> commandRequestQueue_;
    std::list<std::unique_ptr<IEditCommand>> commandHistory_;
    std::list<std::unique_ptr<IEditCommand>>::iterator currentCommandItr_ = commandHistory_.end();

public:
    void setActive(bool active) { isActive_ = active; }
    bool isActive() const { return isActive_; }

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
