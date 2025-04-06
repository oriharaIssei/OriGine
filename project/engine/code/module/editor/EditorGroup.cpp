#include "EditorGroup.h"

#ifdef _DEBUG
/// engine
// module
#include "camera/CameraManager.h"
#include "component/material/light/LightManager.h"
#include "component/material/Material.h"
#include "sceneManager/SceneManager.h"
// scene
#include "iScene/IScene.h"
// lib
#include "input/Input.h"

// externals
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

EditorGroup::EditorGroup() {
    // Constructor implementation
}

EditorGroup::~EditorGroup() {
    // Destructor implementation
}

void EditorGroup::ExecuteCommandRequests() {
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

        // coomand を history に追加
        commandHistory_.erase(currentCommandItr_, commandHistory_.end());
        commandHistory_.push_back(std::move(command));
        currentCommandItr_ = commandHistory_.end();
    }
}

EditorGroup* EditorGroup::getInstance() {
    static EditorGroup instance;
    return &instance;
}

void EditorGroup::Initialize() {
    ///============================= Editor の初期化 ========================================
    for (auto& [editorName, editor] : editors_) {
        editor->Initialize();
    }
}

void EditorGroup::Update() {
    ///-------------------------------------------------------------------------------------------------
    // Editors Update
    ///-------------------------------------------------------------------------------------------------
    for (auto& [name, editor] : editors_) {
        if (editorActive_[editor.get()]) {
            editor->Update();
        }
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

void EditorGroup::Finalize() {
    for (auto& [editorName, editor] : editors_) {
        editor->Finalize();
    }
}
#endif // _DEBUG
