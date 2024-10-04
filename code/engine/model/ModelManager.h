#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <mutex>
#include <queue>
#include <thread>

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
	void LoadLoop();

	void LoadObjFile(std::vector<std::unique_ptr<ModelData>>* data,const std::string& directoryPath,const std::string& filename);
	ModelMaterial LoadMtlFile(const std::string& directoryPath,const std::string& filename,const std::string& materialName);
	void ProcessMeshData(std::unique_ptr<ModelData>& modelData,const std::vector<TextureVertexData>& vertices,const std::vector<uint32_t>& indices);

private:
	std::thread loadingThread_;
	std::queue<std::pair<std::string,std::string>> loadingQueue_;
	std::mutex queueMutex_;
	std::condition_variable queueCondition_;
	bool stopLoadingThread_;

	std::unordered_map<std::string,std::unique_ptr<Model>> modelLibrary_;
};
