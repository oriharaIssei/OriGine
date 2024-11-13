#include "Enemy.h"

#include "SLerp.h"
#include "System.h"

#include "../score/Score.h"
#include "globalVariables/GlobalVariables.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

void Enemy::Init(const std::string& groupName,int32_t index,Model* model){
	isAlive_ = true;
	GlobalVariables* variables = GlobalVariables::getInstance();
	///===============================================
	/// Object Initialize
	///===============================================
	object_ = std::make_unique<Object3d>();
	object_->SetModel(model);

	///===============================================
	/// Object.Transform Initialize
	///===============================================
	object_->transform_.CreateBuffer(System::getInstance()->getDxDevice()->getDevice());
	object_->transform_.openData_.Init();
#ifndef _DEBUG
	Vector3 spawnPos_;
#endif
	variables->addValue("Game",groupName,"Enemy_" + std::to_string(index) + "_SpawnPos",spawnPos_);
	object_->transform_.openData_.translate = spawnPos_;

	///===============================================
	/// Velocity Initialize
	///===============================================
	variables->addValue("Game",groupName,"Enemy_" + std::to_string(index) + "_Direction",direction_);
	variables->addValue("Game",groupName,"Enemy_" + std::to_string(index) + "_Speed",speed_);
	velocity_ = direction_ * speed_;

	GlobalVariables::getInstance()->addValue("Game","Enemy","score",score_);

	radius_ = 1.0f;
}

void Enemy::Update(){

	object_->transform_.openData_.translate += velocity_ * System::getInstance()->getDeltaTime();

	velocity_ = Slerp(0.02f,velocity_.Normalize(),velocity_);

	// xz 平面内での velocity_ の角度を計算
	object_->transform_.openData_.rotate.y = std::atan2(velocity_.x,velocity_.z);

	// y 軸周りに回転させる
	Vector3 velocityZ = MakeMatrix::RotateY(-object_->transform_.openData_.rotate.y) * velocity_;

	// 回転後の velocity_ を使って zy 平面内での角度を計算
	object_->transform_.openData_.rotate.x = std::atan2(-velocityZ.y,velocityZ.z);

	object_->transform_.openData_.UpdateMatrix();
	object_->transform_.ConvertToBuffer();
}

void Enemy::Draw(const IConstantBuffer<CameraTransform>& cameraTrans){
	object_->Draw(cameraTrans);
}

void Enemy::OnCollision(){
	Score::getInstance()->AddScore(score_);
	isAlive_ = false;
}
