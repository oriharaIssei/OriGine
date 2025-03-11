#include "SceneManager.h"

// Interface
#include "iScene/IScene.h"

/// engine
#include "ECS/ECSManager.h"
#include "Engine.h"
// directX12Object
#include "directX12/DxRtvArrayManager.h"
#include "directX12/DxSrvArrayManager.h"
#include "directX12/RenderTexture.h"
// module
#include "camera/CameraManager.h"

/// math
#include "math/Vector2.h"
#include "math/Vector4.h"

SceneManager* SceneManager::getInstance() {
    static SceneManager instance;
    return &instance;
}

void SceneManager::Init() {
    sceneViewRtvArray_ = DxRtvArrayManager::getInstance()->Create(1);
    sceneViewSrvArray_ = DxSrvArrayManager::getInstance()->Create(1);

    ecsManager_ = EntityComponentSystemManager::getInstance();
    ecsManager_->Init();

    sceneView_ = std::make_unique<RenderTexture>(Engine::getInstance()->getDxCommand(), sceneViewRtvArray_.get(), sceneViewSrvArray_.get());
    /// TODO
    // fix MagicNumber
    sceneView_->Init({1280.0f, 720.0f}, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, {0.0f, 0.0f, 0.0f, 1.0f});
}

void SceneManager::Finalize() {
    currentScene_->Finalize();
    scenes_.clear();

    sceneView_->Finalize();

    sceneViewRtvArray_->Finalize();
    sceneViewSrvArray_->Finalize();
}

void SceneManager::Update() {
    ecsManager_->Run();

    CameraManager::getInstance()->DataConvertToBuffer();
}

void SceneManager::Draw() {
    Engine::getInstance()->ScreenPreDraw();
    sceneView_->DrawTexture();
    Engine::getInstance()->ScreenPostDraw();
}

SceneManager::SceneManager() {}

SceneManager::~SceneManager() {}

void SceneManager::addScene(
    const std::string& name,
    std::function<std::unique_ptr<IScene>()> _sceneMakeFunc) {
    sceneIndexs_[name] = static_cast<int32_t>(scenes_.size());
    scenes_.push_back(std::move(_sceneMakeFunc));
}

void SceneManager::changeScene(const std::string& name) {
    if (currentScene_) {
        currentScene_->Finalize();
    }

    currentScene_ = scenes_[sceneIndexs_[name]]();
    currentScene_->Init();
}
