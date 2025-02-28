#include "GameScene.h"

/// stl
#include <string>

/// Engine & application
#include "engine/Engine.h"
// module
#include "engine/directX12/DxRtvArrayManager.h"
#include "engine/directX12/DxSrvArrayManager.h"
#include "engine/effect/manager/EffectManager.h"
#include "engine/model/ModelManager.h"
#include "engine/primitiveDrawer/PrimitiveDrawer.h"
#include "engine/texture/TextureManager.h"
#include "lib/myFileSystem/MyFileSystem.h"
#include "scene/manager/SceneManager.h"

// component
#include "engine/camera/CameraManager.h"
#include "engine/directX12/RenderTexture.h"
#include "engine/ECS/component/renderer/sprite/Sprite.h"

/// math
#include "math/Vector3.h"

// object
#include "engine/camera/gameCamera/GameCamera.h"
// debug
#ifdef _DEBUG
#include "../externals/imgui/imgui.h"
#include "engine/camera/debugCamera/DebugCamera.h"
#endif // _DEBUG

GameScene::GameScene()
    : IScene("GameScene") {}

GameScene::~GameScene() {}

void GameScene::Init() {
    // #ifdef _DEBUG
    //     debugCamera_ = std::make_unique<DebugCamera>();
    //     debugCamera_->Init();
    //
    //     debugCamera_->setViewTranslate({0.0f, 0.0f, -12.0f});
    //
    // #endif // _DEBUG
    //     ModelManager* modelManager = ModelManager::getInstance();
    //
    //     // input
    //     input_ = Input::getInstance();
    //     // camera
    //     gameCamera_ = std::make_unique<GameCamera>();
    //     gameCamera_->Init();
}

void GameScene::Update() {
    // #ifdef _DEBUG
    //     ImGui::Begin("DebugCamera");
    //     ImGui::Checkbox("isActive", &isDebugCameraActive_);
    //     ImGui::End();
    //     if (isDebugCameraActive_) {
    //         debugCamera_->Update();
    //         debugCamera_->DebugUpdate();
    //         CameraManager::getInstance()->setTransform(debugCamera_->getCameraTransform());
    //     } else {
    //         gameCamera_->Update();
    //         CameraManager::getInstance()->setTransform(gameCamera_->getCameraTransform());
    //     }
    // #else // !_DEBUG
    //     gameCamera_->Update();
    //     CameraManager::getInstance()->setTransform(gameCamera_->getCameraTransform());
    // #endif // _DEBUG
}
