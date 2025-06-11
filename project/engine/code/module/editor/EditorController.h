#pragma once

/// stl
#include <functional>
#include <list>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>

/// engine
#include "module/editor/IEditor.h"

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
    std::unordered_map<std::string, std::unique_ptr<IEditor>> editors_;
    const std::string defaultSerializeSceneName_ = "Settings";
    const std::string defaultSerializeGroupName_ = "Editor";
    std::unordered_map<IEditor*, SerializedField<bool>> editorActivity_;

    std::queue<std::unique_ptr<IEditCommand>> commandRequestQueue_;
    std::list<std::unique_ptr<IEditCommand>> commandHistory_;
    std::list<std::unique_ptr<IEditCommand>>::iterator currentCommandItr_ = commandHistory_.end();

public:
    template <IsEditor EditorClass>
    void addEditor(std::unique_ptr<EditorClass>&& editor) {
        std::string name                      = nameof<EditorClass>();
        editors_[name]                        = std::move(editor);
        editorActivity_[editors_[name].get()] = SerializedField<bool>(defaultSerializeSceneName_, defaultSerializeGroupName_, name, false);
    }


    void pushCommand(std::unique_ptr<IEditCommand>&& command) {
        commandRequestQueue_.push(std::move(command));
    }
};
