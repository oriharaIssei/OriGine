#pragma once

#include <memory>
#include <string>

#include "object3d/Object3d.h"

class Enemy{
public:
	Enemy() = default;
	~Enemy(){}

	void Init(const std::string& groupName,int32_t index,Model* model);
	void Update();
	void Draw(const IConstantBuffer<CameraTransform>& cameraTrans);

	void OnCollision();
private:
	std::unique_ptr<Object3d> object_;

#ifdef _DEBUG
	Vector3 spawnPos_;
#endif // _DEBUG
	Vector3 velocity_;
	float radius_;
public:
	Vector3 GetPos()const{ return object_->transform_.openData_.worldMat[3]; }
	void SetPos(const Vector3& pos){ object_->transform_.openData_.translate = pos; }

	const Vector3& GetVelocity()const{ return velocity_; }
	void SetVelocity(const Vector3& velo){ velocity_ = velo; }

	float GetRadius()const{ return radius_; }
};