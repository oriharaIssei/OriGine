#include "Enemy.h"

#include "SLerp.h"
#include "System.h"

void Enemy::Init(const Vector3& pos,const Vector3& velocity,Model* model){
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
	object_->transform_.openData_.translate = pos;

	///===============================================
	/// Velocity Initialize
	///===============================================
	velocity_ = velocity;

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