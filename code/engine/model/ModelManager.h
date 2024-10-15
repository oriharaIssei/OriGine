#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Thread/Thread.h"

class Model;
struct ModelMaterial;
struct ModelData;
struct TextureVertexData;

class ModelManager{
public:
	Model* Create(const std::string& directoryPath,const std::string& filename);
	void Init();
	void Finalize();
private:
	
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
