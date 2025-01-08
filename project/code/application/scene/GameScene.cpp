#include "GameScene.h"

///stl
#include <string>

///Engine & application
#include "Engine.h"
//module
#include "../Collision/CollisionManager.h"
#include "SceneManager.h"
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
#include "sprite/Sprite.h"

//object
#include "../AttackCollider/AttackCollider.h"
#include "../Enemy/Manager/EnemyManager.h"
#include "../Enemy/WeakEnemy.h"
#include "../HitEffectManager/HitEffectManager.h"
#include "../Player/Player.h"
#include "../PlayerHpBar/PlayerHpBar.h"
#include "camera/gameCamera/GameCamera.h"
//debug
#ifdef _DEBUG
#include "animationEditor/AnimationEditor.h"
#include "camera/debugCamera/DebugCamera.h"
#include "imgui/imgui.h"
#endif // _DEBUG

GameScene::GameScene()
    : IScene("GameScene"),
      dashUIPos_{"Game", "UI", "dashUIPos"},
      attackUIPos_{"Game", "UI", "attackUIPos"},
      jumpUIPos_{"Game", "UI", "jumpUIPos"} {}

GameScene::~GameScene() {}

void GameScene::Init() {
#ifdef _DEBUG
    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Init();

    debugCamera_->setViewTranslate({0.0f, 0.0f, -12.0f});

#endif // _DEBUG
    ModelManager* modelManager = ModelManager::getInstance();

    {
        std::vector<std::unique_ptr<Model>> useModels;
        useModels.reserve(10);
        useModels.push_back(std::move(modelManager->Create("resource/Models", "Player.obj")));
        useModels.push_back(std::move(modelManager->Create("resource/Models", "Enemy.obj")));
        useModels.push_back(std::move(modelManager->Create("resource/Models", "EnemySpawner.obj")));
        useModels.push_back(std::move(modelManager->Create("resource/Models", "BattleField.obj")));
        useModels.push_back(std::move(modelManager->Create("resource/Models", "Skydome.obj")));
        useModels.push_back(std::move(modelManager->Create("resource/Models", "hitParticle.obj")));
        useModels.push_back(std::move(modelManager->Create("resource/Models", "shadowPlane.obj")));
        while (true) {
            //model の読み込みを先にする
            if (useModels.empty()) {
                break;
            }
            if (useModels.back()) {
                useModels.pop_back();
            }
        }
    }

    //input
    input_ = Input::getInstance();
    //camera
    gameCamera_ = std::make_unique<GameCamera>();
    gameCamera_->Init();

    activeGameObjects_.reserve(100);

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

    collisionManager_->clearCollider();
    enemyManager_->removeDeadEnemy();
    enemyManager_->removeDeadSpawner();
    activeGameObjects_.clear();

    // add activeGameObjects_
    if (player_->getIsAlive()) {
        activeGameObjects_.push_back(player_.get());

        auto playerAttackCollider = player_->getAttackCollider();
        if (playerAttackCollider && playerAttackCollider->getIsAlive()) {
            activeGameObjects_.push_back(playerAttackCollider);
        }
    } else {
        // playerが死んだらタイトルに戻る
        SceneManager::getInstance()->changeScene("TitleScene");
        return;
    }
    if (enemyManager_->getSpawners().empty()) {
        // クリア
        SceneManager::getInstance()->changeScene("GameClearScene");
        return;
    }
    for (auto& enemySpawner : enemyManager_->getSpawners()) {
        if (enemySpawner->getIsAlive()) {
            activeGameObjects_.push_back(enemySpawner.get());
        }
    }
    for (auto& enemy : enemyManager_->getEnemies()) {
        if (enemy->getIsAlive()) {
            activeGameObjects_.push_back(enemy.get());

            auto enemyAttackCollider = enemy->getAttackCollider();
            if (enemyAttackCollider && enemyAttackCollider->getIsAlive()) {
                activeGameObjects_.push_back(enemyAttackCollider);
            }
        }
    }

    // update activeGameObjects_
    for (auto& gameObject : activeGameObjects_) {
        gameObject->Update();
    }

    ///collision
    //add
    for (auto& gameObject : activeGameObjects_) {
        auto collider = gameObject->getHitCollider();
        if (collider && collider->getIsAlive()) {
            collisionManager_->addCollider(collider);
        }
    }

    //checkCollison & clear
    collisionManager_->Update();

    HitEffectManager::getInstance()->Update();

    //UI
    dashUI_->setPosition(dashUIPos_);
    dashUI_->Update();
    attackUI_->setPosition(attackUIPos_);
    attackUI_->Update();
    jumpUI_->setPosition(jumpUIPos_);
    jumpUI_->Update();

    playerHpBar_->Update();
}

void GameScene::Draw3d() {
    ground_->Draw();
    skyDome_->Draw();

    player_->Draw();

    enemyManager_->Draw();
    HitEffectManager::getInstance()->Draw();
    //collisionManager_->Draw();
}

void GameScene::DrawLine() {}

void GameScene::DrawSprite() {
    dashUI_->Draw();
    attackUI_->Draw();
    jumpUI_->Draw();

    playerHpBar_->Draw();
}

void GameScene::DrawParticle() {}
