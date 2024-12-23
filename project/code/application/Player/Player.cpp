#include "Player.h"

#include "Engine.h"
#include "input/Input.h"
#include "sprite/SpriteCommon.h"

#include "sprite/Sprite.h"

#include "../bullet/Bullet.h"
#include "application/scene/GameScene.h"

#include "Vector2.h"

Player::Player(){}
Player::~Player(){}

void Player::Init(){
    isAlive_ = true;

    sprite_ = std::make_unique<Sprite>(SpriteCommon::getInstance());
    sprite_->Init("resource/Texture/white1x1.png");

    sprite_->setAnchorPoint(Vector2(0.5f,0.5f));
    sprite_->setPosition(Vector2(360.0f,512.0f));
    sprite_->setTextureSize(Vector2(1.0f,1.0f));
    sprite_->setSize(Vector2(16.0f,16.0f));
    sprite_->setColor(Vector4(1.0f,1.0f,1.0f,1.0f));
    sprite_->ConvertMappingData();
}

void Player::Update(GameScene* _hostScene){
    Input* input             = Input::getInstance();
    float deltaTime          = Engine::getInstance()->getDeltaTime();
    const Vector2& spritePos = sprite_->GetPosition();

    { // 移動
        Vector2 move = {
            input->isPressKey(DIK_D) - input->isPressKey(DIK_A),
            input->isPressKey(DIK_S) - input->isPressKey(DIK_W)};
        move = move.normalize();
        sprite_->setPosition(spritePos + move * speed_ * deltaTime);

        sprite_->Update();
    }

    { // 弾発射
        shotCoolTimeCounter_ += deltaTime;
        if(input->isPressKey(DIK_SPACE) && shotCoolTimeCounter_ >= shotCoolTime_){
            std::unique_ptr<Bullet> spawnBullet = std::make_unique<Bullet>();
            // 弾の初期化(位置、上方向固定)
            spawnBullet->Init(spritePos,{0.0f,-1.0f});
            _hostScene->addBullet(std::move(spawnBullet));
            shotCoolTimeCounter_ = 0.0f;
        }
    }
}

void Player::Draw(){
    sprite_->Draw();
}

const Vector2& Player::getPosition() const{
    return sprite_->GetPosition();
}
