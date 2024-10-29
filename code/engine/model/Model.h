#pragma once

#include <memory>
#include <vector>

#include <fstream>
#include <sstream>

#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include "transform/CameraTransform.h"
#include "directX12/IConstantBuffer.h"
#include "material/Material.h"
#include "transform/Transform.h"

#include "directX12/Object3dMesh.h"

struct ModelMaterial{
	int textureNumber;
	IConstantBuffer<Material>* material;
};

struct ModelMeshData{
	std::unique_ptr<IObject3dMesh> meshBuff;
	size_t dataSize  = 0;
	size_t vertSize  = 0;
	size_t indexSize = 0;
};

struct ModelData{
	ModelMeshData meshData;
	ModelMaterial materialData;
};

class ModelManager;
class Model{
	friend class ModelManager;
public:
	static Model* Create(const std::string &directoryPath,const std::string &filename);
public:
	Model() = default;
	~Model(){transform_.Finalize();}

	IConstantBuffer<Transform> transform_;

	void Draw(const IConstantBuffer<CameraTransform>& view,[[maybe_unused]] BlendMode blend = BlendMode::Normal);
private:
	void NotDraw([[maybe_unused]] const IConstantBuffer<CameraTransform>& view,[[maybe_unused]] BlendMode blend = BlendMode::Normal){}

	void DrawThis(const IConstantBuffer<CameraTransform>& view,BlendMode blend = BlendMode::Normal);
private:
	std::vector<std::unique_ptr<ModelData>> data_;
	enum class LoadState{
		Loading,
		Loaded,
	};
	LoadState currentState_;
	std::array<std::function<void(const IConstantBuffer<CameraTransform>& ,BlendMode)>,2> drawFuncTable_ = {
		[this](const IConstantBuffer<CameraTransform>& view,BlendMode blend){ NotDraw(view,blend); },
		[this](const IConstantBuffer<CameraTransform>& view,BlendMode blend = BlendMode::Alpha){ DrawThis(view,blend); }
	};
public:
	const std::vector<std::unique_ptr<ModelData>> &getData()const{ return data_; }
	void setMaterial(IConstantBuffer<Material>* material,uint32_t index = 0){
		data_[index]->materialData.material = material;
	}
};