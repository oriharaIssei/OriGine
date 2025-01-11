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
#include "camera/gameCamera/GameCamera.h"
//debug
#ifdef _DEBUG
#include "animationEditor/AnimationEditor.h"
#include "camera/debugCamera/DebugCamera.h"
#include "imgui/imgui.h"
#endif // _DEBUG

GameScene::GameScene()
    : IScene("GameScene"){}

GameScene::~GameScene() {}

void GameScene::Init() {
#ifdef _DEBUG
    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Init();

    debugCamera_->setViewTranslate({0.0f, 0.0f, -12.0f});

#endif // _DEBUG
    ModelManager* modelManager = ModelManager::getInstance();

    //input
    input_ = Input::getInstance();
    //camera
    gameCamera_ = std::make_unique<GameCamera>();
    gameCamera_->Init();

    ground_ = std::make_unique<Object3d>();
    ground_->Init("resource/Models", "BattleField.obj");

    skyDome_ = std::make_unique<Object3d>();
    skyDome_->Init("resource/Models", "Skydome.obj");

    HitEffectManager::getInstance()->Init();

    //UI
    dashUI_   = std::make_unique<Sprite>();
    attackUI_ = std::make_unique<Sprite>();
    jumpUI_   = std::make_unique<Sprite>();
    dashUI_->Init("resource/Texture/dashUI.png");
    attackUI_->Init("resource/Texture/attackUI.png");
    jumpUI_->Init("resource/Texture/jumpUI.png");

    dashUI_->setAnchorPoint(Vec2f(0.5f, 0.5f));
    attackUI_->setAnchorPoint(Vec2f(0.5f, 0.5f));
    jumpUI_->setAnchorPoint(Vec2f(0.5f, 0.5f));
    Vec2f uiSize_ = Vec2f(128.0f, 32.0f);
    dashUI_->setSize(uiSize_);
    attackUI_->setSize(uiSize_);
    jumpUI_->setSize(uiSize_);

    playerHpBar_ = std::make_unique<PlayerHpBar>();
    playerHpBar_->setPlayer(player_.get());
    playerHpBar_->Init();
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

}

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
    dashUI_->setTranslate(dashUIPos_);
    dashUI_->Update();
    attackUI_->setTranslate(attackUIPos_);
    attackUI_->Update();
    jumpUI_->setTranslate(jumpUIPos_);
    jumpUI_->Update();

    playerHpBar_->Update();
}

void GameScene::Draw3d() {
    ground_->Draw();
    skyDome_->Draw();
}

void GameScene::DrawLine() {}

void GameScene::DrawSprite() {}

void GameScene::DrawParticle() {}
