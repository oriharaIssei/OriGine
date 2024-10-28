#pragma once

#include <memory>

#include <list>

#include <string>

#include "directX12/buffer/CameraTransform.h"
#include "directX12/buffer/IConstantBuffer.h"
#include "directX12/buffer/Material.h"
#include "directX12/buffer/Transform.h"

class IGameObject{
public:
	virtual ~IGameObject(){};

	virtual void Init(const std::string &directryPath,const std::string &objectName);
	virtual void Update();
	virtual void Draw(const IConstantBuffer<CameraTransform>& viewProj) = 0;
protected:
	static std::list<std::pair<std::string,std::string>> textureList_;
	int currentTextureNum_;

	MaterialManager *materialManager_;
	[[maybe_unused]] IConstantBuffer<Material>* material_;

	[[maybe_unused]] IConstantBuffer<Transform> transform_;

	std::vector<const char *> materialNameVector_;
	std::vector<int> checkedMaterial_;

	std::string name_;
public:
	void setMaterialManager(MaterialManager *materialManager){ materialManager_ = materialManager; }
	const std::string &getName()const{ return name_; }
};