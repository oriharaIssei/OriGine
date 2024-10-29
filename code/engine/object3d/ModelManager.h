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

class Object3d;
struct Material3D;
struct Model;
struct TextureVertexData;

class ModelManager{
	friend class Object3d;
public:
	static ModelManager* getInstance();
	Object3d* Create(const std::string& directoryPath,const std::string& filename);
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
		Object3d* model = nullptr;
		void Update();
	};
private:
	std::unique_ptr<TaskThread<LoadTask>> loadThread_;

	std::unordered_map<std::string,std::unique_ptr<Object3d>> modelLibrary_;
};
