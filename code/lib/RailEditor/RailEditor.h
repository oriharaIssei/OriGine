#pragma once

#include <memory>

#include "directX12/buffer/Material.h"
#include "directX12/buffer/CameraBuffer.h"
#include "directX12/buffer/TransformBuffer.h"

#include "Matrix4x4.h"
#include "stdint.h"
#include "Vector3.h"

class ControlPoint{
public:
	ControlPoint(const CameraBuffer& pCameraBuffer):pCameraBuffer_(pCameraBuffer){}
	~ControlPoint() = default;

	void Init(const Vector3 pos,float radius);
	void Update(int32_t num);
	void Draw(const Material* material);

private:
	const CameraBuffer& pCameraBuffer_;

	TransformBuffer transform_;
	float radius_;
public:
	Vector3 getWorldPosition()const{ return transform_.worldMat[3]; }
	Vector3 getTranslate()const{return transform_.translate;}
};

class RailEditor{
public:
	RailEditor(CameraBuffer& pCameraBuffer):pCameraBuffer_(pCameraBuffer){}
	void Init();
	void Update();
	void Draw();

	void Load();
	void Save();
private:
	TransformBuffer origin_;
	 CameraBuffer& pCameraBuffer_;

	uint32_t segmentCount_ = 432;
	std::vector<Vector3> splineSegmentPoint_;

	std::list<std::unique_ptr<ControlPoint>> ctlPoints_;
	std::vector<Vector3> controlPointPositions_;
};