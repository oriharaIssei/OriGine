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
    keyboardInput_ = std::make_unique<KeyboardInput>();
    mouseInput_    = std::make_unique<MouseInput>();
    gamePadInput_  = std::make_unique<GamePadInput>();

    sceneManager_ = std::make_unique<SceneManager>();
    sceneManager_->Initialize(keyboardInput_.get(), mouseInput_.get(), gamePadInput_.get());

    replayPlayer_ = std::make_unique<ReplayPlayer>();

    // addArea
    addArea(std::make_unique<DebugReplayControlArea>(this));
    addArea(std::make_unique<DebugReplayViewArea>(this));

    // addMenu
    addMenu(std::make_unique<DebugReplayFileMenu>(this));

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

    // リプレイプレイヤーが有効 & 指定フレームにシーク成功時のみ実行
    if (replayPlayer_->getIsActive()) {

        size_t playerCurrentFrameIndex = replayPlayer_->getCurrentFrameIndex();

        if (playerCurrentFrameIndex == replayFrameIndex_) {
            return;
        }

        // 過去に遡る場合、シーンを最初からやり直す
        if (playerCurrentFrameIndex > replayFrameIndex_) {
            sceneManager_->changeScene(replayPlayer_->getStartSceneName());
            sceneManager_->executeSceneChange();

            // index の初期化
            playerCurrentFrameIndex = 0;
            // input の初期化
            keyboardInput_->clearKeyStates();
            mouseInput_->clearButtonStates();
            mouseInput_->resetWheelDelta();
            mouseInput_->resetPosition();
            gamePadInput_->clearButtonStates();
            gamePadInput_->clearStickStates();
            gamePadInput_->clearTriggerStates();

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
                Engine::getInstance()->setDeltaTime(deltaTime);

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

    bool isOpen = isOpen_.current();

    if (!isOpen) {
        isOpen_.set(isOpen);
        isFocused_.set(ImGui::IsWindowFocused());
        UpdateFocusAndOpenState();
        return;
    }

    auto sceneManager = parent_->getSceneManager();
    auto currentScene = sceneManager->getCurrentScene();
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
        if (areaSize_[Y] != guiWindowSize[Y]) {
            if (guiWindowSize[X] < 1.f) {
                guiWindowSize[X] = 1.f;
            }
            if (guiWindowSize[Y] < 1.f) {
                guiWindowSize[Y] = 1.f;
            }

            int32_t biggerIndex  = 0;
            int32_t smallerIndex = 0;
            if (guiWindowSize.x > guiWindowSize.y) {
                biggerIndex  = X;
                smallerIndex = Y;
            } else {
                biggerIndex  = Y;
                smallerIndex = X;
            }

            const Vec2f& sceneViewSize = currentScene->getSceneView()->getTextureSize();
            float aspectRatio          = sceneViewSize[smallerIndex] / sceneViewSize[biggerIndex];

            // aspect比を維持したままリサイズ
            areaSize_[biggerIndex]  = guiWindowSize[biggerIndex];
            areaSize_[smallerIndex] = areaSize_[biggerIndex] * aspectRatio;
            // シーンビューのリサイズ
            currentScene->getSceneView()->Resize(areaSize_);
            // guiWindowのリサイズ通知
            isResizing_ = true;
        }

        sceneManager->Render();
        ImGui::Image(reinterpret_cast<ImTextureID>(currentScene->getSceneView()->getBackBufferSrvHandle().ptr), areaSize_.toImVec2());
    }

    isOpen_.set(isOpen);
    isFocused_.set(ImGui::IsWindowFocused());
    UpdateFocusAndOpenState();

    ImGui::End();
}

#endif // _DEBUG
