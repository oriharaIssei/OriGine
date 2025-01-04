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
#include "camera/gameCamera/GameCamera.h"

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

    gameCamera_ = std::make_unique<GameCamera>();
    gameCamera_->Init();

    collisionManager_ = std::make_unique<CollisionManager>();

    player_ = std::make_unique<Player>();
    player_->Init();
    player_->setCameraTransform(const_cast<CameraTransform*>(&gameCamera_->getCameraTransform()));
    gameCamera_->setFollowTarget(const_cast<Transform*>(&player_->getTransform()));

    enemyManager_ = std::make_unique<EnemyManager>();
    enemyManager_->Init();
    enemyManager_->setPlayer(player_.get());

    ground_ = std::make_unique<Object3d>();
    ground_->Init("resource/Models", "Ground.obj");
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
#else  // !_DEBUG
    gameCamera_->Update();
    CameraManager::getInstance()->setTransform(gameCamera_->getCameraTransform());
#endif // _DEBUG

    player_->Update();

    enemyManager_->Update();

    ///collision
    //clear
    collisionManager_->clearCollider();
    //add
    collisionManager_->addCollider(player_->getHitCollider());
    enemyManager_->setCollidersForCollisionManager(collisionManager_.get());

    collisionManager_->Update();
}

void GameScene::Draw3d() {
    ground_->Draw();

    player_->Draw();

    enemyManager_->Draw();

    collisionManager_->Draw();
}

void GameScene::DrawLine() {}

void GameScene::DrawSprite() {}

void GameScene::DrawParticle() {}
