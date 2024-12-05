#pragma once

#include <memory>
#include <vector>

#include "directX12/IConstantBuffer.h"
#include "directX12/Object3dMesh.h"
#include "material/Material.h"

struct Material3D{
	uint32_t textureNumber;
	IConstantBuffer<Material>* material;
};
struct Mesh3D{
	std::unique_ptr<IObject3dMesh> meshBuff;
	size_t dataSize  = 0;
	size_t vertSize  = 0;
	size_t indexSize = 0;
};
struct ModelMeshData{
	std::vector<Mesh3D> mesh_;
};
struct Model{
	enum class LoadState{
		Loading,
		Loaded,
	};
	LoadState currentState_ = LoadState::Loading;

	ModelMeshData* meshData_;

	using ModelMaterialData =  std::vector<Material3D>;
	ModelMaterialData materialData_;

	void setMaterialBuff(int32_t part,IConstantBuffer<Material>* buff){
		materialData_[part].material = buff;
	}
	void setTexture(int32_t part,uint32_t textureNumber){
		materialData_[part].textureNumber = textureNumber;
	}
};