#pragma once

#include <array>
#include <memory>
#include <string>
#include <unordered_map>

#include "directX12/DxCommand.h"
#include "directX12/Object3dMesh.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include "material/Material.h"
#include "Matrix4x4.h"
#include "Thread/Thread.h"

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

struct TextureVertexData;
class Object3d;
class ModelManager{
	friend class Object3d;
public:
	static ModelManager* getInstance();
	Model* Create(const std::string& directoryPath,const std::string& filename);
	void Init();

	void PreDraw();

	void Finalize();
private:
	std::array<PipelineStateObj*,kBlendNum> texturePso_;
	std::unique_ptr<DxCommand> dxCommand_;

	std::unique_ptr<Matrix4x4> fovMa_;
	// TODOタスク 
	// ModelManager* manager を どうにかする
	struct LoadTask{
		std::string directory;
		std::string fileName ;
		Model* model = nullptr;
		void Update();
	};
private:
	std::unique_ptr<TaskThread<LoadTask>> loadThread_;

	std::unordered_map<std::string,std::unique_ptr<Model>> modelLibrary_;
};
