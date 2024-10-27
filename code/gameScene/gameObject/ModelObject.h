#pragma once
#include "IGameObject.h"

#include "directX12/buffer/Transform.h"
#include "model/Model.h"

class ModelObject :
	public IGameObject{
public:
	~ModelObject()override{};
	void Init(const std::string &directryPath,const std::string &objectName)override;
	void Update()override;
	void Draw(const IConstantBuffer<CameraTransform>&viewProj)override;
private:
	Model* model_;
};