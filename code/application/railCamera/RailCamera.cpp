#include "RailCamera.h"

#include "Spline.h"
#include "System.h"

void RailCamera::Init(int32_t dimension){
	cameraBuff_.UpdateMatrix();
	object_.reset(Object3d::Create("resource","axis.obj"));
	object_->transform_.CreateBuffer(System::getInstance()->getDxDevice()->getDevice());

	dimension_ = dimension;
}

void RailCamera::Update(){
	if(count_++ > dimension_){
		count_ = 0;
	}

	float t = 1.0f / dimension_ * count_;
	Vector3 eye = CatmullRomInterpolation(controlPoints_,t);
	float nextT = 1.0f / dimension_ * ((count_ + 1 >= dimension_ ? count_ - dimension_ + 1 : count_ + 1));

	Vector3 target = CatmullRomInterpolation(controlPoints_,nextT);

	Transform& transform = object_->transform_.openData_;
	Vector3 direction = (target - eye).Normalize();
	transform.rotate.y = std::atan2(direction.x,direction.z);
	Vector2 veloXZ = {direction.x,direction.z};
	transform.rotate.x = std::atan2(-direction.y,veloXZ.Length());

	transform.translate = eye;

	transform.UpdateMatrix();
	object_->transform_.ConvertToBuffer();
	cameraBuff_.viewMat = transform.worldMat.Inverse();
}

void RailCamera::Draw(const IConstantBuffer<CameraTransform>& cameraBuff){
	object_->Draw(cameraBuff);
}
