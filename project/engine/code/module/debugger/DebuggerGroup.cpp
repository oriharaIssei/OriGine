#include "DebuggerGroup.h"

/// Engine
#include "input/Input.h"

DebuggerGroup::DebuggerGroup() {}

DebuggerGroup::~DebuggerGroup() {}

DebuggerGroup* DebuggerGroup::getInstance() {
    static DebuggerGroup instance;
    return &instance;
}

void DebuggerGroup::Initialize() {
    for (auto& [name, DebuggerGroup] : debuggers_) {
        DebuggerGroup->Initialize();
    }
}

void DebuggerGroup::Update() {
    for (auto& [name, DebuggerGroup] : debuggers_) {
        if (debuggersActive_[name]) {
            DebuggerGroup->Update();
        }
    }

    // Execute Command Requests
    ExecuteCommandRequests();
    // Undo/Redo
    // Undo Redo
    Input* input = Input::getInstance();
    if (input->isPressKey(DIK_LCONTROL)) {
        if (input->isPressKey(DIK_LSHIFT)) {
            // SHIFT あり
            if (input->isTriggerKey(DIK_Z)) {
                Redo();
            }
        } else {
            // SHIFT なし
            if (input->isTriggerKey(DIK_Z)) {
                Undo();
            }
            if (input->isTriggerKey(DIK_Y)) {
                Redo();
            }
        }
    }
}

void DebuggerGroup::ExecuteCommandRequests() {
    while (!commandRequestQueue_.empty()) {
        auto command = std::move(commandRequestQueue_.front());
        commandRequestQueue_.pop();
        command->Execute();
        commandHistory_.emplace_back(std::move(command));
        currentCommandItr_ = commandHistory_.end();
    }
}

void DebuggerGroup::Finalize() {
    for (auto& [name, DebuggerGroup] : debuggers_) {
        DebuggerGroup->Finalize();
    }
}
