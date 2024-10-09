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
#include <directX12/buffer/ViewProjection.h>
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

	void Debug();

	void Draw(const TransformBuffer &world,const ViewProjection &view,[[maybe_unused]] BlendMode blend = BlendMode::Normal);
private:
	void NotDraw([[maybe_unused]] const TransformBuffer &world,[[maybe_unused]] const ViewProjection &view,[[maybe_unused]] BlendMode blend = BlendMode::Normal){}

	void DrawThis(const TransformBuffer &world,const ViewProjection &view,BlendMode blend = BlendMode::Normal);
private:
	std::vector<std::unique_ptr<ModelData>> data_;
	enum class LoadState{
		Loading,
		Loaded,
	};
	LoadState currentState_;
	std::array<std::function<void(const TransformBuffer &,const ViewProjection &,BlendMode)>,2> drawFuncTable_ = {
		[this](const TransformBuffer &world,const ViewProjection &view,BlendMode blend){ NotDraw(world,view,blend); },
		[this](const TransformBuffer &world,const ViewProjection &view,BlendMode blend = BlendMode::Alpha){ DrawThis(world,view,blend); }
	};
public:
	const std::vector<std::unique_ptr<ModelData>> &getData()const{ return data_; }
	void setMaterial(Material *material,uint32_t index = 0){
		data_[index]->materialData.material = material;
	}
};