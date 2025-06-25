#include "EditorController.h"

#ifdef _DEBUG
/// engine
// lib
#include "input/Input.h"

// externals
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

EditorController::EditorController() {
    // Constructor implementation
}

EditorController::~EditorController() {
    // Destructor implementation
}

void EditorController::ExecuteCommandRequests() {
    while (true) {
        // request がなくなったら終了
        if (commandRequestQueue_.empty()) {
            break;
        }

        // command を取り出す
        auto command = std::move(commandRequestQueue_.front());
        commandRequestQueue_.pop();
        if (command == nullptr) {
            continue;
        }

        // command を実行
        command->Execute();

        // command を history に追加
        commandHistory_.erase(currentCommandItr_, commandHistory_.end());
        commandHistory_.push_back(std::move(command));
        currentCommandItr_ = commandHistory_.end();
    }
}

EditorController* EditorController::getInstance() {
    static EditorController instance;
    return &instance;
}

void EditorController::Initialize() {
    ///============================= Editor の初期化 ========================================
    for (auto& [editorName, editor] : editorWindows_) {
        editor->Initialize();
    }
}

void EditorController::Update() {
    ///-------------------------------------------------------------------------------------------------
    // Editors Update
    ///-------------------------------------------------------------------------------------------------
    for (auto& [name, editor] : editorWindows_) {
            editor->DrawGui();
    }

    ///-------------------------------------------------------------------------------------------------
    // Commandの実行
    ///-------------------------------------------------------------------------------------------------
    ExecuteCommandRequests();

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

void EditorController::Finalize() {
    GlobalVariables::getInstance()->SaveFile(defaultSerializeSceneName_, defaultSerializeGroupName_);
    for (auto& [editorName, editor] : editorWindows_) {
        editor->Finalize();
    }

    clearCommandHistory();
}
#endif // _DEBUG
