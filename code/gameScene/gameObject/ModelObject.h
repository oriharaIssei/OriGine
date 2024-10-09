#pragma once
#include "IGameObject.h"

#include "model/Model.h"
#include "directX12/buffer/TransformBuffer.h"

class ModelObject :
	public IGameObject{
public:
	~ModelObject()override{};
	void Init(const std::string &directryPath,const std::string &objectName)override;
	void Update()override;
	void Draw(const ViewProjection &viewProj)override;
private:
	Model* model_;
};