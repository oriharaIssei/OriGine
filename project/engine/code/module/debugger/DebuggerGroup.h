#pragma once

/// stl
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <string>

/// engine
#include "module/Debugger/IDebugger.h"

// lib
#include "globalVariables/SerializedField.h"

/// <summary>
/// デバッグ中にしよすることのできる 機能 をまとめるクラス
/// </summary>
class DebuggerGroup {
    friend class SceneManager;

public:
    static DebuggerGroup* getInstance();
    void Initialize();
    void Update();
    void Finalize();

    DebuggerGroup(const DebuggerGroup&)            = delete;
    DebuggerGroup& operator=(const DebuggerGroup&) = delete;
    DebuggerGroup& operator=(DebuggerGroup&&)      = delete;

private:
    DebuggerGroup();
    ~DebuggerGroup();

    void ExecuteCommandRequests();

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
    std::map<std::string, std::unique_ptr<IDebugger>> debuggers_;
    std::map<std::string, bool> debuggersActive_;

    std::queue<std::unique_ptr<IDebugCommand>> commandRequestQueue_;
    std::list<std::unique_ptr<IDebugCommand>> commandHistory_;
    std::list<std::unique_ptr<IDebugCommand>>::iterator currentCommandItr_ = commandHistory_.end();

public:
    template <IsDebuggerGroup DebuggerGroupClass>
    void addDebugger(std::unique_ptr<DebuggerGroupClass>&& editor) {
        std::string name                    = nameof<DebuggerGroupClass>();
        debuggers_[name]                    = std::move(editor);
        debuggersActive_[name] = false;
    }

    void pushCommand(std::unique_ptr<IDebugCommand>&& command) {
        commandRequestQueue_.push(std::move(command));
    }
};
