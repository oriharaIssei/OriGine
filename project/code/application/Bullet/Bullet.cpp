#include "Bullet.h"

#include "Engine.h"
#include "sprite/Sprite.h"
#include "sprite/SpriteCommon.h"

Bullet::Bullet(){}

Bullet::~Bullet(){}

void Bullet::Init(const Vector2& _pos,const Vector2& _direction){
    isAlive_ = true;

    sprite_ = std::make_unique<Sprite>(SpriteCommon::getInstance());
    sprite_->Init("resource/Texture/white1x1.png");
    sprite_->setTextureSize(Vector2(1.0f,1.0f));
    sprite_->setAnchorPoint(Vector2(0.5f,0.5f));
    sprite_->setPosition(_pos);
    sprite_->setSize(Vector2(6.0f,6.0f));
    sprite_->setColor(Vector4(0.32f,0.32f,1.0f,1.0f));
    sprite_->ConvertMappingData();

    velocity_ = _direction * speed_;
}

void Bullet::Update(){
    float deltaTime = Engine::getInstance()->getDeltaTime();

    sprite_->setPosition(sprite_->GetPosition() + velocity_ * deltaTime);

    const Vector2& pos = sprite_->GetPosition();

    // 画面外に出たら消滅
    if(0.0f >= pos.x - radius_ ||
       pos.x + radius_ >= Engine::getInstance()->getWinApp()->getWidth() ||
       0.0f >= pos.y - radius_ ||
       pos.y + radius_ >= Engine::getInstance()->getWinApp()->getHeight()){
        isAlive_ = false;
    }
}

void Bullet::Draw(){
    sprite_->Draw();
}

const Vector2& Bullet::getPosition() const{ return sprite_->GetPosition(); }
