#include "GameScene.h"

#include "Engine.h"
#include "camera/CameraManager.h"
#include "directX12/DxCommand.h"
#include "directX12/DxRtvArray.h"
#include "directX12/DxRtvArrayManager.h"
#include "directX12/DxSrvArrayManager.h"
#include "directX12/RenderTexture.h"
#include "material/texture/TextureManager.h"
#include "model/ModelManager.h"
#include "myFileSystem/MyFileSystem.h"
#include "object3d/AnimationObject3d.h"
#include "particle/manager/ParticleManager.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "sprite/SpriteCommon.h"

#include <string>

#ifdef _DEBUG
#include "camera/debugCamera/DebugCamera.h"
#include "imgui/imgui.h"

#include "animationEditor/AnimationEditor.h"
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

    input_ = Input::getInstance();

    materialManager_ = Engine::getInstance()->getMaterialManager();

    ground_ = std::make_unique<Object3d>();
    ground_->Init("resource/Models", "Ground.obj");
}

void GameScene::Update() {
#ifdef _DEBUG
    debugCamera_->Update();
    debugCamera_->DebugUpdate();
    CameraManager::getInstance()->setTransform(debugCamera_->getCameraTransform());

#endif // _DEBUG

    Engine::getInstance()->getLightManager()->Update();
}

void GameScene::Draw3d() {
    ground_->Draw();
}

void GameScene::DrawLine() {}

void GameScene::DrawSprite() {}

void GameScene::DrawParticle() {}
