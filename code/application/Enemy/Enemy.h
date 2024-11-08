#pragma once

#include <memory>

#include "object3d/Object3d.h"

#ifdef _DEBUG
struct EnemyInitializeVariables{
	Vector3 pos,velocity;
};
#endif // _DEBUG


class Enemy{
public:
	Enemy() = default;
	~Enemy(){}

	void Init(const Vector3& pos,const Vector3& velocity,Model* model);
	void Update();
	void Draw(const IConstantBuffer<CameraTransform>& cameraTrans);
private:
	std::unique_ptr<Object3d> object_;

	Vector3 velocity_;
	float radius_;
public:
	Vector3 GetPos()const{ object_->transform_.openData_.worldMat[3]; }
	void SetPos(const Vector3& pos){ object_->transform_.openData_.translate = pos; }

	const Vector3& GetVelocity()const{ return velocity_; }
	void SetVelocity(const Vector3& velo){ velocity_ = velo; }

	float GetRadius()const{ return radius_; }
};