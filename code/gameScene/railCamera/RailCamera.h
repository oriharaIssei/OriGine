#pragma once

#include <vector>

#include "directX12/buffer/CameraBuffer.h"
struct Vector3;

class RailCamera
{
public:
	RailCamera(const std::vector<Vector3>& points):points_(points){}
	~RailCamera(){}
	void Init();
	void Update();
	
private:
	const std::vector<Vector3>& points_;
	CameraBuffer cameraBuff_;
public:
	const CameraBuffer& getCameraBuffer()const{return cameraBuff_;}
};