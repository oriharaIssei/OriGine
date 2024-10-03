#pragma once

#include <memory>

#include "directX12/buffer/Material.h"
#include "directX12/buffer/ViewProjection.h"
#include "directX12/buffer/WorldTransform.h"

#include "Matrix4x4.h"
#include "stdint.h"
#include "Vector3.h"

class ControlPoint{
public:
	ControlPoint(const ViewProjection& pViewProjection):pViewProjection_(pViewProjection){}
	~ControlPoint() = default;

	void Init(const Vector3 pos,float radius);
	void Update();
	void Draw(const Material* material);
private:
	const ViewProjection& pViewProjection_;

	WorldTransform transform_;
	float radius_;
public:
	Vector3 getWorldPosition()const{ return transform_.worldMat[3]; }

};

class RailEditor{
public:
	RailEditor(const ViewProjection& pViewProjection):pViewProjection_(pViewProjection){}
	void Init();
	void Update();
	void Draw();
private:
	const ViewProjection& pViewProjection_;

	uint32_t segmentCount_ = 432;
	std::vector<Vector3> splineSegmentPoint_;

	std::list<std::unique_ptr<ControlPoint>> ctlPoints_;
	std::vector<Vector3> controlPointPositions_;
};