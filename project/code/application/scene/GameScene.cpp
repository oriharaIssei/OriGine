#include "GameScene.h"

///stl
#include <string>

///Engine & application
#include "Engine.h"
#include "camera/CameraManager.h"
#include "directX12/DxCommand.h"
#include "directX12/DxRtvArray.h"
//module
#include "directX12/DxRtvArrayManager.h"
#include "directX12/DxSrvArrayManager.h"
#include "material/texture/TextureManager.h"
#include "model/ModelManager.h"
#include "myFileSystem/MyFileSystem.h"
#include "particle/manager/ParticleManager.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "sprite/SpriteCommon.h"

//component
#include "directX12/RenderTexture.h"
#include "object3d/AnimationObject3d.h"

//object
#include "../Player/Player.h"
#include "../Enemy/IEnemy.h"
#include "../Enemy/WeakEnemy.h"

//debug
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

    input_ = Input::getInstance();

    player_ = std::make_unique<Player>();
    player_->Init();

    enemy_ = std::make_unique<WeakEnemy>();
    enemy_->Init();
    enemy_->setPlayer(player_.get());

    ground_ = std::make_unique<Object3d>();
    ground_->Init("resource/Models", "Ground.obj");
}

void GameScene::Update() {
#ifdef _DEBUG
    debugCamera_->Update();
    debugCamera_->DebugUpdate();
    CameraManager::getInstance()->setTransform(debugCamera_->getCameraTransform());
#endif // _DEBUG

    player_->Update();
    enemy_->Update();
}

void GameScene::Draw3d() {
    ground_->Draw();
    player_->Draw();

    enemy_->Draw();
}

void GameScene::DrawLine() {}

void GameScene::DrawSprite() {}

void GameScene::DrawParticle() {}
