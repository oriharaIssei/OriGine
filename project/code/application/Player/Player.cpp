#include "Player.h"

#include "Engine.h"

#include "PlayerBehaviors/PlayerRootBehavior.h"

#include "object3d/Object3d.h"
#include "transform/Transform.h"

Player::Player(){}

Player::~Player(){}

void Player::Init(){
	// 座標系
	transform_.UpdateMatrix();

	// DrawObject
	drawObject3d_ = std::move(Object3d::Create("resource/Models","Enemy.obj"));

	// Behavior
	currentBehavior_ = std::make_unique<PlayerRootBehavior>(this);
	currentBehavior_->Init();
}

void Player::Update(){
	currentBehavior_->Update();

	{// Transform Update
		transform_.UpdateMatrix();
	}
}

void Player::Draw(){
	drawObject3d_->Draw();
}

void Player::ChangeBehavior(IPlayerBehavior* next){
	currentBehavior_.reset(next);
	currentBehavior_->Init();
}

const Vector3& Player::getScale() const{
	return transform_.scale;
}

const Quaternion& Player::getRotate() const{
	return transform_.rotate;
}

const Vector3& Player::getTranslate() const{
	return transform_.translate;
}

void Player::setScale(const Vector3& s){
	transform_.scale = s;
}

void Player::setRotate(const Quaternion& q){
	transform_.rotate = q;
}

void Player::setTranslate(const Vector3& t){
	transform_.translate = t;
}
