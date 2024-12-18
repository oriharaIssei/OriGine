#include "GameScene.h"


#include "engine/Engine.h"
#include "input/Input.h"
#include "material/Material.h"
// camera
#include "camera/Camera.h"
// Player
#include "../Player/Player.h"

#ifdef _DEBUG
#include "camera/debugCamera/DebugCamera.h"
#include "imgui/imgui.h"
#endif // _DEBUG

GameScene::GameScene():IScene("GameScene"){}

GameScene::~GameScene(){}

void GameScene::Init(){
#ifdef _DEBUG
    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Init();

    debugCamera_->setViewTranslate({0.0f,0.0f,-12.0f});
#endif // _DEBUG

    input_ = Input::getInstance();

    materialManager_ = Engine::getInstance()->getMaterialManager();

    player_ = std::make_unique<Player>();
    player_->Init();
}

void GameScene::Update(){
#ifdef _DEBUG
    debugCamera_->Update();
    debugCamera_->DebugUpdate();
    Camera::getInstance()->setTransform(debugCamera_->getCameraTransform());
#endif // _DEBUG

    player_->Update();

#ifdef _DEBUG
    materialManager_->DebugUpdate();
#endif // _DEBUG

    Engine::getInstance()->getLightManager()->Update();
}

void GameScene::Draw3d(){}

void GameScene::DrawLine(){}

void GameScene::DrawSprite(){
    player_->Draw();
}

void GameScene::DrawParticle(){}
