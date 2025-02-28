#pragma once

/// stl
#include <list>
#include <memory>
#include <string>
#include <unordered_map>

/// engine
#include "module/editor/IEditor.h"

///=============================================================================
/// EngineEditor (Engine の 汎用的な editor郡)
///=============================================================================
class EngineEditor {
public:
    static EngineEditor* getInstance();

    void Update();

    //void Undo() {
    //    if (currentCommand_ != editCommands_.begin()) {
    //        --currentCommand_;
    //        (*currentCommand_)->Undo();
    //    }
    //}

    //void Redo() {
    //    if (currentCommand_ != editCommands_.end()) {
    //        (*currentCommand_)->Execute();
    //        ++currentCommand_;
    //    }
    //}

private:
    EngineEditor();
    EngineEditor(const EngineEditor&) = delete;
    ~EngineEditor();
    EngineEditor& operator=(const EngineEditor&) = delete;
    EngineEditor& operator=(EngineEditor&&)      = delete;

private:
    bool isActive_ = true;

    std::unordered_map<std::string, std::unique_ptr<IEditor>> editors_;
    std::unordered_map<IEditor*, bool> editorActive_;

    std::list<std::unique_ptr<IEditCommand>> editCommands_;
    std::list<std::unique_ptr<IEditCommand>>::iterator currentCommand_ = editCommands_.end();

public:
    void SetActive(bool active) { isActive_ = active; }
    bool IsActive() const { return isActive_; }

    void addEditor(const std::string& name, std::unique_ptr<IEditor>&& editor);

    void AddCommand(std::unique_ptr<IEditCommand>&& command) {
        editCommands_.erase(currentCommand_, editCommands_.end());
        editCommands_.push_back(std::move(command));
        currentCommand_ = editCommands_.end();
    }
};
