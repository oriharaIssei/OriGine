#include "TitleScene.h"
#include "GameScene.h"

#include "input/Input.h"
#include "SceneManager.h"
#include "sprite/SpriteCommon.h"

#include "sprite/Sprite.h"

TitleScene::TitleScene()
    :IScene("TitleScene"){}

TitleScene::~TitleScene(){}

void TitleScene::Init(){
    input_ = Input::getInstance();

    titleSprite_ = std::make_unique<Sprite>(SpriteCommon::getInstance());
    titleSprite_->Init("resource/Texture/title.png");
    titleSprite_->setPosition(Vector2(640.0f,128.0f));
    titleSprite_->setAnchorPoint(Vector2(0.5f,0.5f));
    titleSprite_->setTextureSize(Vector2(512.0f,128.0f));
    titleSprite_->setSize(Vector2(512.0f,128.0f));
    titleSprite_->Update();

    pushSpaceSprite_ = std::make_unique<Sprite>(SpriteCommon::getInstance());
    pushSpaceSprite_->Init("resource/Texture/pushSpace.png");
    pushSpaceSprite_->setPosition(Vector2(640.0f,480.0f));
    pushSpaceSprite_->setAnchorPoint(Vector2(0.5f,0.5f));
    pushSpaceSprite_->setTextureSize(Vector2(256.0f,64.0f));
    pushSpaceSprite_->setSize(Vector2(256.0f,64.0f));
    pushSpaceSprite_->Update();
}

void TitleScene::Update(){
    if(input_->isTriggerKey(DIK_SPACE)){
        SceneManager::getInstance()->ChangeScene(std::make_unique<GameScene>());
    }
}

void TitleScene::Draw3d(){}

void TitleScene::DrawLine(){}

void TitleScene::DrawSprite(){
    titleSprite_->Draw();
    pushSpaceSprite_->Draw();
}

void TitleScene::DrawParticle(){}
