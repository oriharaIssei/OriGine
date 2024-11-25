#pragma once

#include <array>
#include <memory>
#include <string>
#include <unordered_map>

#include "directX12/DxCommand.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include "Matrix4x4.h"
#include "Thread/Thread.h"

class Object3d;
struct Model;
class ModelManager{
	friend class Object3d;
public:
	static ModelManager* getInstance();
	Model* Create(const std::string& directoryPath,const std::string& filename);
	void Init();

	void Finalize();
private:
	std::array<PipelineStateObj*,kBlendNum> texturePso_;
	std::unique_ptr<DxCommand> dxCommand_;

	std::unique_ptr<Matrix4x4> fovMa_;
	struct LoadTask{
		std::string directory;
		std::string fileName;
		Model* model = nullptr;
		void Update();
	};
private:
	std::unique_ptr<TaskThread<LoadTask>> loadThread_;

	std::unordered_map<std::string,std::unique_ptr<Model>> modelLibrary_;
};
