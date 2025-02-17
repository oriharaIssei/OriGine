#include "GameScene.h"

/// stl
#include <string>

/// Engine & application
#include "Engine.h"
// module
#include "directX12/DxRtvArrayManager.h"
#include "directX12/DxSrvArrayManager.h"
#include "effect/manager/EffectManager.h"
#include "texture/TextureManager.h"
#include "model/ModelManager.h"
#include "myFileSystem/MyFileSystem.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "SceneManager.h"

// component
#include "camera/CameraManager.h"
#include "component/renderer/RendererComponentFactory.h"
#include "directX12/RenderTexture.h"
#include "object3d/AnimationObject3d.h"
#include "component/renderer/sprite/Sprite.h"

// object
#include "camera/gameCamera/GameCamera.h"
// debug
#ifdef _DEBUG
#include "animationEditor/AnimationEditor.h"
#include "camera/debugCamera/DebugCamera.h"
#include "imgui/imgui.h"
#endif // _DEBUG

GameScene::GameScene()
    : IScene("GameScene") {}

GameScene::~GameScene() {}

void GameScene::Init() {
#ifdef _DEBUG
    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Init();

    debugCamera_->setViewTranslate({0.0f, 0.0f, -12.0f});

#endif // _DEBUG
    ModelManager* modelManager = ModelManager::getInstance();

    // input
    input_ = Input::getInstance();
    // camera
    gameCamera_ = std::make_unique<GameCamera>();
    gameCamera_->Init();

    test = CreateRendererComponent<SpriteRenderer,SpriteRendererController>("resource/Texture/white1x1.png");
}

void GameScene::Update() {
#ifdef _DEBUG
    ImGui::Begin("DebugCamera");
    ImGui::Checkbox("isActive", &isDebugCameraActive_);
    ImGui::End();
    if (isDebugCameraActive_) {
        debugCamera_->Update();
        debugCamera_->DebugUpdate();
        CameraManager::getInstance()->setTransform(debugCamera_->getCameraTransform());
    } else {
        gameCamera_->Update();
        CameraManager::getInstance()->setTransform(gameCamera_->getCameraTransform());
    }
#else // !_DEBUG
    gameCamera_->Update();
    CameraManager::getInstance()->setTransform(gameCamera_->getCameraTransform());
#endif // _DEBUG
}

void GameScene::Draw3d() {}

void GameScene::DrawLine() {}

void GameScene::DrawSprite() {}

void GameScene::DrawParticle() {}
