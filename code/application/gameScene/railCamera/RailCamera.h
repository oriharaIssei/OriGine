#pragma once

#include <vector>

#include "model/Model.h"
#include "directX12/buffer/CameraBuffer.h"
#include "directX12/buffer/TransformBuffer.h"

#include "Vector3.h"

class RailCamera
{
public:
	RailCamera(const std::vector<Vector3>& controlPoints):controlPoints_(controlPoints){}
	~RailCamera(){ transform_.Finalize(); cameraBuff_.Finalize();}
	void Init();
	void Update();
	void Draw(const CameraBuffer& cameraBuff);
	
	void Reset(){count_ = 0;}
private:
	Model* model_;
	TransformBuffer transform_;
	CameraBuffer cameraBuff_;
	const std::vector<Vector3>& controlPoints_;

	int32_t count_;
	int32_t dimension_;
public:
	const CameraBuffer& getCameraBuffer()const{return cameraBuff_;}
	const TransformBuffer& getTransform()const{return transform_;}
	void setDimension(int32_t dimension){dimension_ = dimension;}
};