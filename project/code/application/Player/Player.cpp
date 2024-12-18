#include "Player.h"
// commands
#include "IPlayerCommand.h"
#include "PlayerMoveCommand.h"

// input
#include "input/Input.h"

// sprite
#include "sprite/Sprite.h"
#include "sprite/SpriteCommon.h"

Player::Player(){}

Player::~Player(){}

void Player::Init(){
    sprite_.reset(SpriteCommon::getInstance()->Create("resource/Texture/white1x1.png"));
    // TODO マルチスレッドでの textureSize 同期 
    sprite_->setTextureSize({1.0f,1.0f});
    // 移動分と サイズを 同じにする
    sprite_->setSize({speed_,speed_});
    // セットした内容を 反映
    sprite_->Update();
}

void Player::Update(){
    // command 初期化
    command_.reset();

    InputHandle();

    if(command_){
        command_->Execute();
    }
}

void Player::Draw(){
    sprite_->Draw();
}

void Player::InputHandle(){
    // 入力取得
    Input* input = Input::getInstance();
    if(input->isTriggerKey(DIK_W)){
        command_.reset(new PlayerMoveUpCommand(this,speed_));
    } else if(input->isTriggerKey(DIK_S)){
        command_.reset(new PlayerMoveDownCommand(this,speed_));
    } else if(input->isTriggerKey(DIK_A)){
        command_.reset(new PlayerMoveLeftCommand(this,speed_));
    } else if(input->isTriggerKey(DIK_D)){
        command_.reset(new PlayerMoveRightCommand(this,speed_));
    }
}

const Vector2& Player::getPos() const{
    return sprite_->getPosition();
}

const void Player::setPos(const Vector2& pos){
    sprite_->setPosition(pos);
}
