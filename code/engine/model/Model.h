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
#include <directX12/buffer/WorldTransform.h>

struct ModelMtl{
	/// <summary>
	/// mtl file などから読み込んだ情報を保存するためのもの
	/// </summary>
	std::unique_ptr<int> textureNumber = nullptr;
};
struct ModelData{
	std::unique_ptr<IObject3dMesh> meshBuff_;

	ModelMtl materialData;

	size_t dataSize;
	size_t vertSize;
	size_t indexSize;

	Material *material_;
};

class ModelManager;
class Model{
	friend class ModelManager;
public:
	static std::shared_ptr<Model> Create(const std::string &directoryPath,const std::string &filename);
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

	void Draw(const WorldTransform &world,const ViewProjection &view,[[maybe_unused]] BlendMode blend = BlendMode::Normal);
private:
	void NotDraw([[maybe_unused]] const WorldTransform &world,[[maybe_unused]] const ViewProjection &view,[[maybe_unused]] BlendMode blend = BlendMode::Normal){}

	void DrawThis(const WorldTransform &world,const ViewProjection &view,BlendMode blend = BlendMode::Normal);
private:
	std::vector<std::unique_ptr<ModelData>> data_;
	enum class LoadState{
		Loading,
		Loaded,
	};
	LoadState currentState_;
	std::array<std::function<void(const WorldTransform &,const ViewProjection &,BlendMode)>,2> drawFuncTable_ = {
		[this](const WorldTransform &world,const ViewProjection &view,BlendMode blend){ NotDraw(world,view,blend); },
		[this](const WorldTransform &world,const ViewProjection &view,BlendMode blend = BlendMode::Alpha){ DrawThis(world,view,blend); }
	};
public:
	const std::vector<std::unique_ptr<ModelData>> &getData()const{ return data_; }
	void setMaterial(Material *material,uint32_t index = 0){
		data_[index]->material_ = material;
	}
};