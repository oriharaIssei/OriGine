#pragma once

#include <vector>

#include "directX12/buffer/CameraBuffer.h"
#include "directX12/buffer/TransformBuffer.h"

#include "Vector3.h"

class RailCamera
{
public:
	RailCamera(const std::vector<Vector3>& controlPoints):controlPoints_(controlPoints){}
	~RailCamera(){}
	void Init();
	void Update();
	
	void Reset(){count_ = 0;}
private:
	TransformBuffer transform_;
	CameraBuffer cameraBuff_;
	const std::vector<Vector3>& controlPoints_;

	int32_t count_;
	int32_t dimension_;
public:
	const CameraBuffer& getCameraBuffer()const{return cameraBuff_;}
	void setDimension(int32_t dimension){dimension_ = dimension;}
};