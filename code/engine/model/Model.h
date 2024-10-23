#pragma once

#include <memory>
#include <vector>

#include <fstream>
#include <sstream>

#include "directX12/dxCommand/DxCommand.h"
#include "directX12/shaderManager/ShaderManager.h"
#include <directX12/PipelineStateObj.h>

#include "directX12/buffer/Material.h"
#include <directX12/buffer/Object3dMesh.h>
#include <directX12/buffer/CameraBuffer.h>
#include "directX12/buffer/TransformBuffer.h"

struct ModelMaterial{
	int textureNumber;

	Material* material;
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
	static void Init();
	static void Finalize();
private:
	static std::unique_ptr<ModelManager> manager_;

	static std::array<PipelineStateObj *,kBlendNum> primitivePso_;
	static std::array<PipelineStateObj *,kBlendNum> texturePso_;

	static std::unique_ptr<DxCommand> dxCommand_;

	static std::unique_ptr<Matrix4x4> fovMa_;
public:
	Model() = default;
	~Model(){
		drawFuncTable_.fill(nullptr);
	}

	void Debug();

	void Draw(const TransformBuffer &world,const CameraBuffer &view,[[maybe_unused]] BlendMode blend = BlendMode::Normal);
private:
	void NotDraw([[maybe_unused]] const TransformBuffer &world,[[maybe_unused]] const CameraBuffer &view,[[maybe_unused]] BlendMode blend = BlendMode::Normal){}

	void DrawThis(const TransformBuffer &world,const CameraBuffer &view,BlendMode blend = BlendMode::Normal);
private:
	std::vector<std::unique_ptr<ModelData>> data_;
	enum class LoadState{
		Loading,
		Loaded,
	};
	LoadState currentState_;
	std::array<std::function<void(const TransformBuffer &,const CameraBuffer &,BlendMode)>,2> drawFuncTable_ = {
		std::bind(&Model::NotDraw,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3),
		std::bind(&Model::DrawThis,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)
	};
public:
	const std::vector<std::unique_ptr<ModelData>> &getData()const{ return data_; }
	void setMaterial(Material *material,uint32_t index = 0){
		data_[index]->materialData.material = material;
	}
};