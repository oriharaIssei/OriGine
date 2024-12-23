#include "Enemy.h"

#include <algorithm>

#include "Engine.h"
#include "myRandom/MyRandom.h"
#include "sprite/Sprite.h"
#include "sprite/spriteCommon.h"

Enemy::Enemy(){}

Enemy::~Enemy(){}

void Enemy::Init(){
    isAlive_ = true;

    sprite_ = std::make_unique<Sprite>(SpriteCommon::getInstance());
    sprite_->Init("resource/Texture/white1x1.png");

    sprite_->setAnchorPoint(Vector2(0.5f,0.5f));
    float randPozX = MyRandom::Float(0.0f,720.0f).get();
    sprite_->setPosition(Vector2(randPozX,128.0f));
    sprite_->setAnchorPoint({0.5f,0.5f});
    sprite_->setTextureSize(Vector2(1.0f,1.0f));
    sprite_->setSize(Vector2(radius_ * 2.0f,radius_ * 2.0f));
    sprite_->setColor(Vector4(1.0f,0.32f,0.32f,1.0f));

    Vector2 randDirection = Vector2(MyRandom::Float(-1.0f,1.0f).get(),0.0f);
    velocity_ = randDirection.normalize() * speed_;
}

void Enemy::Update(){
    float deltaTime          = Engine::getInstance()->getDeltaTime();
    Vector2 spritePos = sprite_->GetPosition();
    const WinApp* winApp     = Engine::getInstance()->getWinApp();

    spritePos += velocity_ * deltaTime;

    sprite_->setPosition(spritePos);
    spritePos = sprite_->GetPosition();

    if(spritePos.y + radius_ >= winApp->getHeight() || spritePos.y - radius_ <= 0.0f){
        sprite_->setPosition(Vector2(spritePos.x,std::clamp<float>(spritePos.y,radius_,static_cast<float>(winApp->getHeight()) - radius_)));
        velocity_.y *= -1.0f;
    }
    if(spritePos.x + radius_ > winApp->getWidth() || spritePos.x - radius_ < 0.0f){
        sprite_->setPosition(Vector2(std::clamp<float>(spritePos.x,radius_,static_cast<float>(winApp->getWidth()) - radius_),spritePos.y));
        velocity_.x *= -1.0f;
    }
    sprite_->Update();
}

void Enemy::Draw(){
    sprite_->Draw();
}

const Vector2& Enemy::getPosition() const{
    return sprite_->GetPosition();
}
