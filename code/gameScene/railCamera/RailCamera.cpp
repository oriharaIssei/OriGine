#include "RailCamera.h"

#include "Spline.h"

void RailCamera::Init()
{
#ifdef _DEBUG
	transform_.Init();
#endif // _DEBUG

	cameraBuff_.Init();
	cameraBuff_.UpdateMatrix();
	model_ = Model::Create("resource","axis.obj");
}

void RailCamera::Update()
{
	if(count_++ > dimension_)
	{
		count_ = 0;
	}

	float t = 1.0f / dimension_ * count_;
	Vector3 eye = CatmullRomInterpolation(controlPoints_,t);
	float nextT = 1.0f / dimension_ * ((count_ + 1 >= dimension_ ? count_ - dimension_ + 1:count_ + 1));

	Vector3 target = CatmullRomInterpolation(controlPoints_,nextT);

	Vector3 direction = (target - eye).Normalize();
	transform_.rotate.y = std::atan2(direction.x,direction.z);
	Vector2 veloXZ = {direction.x,direction.z};
	transform_.rotate.x = std::atan2(-direction.y,veloXZ.Length());

	transform_.translate = eye;

#ifdef _DEBUG
	transform_.UpdateMatrix();
#endif // _DEBUG
#ifndef _DEBUG
	transform_.worldMat= MakeMatrix::Affine(transform_.scale,transform_.rotate,transform_.translate);
#endif // !_DEBUG
	cameraBuff_.viewMat = transform_.worldMat.Inverse();
}

void RailCamera::Draw(const CameraBuffer& cameraBuff)
{
	model_->Draw(transform_,cameraBuff);
}
