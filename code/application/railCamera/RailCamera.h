#pragma once

#include <memory>
#include <vector>

#include "directX12/IConstantBuffer.h"
#include "object3d/Object3d.h"
#include "transform/CameraTransform.h"
#include "transform/Transform.h"

#include "Vector3.h"

class RailCamera
{
public:
	RailCamera(const std::vector<Vector3>& controlPoints):controlPoints_(controlPoints){}
	~RailCamera(){}
	void Init(int32_t dimension);
	void Update();
	void Draw(const IConstantBuffer<CameraTransform>& cameraBuff);
	
	void Reset(){count_ = 0;}
private:
	std::unique_ptr<Object3d> object_;
	CameraTransform cameraBuff_;
	const std::vector<Vector3>& controlPoints_;

	int32_t count_;
	int32_t dimension_;
public:
	const CameraTransform& getCameraBuffer()const{return cameraBuff_;}
	const Transform& getTransform()const{return object_->transform_.openData_;}
	void setDimension(int32_t dimension){dimension_ = dimension;}
};