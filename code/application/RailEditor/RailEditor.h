#pragma once

#include <memory>

#include "directX12/IConstantBuffer.h"
#include "material/Material.h"
#include "transform/CameraTransform.h"
#include "transform/Transform.h"
#include "object3d/Object3d.h"

#include "Matrix4x4.h"
#include "stdint.h"
#include "Vector3.h"

class ControlPoint{
public:
	ControlPoint(const CameraTransform& pCameraBuffer):pCameraBuffer_(pCameraBuffer){}
	~ControlPoint() = default;

	void Init(const Vector3 pos,float radius);
	void Update();

#ifdef _DEBUG
	void Debug(int32_t num);
#endif // _DEBUG

	void Draw(const IConstantBuffer<CameraTransform>& cameraTrans,const IConstantBuffer<Material>* material);
private:
	const CameraTransform& pCameraBuffer_;

	IConstantBuffer<Transform> transform_;
	float radius_;
public:
	Vector3 getWorldPosition()const{ return transform_.openData_.worldMat[3]; }
	Vector3 getTranslate()const{ return transform_.openData_.translate; }
};

class RailEditor{
public:
	RailEditor(CameraTransform& pCameraBuffer):pCameraBuffer_(pCameraBuffer){}
	void Init();
	void Update();
	void Draw(const IConstantBuffer<CameraTransform>& cameraTrans);

	void Load();
	void Save();
private:
	Transform origin_;
	CameraTransform& pCameraBuffer_;

	uint32_t segmentCount_ = 256;
	std::vector<Vector3> splineSegmentPoint_;
	std::vector<std::unique_ptr<Object3d>> railObjects_;

	std::list<std::unique_ptr<ControlPoint>> ctlPoints_;
	std::vector<Vector3> controlPointPositions_;
public:
	uint32_t getSegmentCount()const{ return segmentCount_; }
	const std::vector<Vector3>& getControlPointPositions()const{ return controlPointPositions_; }
};