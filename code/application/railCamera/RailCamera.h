#pragma once

#include <memory>
#include <vector>

#include "directX12/IConstantBuffer.h"
#include "object3d/Object3d.h"
#include "transform/CameraTransform.h"
#include "transform/Transform.h"

#include "Vector3.h"

class Spline;
class RailCamera{
public:
	RailCamera(){}
	~RailCamera(){}
	void Init(int32_t dimension);
	void Update();
	void Draw(const IConstantBuffer<CameraTransform>& cameraBuff);

	void Reset(){ currentDistance_ = 0; }
private:
	std::unique_ptr<Object3d> object_;
	CameraTransform cameraBuff_;

	Spline* spline_ = nullptr;

	Vector3 offset_;
	float speed_; // 移動速度
	float currentDistance_; // 現在の距離
	int32_t dimension_;
public:
	float GetCurrentDistance()const{ return currentDistance_; }
	void SetSpline(Spline* _spline);

	const CameraTransform& getCameraBuffer()const{ return cameraBuff_; }
	const Transform& getTransform()const{ return object_->transform_.openData_; }
	void setDimension(int32_t dimension){ dimension_ = dimension; }
};