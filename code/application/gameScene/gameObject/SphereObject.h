#pragma once

#include "IGameObject.h"

#include <memory>

#include "transform/CameraTransform.h"
#include "directX12/Object3dMesh.h"
#include "transform/Transform.h"

class SphereObject :
	public IGameObject{
public:
	~SphereObject()override{};

	void Init([[maybe_unused]] const std::string &directryPath,const std::string &objectName)override;
	void Update()override;
	void Draw(const IConstantBuffer<CameraTransform>&viewProj)override;
private:
	std::unique_ptr<PrimitiveObject3dMesh> mesh_;

	IConstantBuffer<Transform> transform_;
};