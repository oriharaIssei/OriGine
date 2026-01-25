#include "DebugReplayWindow.h"

#ifdef _DEBUG

/// engine
#include "scene/Scene.h"
// directX12
#include "directX12/RenderTexture.h"

// editor
#include "DebugReplayControlArea.h"
#include "DebugReplayMenu.h"

DebugReplayWindow::DebugReplayWindow() : Editor::Window(nameof<DebugReplayWindow>()) {}
DebugReplayWindow::~DebugReplayWindow() {}

void DebugReplayWindow::Initialize() {
    // input
    // ただの傀儡なので Initialize()しなくて良い
    keyboardInput_ = std::make_unique<OriGine::KeyboardInput>();
    mouseInput_    = std::make_unique<OriGine::MouseInput>();
    gamePadInput_  = std::make_unique<OriGine::GamepadInput>();

    sceneManager_ = std::make_unique<OriGine::SceneManager>();
    sceneManager_->Initialize(keyboardInput_.get(), mouseInput_.get(), gamePadInput_.get());

    replayPlayer_ = std::make_unique<OriGine::ReplayPlayer>();

    // AddArea
    AddArea(std::make_unique<DebugReplayControlArea>(this));
    AddArea(std::make_unique<DebugReplayViewArea>(this));

    // AddMenu
    AddMenu(std::make_unique<DebugReplayFileMenu>(this));

    // 最大化
    isMaximized_ = true;
}

void DebugReplayWindow::Finalize() {
    sceneManager_->Finalize();
    sceneManager_.reset();

    replayPlayer_.reset();

    keyboardInput_.reset();
    mouseInput_.reset();
    gamePadInput_.reset();
}

void DebugReplayWindow::DrawGui() {
    UpdateSceneManager();
    Editor::Window::DrawGui();
}

void DebugReplayWindow::UpdateSceneManager() {
    // load replay file 処理
    if (isLoadReplayFile_) {
        if (sceneManager_) {
            sceneManager_->ExecuteSceneChange();
        }
        isLoadReplayFile_ = false;
    }

    // リプレイプレイヤーが有効 & 指定フレームにシーク成功時のみ実行
    if (replayPlayer_->GetIsActive()) {

        size_t playerCurrentFrameIndex = replayPlayer_->GetCurrentFrameIndex();

        if (playerCurrentFrameIndex == replayFrameIndex_) {
            return;
        }

        // 過去に遡る場合、シーンを最初からやり直す
        if (playerCurrentFrameIndex > replayFrameIndex_) {
            sceneManager_->ChangeScene(replayPlayer_->GetStartSceneName());
            sceneManager_->ExecuteSceneChange();

            // index の初期化
            playerCurrentFrameIndex = 0;
            // input の初期化
            keyboardInput_->ClearHistory();
            mouseInput_->ClearHistory();
            gamePadInput_->ClearHistory();

            // シーク
            replayPlayer_->Seek(playerCurrentFrameIndex);
        }

        // 指定フレームまでシークして更新を行う
        if (playerCurrentFrameIndex < replayFrameIndex_) {
            while (playerCurrentFrameIndex < replayFrameIndex_) {
                ++playerCurrentFrameIndex;
                // シークして成功したら
                if (!replayPlayer_->Seek(playerCurrentFrameIndex)) {
                    break;
                }
                // 現在のフレームの入力を適用する
                float deltaTime = replayPlayer_->Apply(keyboardInput_.get(), mouseInput_.get(), gamePadInput_.get());

                // deltaTimeをセット
                OriGine::Engine::GetInstance()->SetDeltaTime(deltaTime);

                // シーン変更(あれば)
                if (sceneManager_->IsChangeScene()) {
                    sceneManager_->ExecuteSceneChange();
                }

                // シーンマネージャーを更新する
                sceneManager_->Update();
            }
        }
    }
}

DebugReplayViewArea::DebugReplayViewArea(DebugReplayWindow* _parent)
    : Editor::Area(nameof<DebugReplayViewArea>()), parent_(_parent) {}
DebugReplayViewArea::~DebugReplayViewArea() {}

void DebugReplayViewArea::Initialize() {}

void DebugReplayViewArea::DrawGui() {

    bool isOpen = isOpen_.Current();

    if (!isOpen) {
        isOpen_.Set(isOpen);
        isFocused_.Set(ImGui::IsWindowFocused());
        UpdateFocusAndOpenState();
        return;
    }

    auto sceneManager = parent_->GetSceneManager();
    auto currentScene = sceneManager->GetCurrentScene();
    if (!currentScene) {
        ImGui::Text("No Scene Loaded");
        return;
    }

    if (isResizing_) {
        ImGui::SetNextWindowSize(areaSize_.toImVec2());
        isResizing_ = false;
    }

    if (ImGui::Begin(name_.c_str(), &isOpen)) {
        // sizeが変わったら
        ImVec2 guiWindowSize = ImGui::GetWindowSize();
        if (areaSize_[OriGine::Y] != guiWindowSize[OriGine::Y]) {
            if (guiWindowSize[OriGine::X] < 1.f) {
                guiWindowSize[OriGine::X] = 1.f;
            }
            if (guiWindowSize[OriGine::Y] < 1.f) {
                guiWindowSize[OriGine::Y] = 1.f;
            }

            int32_t biggerIndex  = 0;
            int32_t smallerIndex = 0;
            if (guiWindowSize.x > guiWindowSize.y) {
                biggerIndex  = OriGine::X;
                smallerIndex = OriGine::Y;
            } else {
                biggerIndex  = OriGine::Y;
                smallerIndex = OriGine::X;
            }

            const OriGine::Vec2f& sceneViewSize = currentScene->GetSceneView()->GetTextureSize();
            float aspectRatio                   = sceneViewSize[smallerIndex] / sceneViewSize[biggerIndex];

            // aspect比を維持したままリサイズ
            areaSize_[biggerIndex]  = guiWindowSize[biggerIndex];
            areaSize_[smallerIndex] = areaSize_[biggerIndex] * aspectRatio;
            // シーンビューのリサイズ
            currentScene->GetSceneView()->Resize(areaSize_);
            // guiWindowのリサイズ通知
            isResizing_ = true;
        }

        // シーンマネージャーをレンダリングする
        sceneManager->Render();
        ImGui::Image(reinterpret_cast<ImTextureID>(currentScene->GetSceneView()->GetBackBufferSrvHandle().ptr), areaSize_.toImVec2());
    }

    isOpen_.Set(isOpen);
    isFocused_.Set(ImGui::IsWindowFocused());
    UpdateFocusAndOpenState();

    ImGui::End();
}

#endif // _DEBUG
