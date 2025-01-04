#include "GameScene.h"

///stl
#include <string>

///Engine & application
#include "Engine.h"
//module
#include "../Collision/CollisionManager.h"
#include "directX12/DxRtvArrayManager.h"
#include "directX12/DxSrvArrayManager.h"
#include "material/texture/TextureManager.h"
#include "model/ModelManager.h"
#include "myFileSystem/MyFileSystem.h"
#include "particle/manager/ParticleManager.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "sprite/SpriteCommon.h"

//component
#include "camera/CameraManager.h"
#include "directX12/RenderTexture.h"
#include "object3d/AnimationObject3d.h"

//object
#include "../Enemy/Manager/EnemyManager.h"
#include "../Enemy/WeakEnemy.h"
#include "../Player/Player.h"

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

    collisionManager_ = std::make_unique<CollisionManager>();

    player_ = std::make_unique<Player>();
    player_->Init();

    enemyManager_ = std::make_unique<EnemyManager>();
    enemyManager_->Init();
    enemyManager_->setPlayer(player_.get());

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

    enemyManager_->Update();

    collisionManager_->Update();
}

void GameScene::Draw3d() {
    ground_->Draw();

    player_->Draw();

    enemyManager_->Draw();
}

void GameScene::DrawLine() {}

void GameScene::DrawSprite() {}

void GameScene::DrawParticle() {}
