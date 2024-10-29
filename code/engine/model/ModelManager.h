#pragma once

#include <array>
#include <memory>
#include <string>
#include <unordered_map>

#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include "Thread/Thread.h"

#include "Matrix4x4.h"

class Model;
struct ModelMaterial;
struct ModelData;
struct TextureVertexData;

class ModelManager{
	friend class Model;
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
