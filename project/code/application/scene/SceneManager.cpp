#include "SceneManager.h"

#include "Engine.h"
#include "IScene.h"
#include "camera/CameraManager.h"
#include "engine/directX12/DxRtvArrayManager.h"
#include "engine/directX12/DxSrvArrayManager.h"
#include "engine/directX12/RenderTexture.h"
#include "effect/particle/manager/ParticleManager.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "sprite/SpriteCommon.h"

SceneManager* SceneManager::getInstance() {
    static SceneManager instance;
    return &instance;
}

void SceneManager::Init() {
    sceneViewRtvArray_ = DxRtvArrayManager::getInstance()->Create(1);
    sceneViewSrvArray_ = DxSrvArrayManager::getInstance()->Create(1);

    sceneView_ = std::make_unique<RenderTexture>(Engine::getInstance()->getDxCommand(), sceneViewRtvArray_.get(), sceneViewSrvArray_.get());
    /// TODO
    // fix MagicNumber
    sceneView_->Init({1280.0f, 720.0f}, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, {0.0f, 0.0f, 0.0f, 1.0f});
}

void SceneManager::Finalize() {
    scenes_.clear();

    sceneView_->Finalize();
    sceneViewRtvArray_->Finalize();
    sceneViewSrvArray_->Finalize();
}

void SceneManager::Update() {
    currentScene_->Update();

    CameraManager::getInstance()->DataConvertToBuffer();
}

void SceneManager::Draw() {
    sceneView_->PreDraw();

    ///===============================================
    /// sprite
    ///===============================================
    SpriteCommon::getInstance()->PreDraw();

    currentScene_->DrawSprite();

    ///===============================================
    /// 3d Object
    ///===============================================
    Object3d::PreDraw();

    currentScene_->Draw3d();
    ///===============================================
    /// Line
    ///===============================================
    PrimitiveDrawer::PreDrawLine();

    currentScene_->DrawLine();
    ///===============================================
    /// Particle
    ///===============================================
    ParticleManager::getInstance()->PreDraw();

    currentScene_->DrawParticle();
   
    ///===============================================
    /// off screen Rendering
    ///===============================================
    sceneView_->PostDraw();
    Engine::getInstance()->ScreenPreDraw();
    sceneView_->DrawTexture();
    Engine::getInstance()->ScreenPostDraw();
}

SceneManager::SceneManager() {}

SceneManager::~SceneManager() {}

void SceneManager::addScene(
    const std::string& name,
    std::unique_ptr<IScene> scene) {
    sceneIndexs_[name] = static_cast<int32_t>(scenes_.size());
    scenes_.push_back(std::move(scene));
}

void SceneManager::changeScene(const std::string& name) {
    currentScene_ = scenes_[sceneIndexs_[name]].get();
    currentScene_->Init();
}
