#pragma once

#include <memory>
#include <vector>

#include "directX12/IConstantBuffer.h"
#include "directX12/Object3dMesh.h"
#include "material/Material.h"

struct Material3D{
	int textureNumber;
	IConstantBuffer<Material>* material;
};
struct Mesh3D{
	std::unique_ptr<IObject3dMesh> meshBuff;
	size_t dataSize  = 0;
	size_t vertSize  = 0;
	size_t indexSize = 0;
};
struct ModelData{
	Mesh3D meshData;
	Material3D materialData;
};
struct Model{
	enum class LoadState{
		Loading,
		Loaded,
	};
	std::vector<ModelData> data_;
	LoadState currentState_;
};