#include "GameClearScene.h"
#include "TitleScene.h"

#include "input/Input.h"
#include "SceneManager.h"
#include "sprite/SpriteCommon.h"

#include "sprite/Sprite.h"

GameClearScene::GameClearScene()
    :IScene("GameClear"){}

GameClearScene::~GameClearScene(){}

void GameClearScene::Init(){
    input_ = Input::getInstance();
    
    clearSprite_ = std::make_unique<Sprite>(SpriteCommon::getInstance());
    clearSprite_->Init("resource/Texture/GameClear.png");
    clearSprite_->setPosition(Vector2(640.0f,128.0f));
    clearSprite_->setAnchorPoint(Vector2(0.5f,0.5f));
    clearSprite_->setTextureSize(Vector2(512.0f,128.0f));
    clearSprite_->setSize(Vector2(512.0f,128.0f));
    clearSprite_->Update();

    pushSpaceSprite_ = std::make_unique<Sprite>(SpriteCommon::getInstance());
    pushSpaceSprite_->Init("resource/Texture/pushSpace.png");
    pushSpaceSprite_->setPosition(Vector2(640.0f,360.0f));
    pushSpaceSprite_->setAnchorPoint(Vector2(0.5f,0.5f));
    pushSpaceSprite_->setTextureSize(Vector2(256.0f,64.0f));
    pushSpaceSprite_->setSize(Vector2(256.0f,64.0f));
    pushSpaceSprite_->Update();
}

void GameClearScene::Update(){
    if(input_->isTriggerKey(DIK_SPACE)){
        SceneManager::getInstance()->ChangeScene(std::make_unique<TitleScene>());
    }
}

void GameClearScene::Draw3d(){}

void GameClearScene::DrawLine(){}

void GameClearScene::DrawSprite(){
    clearSprite_->Draw();
    pushSpaceSprite_->Draw();
}

void GameClearScene::DrawParticle(){}
