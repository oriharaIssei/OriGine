#pragma once

#include <memory>

#include "directX12/buffer/Material.h"
#include "directX12/buffer/ViewProjection.h"
#include "directX12/buffer/WorldTransform.h"

#include "Matrix4x4.h"
#include "Vector3.h"

class ControlPoint{
public:
	ControlPoint(const ViewProjection& pViewProjection):pViewProjection_(pViewProjection){};
	~ControlPoint() = default;

	void Init(const Vector3 pos,float radius);
	void Update();
	void Draw(const Material* material);
private:
	const ViewProjection& pViewProjection_;

	WorldTransform transform_;
	float radius_;
public:

};

class RailEditor{
};