#include "EditorController.h"

#ifdef _DEBUG
/// engine

#include "input/InputManager.h"

// externals
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

using namespace OriGine;

EditorController::EditorController() {}
EditorController::~EditorController() {}

void EditorController::Undo() {
    if (currentCommandItr_ != commandHistory_.begin()) {
        --currentCommandItr_;
        (*currentCommandItr_)->Undo();
    }
}

void EditorController::Redo() {
    if (currentCommandItr_ != commandHistory_.end()) {
        (*currentCommandItr_)->Execute();
        ++currentCommandItr_;
    }
}

void EditorController::ClearCommandHistory() {
    commandHistory_.clear();
    currentCommandItr_ = commandHistory_.end();
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

EditorController* EditorController::GetInstance() {
    static EditorController instance;
    return &instance;
}

void EditorController::Initialize() {
    ///============================= Editor の初期化 ========================================
    for (auto& [editorName, editor] : editorWindows_) {
        if (!editor) {
            LOG_ERROR("Editor with name '{}' is nullptr.", editorName);
            continue;
        }
        editor->Initialize();
    }
    ///============================= メインメニューの初期化 ========================================
    for (auto& [menuName, menu] : mainMenus_) {
        if (!menu) {
            LOG_ERROR("Menu with name '{}' is nullptr.", menuName);
            continue;
        }
        menu->Initialize();
    }
}

void EditorController::Update() {
    ///-------------------------------------------------------------------------------------------------
    // MainMenu Update
    ///-------------------------------------------------------------------------------------------------
    ImGui::BeginMainMenuBar();
    for (auto& [menuName, menu] : mainMenus_) {
        if (!menu) {
            LOG_ERROR("Menu with name '{}' is nullptr.", menuName);
            continue;
        }
        if (ImGui::BeginMenu(menu->GetName().c_str())) {
            menu->DrawGui();
            ImGui::EndMenu();
        }
    }
    ImGui::EndMainMenuBar();

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
    // 直接Inputを参照
    KeyboardInput* keyInput = InputManager::GetInstance()->GetKeyboard();
    if (keyInput->IsPress(DIK_LCONTROL)) {
        if (keyInput->IsPress(DIK_LSHIFT)) {
            // SHIFT あり
            if (keyInput->IsTrigger(DIK_Z)) {
                Redo();
            }
        } else {
            // SHIFT なし
            if (keyInput->IsTrigger(DIK_Z)) {
                Undo();
            }
            if (keyInput->IsTrigger(DIK_Y)) {
                Redo();
            }
        }
    }
}

void EditorController::Finalize() {
    GlobalVariables::GetInstance()->SaveFile(defaultSerializeSceneName_, defaultSerializeGroupName_);

    for (auto& [editorName, editor] : editorWindows_) {
        editor->Finalize();
    }

    ClearCommandHistory();
}
#endif // _DEBUG
