#pragma once

#include "IGameObject.h"

#include <memory>

#include "directX12/buffer/CameraTransform.h"
#include "directX12/buffer/Object3dMesh.h"
#include "directX12/buffer/Transform.h"

class SphereObject :
	public IGameObject{
public:
	~SphereObject()override{};

	void Init([[maybe_unused]] const std::string &directryPath,const std::string &objectName)override;
	void Update()override;
	void Draw(const IConstantBuffer<CameraTransform>&viewProj)override;
private:
	std::unique_ptr<PrimitiveObject3dMesh> mesh_;
};