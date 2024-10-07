#include "model/Model.h"
#include "model/ModelManager.h"

#include <cassert>
#include <unordered_map>

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#include "directX12/dxFunctionHelper/DxFunctionHelper.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "texture/TextureManager.h"
#include "System.h"
#include "directX12/dxHeap/DxHeap.h"

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <stdint.h>

#include <chrono>

std::unique_ptr<Matrix4x4> Model::fovMa_ = nullptr;
std::unique_ptr<ModelManager> Model::manager_ = nullptr;

std::array<PipelineStateObj *,kBlendNum> Model::primitivePso_;
std::array<PipelineStateObj *,kBlendNum> Model::texturePso_;
std::unique_ptr<DxCommand> Model::dxCommand_;

struct VertexKey{
	Vector4 position;
	Vector3 normal;
	Vector2 texCoord;

	bool operator==(const VertexKey &other) const{
		return position == other.position &&
			normal == other.normal &&
			texCoord == other.texCoord;
	}
};

namespace std{
	template<>
	struct hash<VertexKey>{
		size_t operator()(const VertexKey &key) const{
			return hash<float>()(key.position.x) ^ hash<float>()(key.position.y) ^ hash<float>()(key.position.z) ^
				hash<float>()(key.normal.x) ^ hash<float>()(key.normal.y) ^ hash<float>()(key.normal.z) ^
				hash<float>()(key.texCoord.x) ^ hash<float>()(key.texCoord.y);
		}
	};
}

//#pragma region"ModelManager"
//class ModelManager{
//public:
//	void Create(std::shared_ptr<Model> &model,const std::string &directoryPath,const std::string &filename);
//	void Init();
//	void Finalize();
//private:
//	void LoadLoop();
//
//	void LoadObjFile(std::vector<std::unique_ptr<ModelData>> &data,const std::string &directoryPath,const std::string &filename);
//	ModelMtl LoadMtlFile(const std::string &directoryPath,const std::string &filename,const std::string &materialName);
//	void ProcessMeshData(std::unique_ptr<ModelData> &modelData,const std::vector<TextureVertexData> &vertices,const std::vector<uint32_t> &indices);
//
//private:
//	std::thread loadingThread_;
//	std::queue<std::tuple<std::weak_ptr<Model>,std::string,std::string>> loadingQueue_;
//	std::mutex queueMutex_;
//	std::condition_variable queueCondition_;
//	bool stopLoadingThread_;
//};
//
//void ModelManager::Create(std::shared_ptr<Model> &model,const std::string &directoryPath,const std::string &filename){
//	{
//		std::unique_lock<std::mutex> lock(queueMutex_);
//		loadingQueue_.emplace(model,directoryPath,filename);
//	}
//	queueCondition_.notify_one();//threadに通知
//}
//
//void ModelManager::Init(){
//	stopLoadingThread_ = false;
//	loadingThread_ = std::thread(&ModelManager::LoadLoop,this);
//}
//
//void ModelManager::Finalize(){
//	{
//		std::unique_lock<std::mutex> lock(queueMutex_);
//		stopLoadingThread_ = true;
//	}
//	queueCondition_.notify_all();
//	if(loadingThread_.joinable()){
//		loadingThread_.join();
//	}
//}
//
//void ModelManager::LoadLoop(){
//	while(true){
//		std::tuple<std::weak_ptr<Model>,std::string,std::string> task;
//		{
//			std::unique_lock<std::mutex> lock(queueMutex_);
//			queueCondition_.wait(lock,[this]{ return !loadingQueue_.empty() || stopLoadingThread_; });
//
//			if(stopLoadingThread_ && loadingQueue_.empty()){
//				return;
//			}
//
//			task = loadingQueue_.front();
//			loadingQueue_.pop();
//		}
//
//		if(auto model = std::get<0>(task).lock()){
//			model->currentState_ = Model::LoadState::Loading;
//			LoadObjFile(model->data_,std::get<1>(task),std::get<2>(task));
//			model->currentState_ = Model::LoadState::Loaded;
//		}
//	}
//}
//
//void ModelManager::LoadObjFile(std::vector<std::unique_ptr<ModelData>> &data,const std::string &directoryPath,const std::string &filename){
//	// 変数の宣言
//	std::vector<Vector4> poss;
//	std::vector<Vector3> normals;
//	std::vector<Vector2> texCoords;
//	std::unordered_map<VertexKey,uint32_t> vertexMap;
//
//	std::vector<TextureVertexData> vertices;
//	std::vector<uint32_t> indices;
//
//	std::string currentMaterial;
//	std::string materialName;
//
//	data.emplace_back(new ModelData());
//	// ファイルを開く
//	std::ifstream file(directoryPath + "/" + filename);
//	assert(file.is_open());
//
//	std::string line;
//	// ファイル読み込み
//	while(std::getline(file,line)){
//		std::string identifier;
//		std::istringstream s(line);
//		s >> identifier;
//
//		if(identifier[0] == 'v'){
//			if(identifier[1] == '\0'){
//				Vector4 pos;
//				s >> pos.x >> pos.y >> pos.z;
//				pos.x *= -1.0f;
//				pos.w = 1.0f;
//				poss.push_back(pos);
//			} else if(identifier[1] == 't'){
//				Vector2 texcoord;
//				s >> texcoord.x >> texcoord.y;
//				texcoord.y = 1.0f - texcoord.y;
//				texCoords.push_back(texcoord);
//			} else if(identifier[1] == 'n'){
//				Vector3 normal;
//				s >> normal.x >> normal.y >> normal.z;
//				normal.x *= -1.0f;
//				normals.push_back(normal);
//			}
//		} else if(identifier[0] == 'f'){
//			VertexKey triangle[3];
//			for(int32_t faceVert = 0; faceVert < 3; ++faceVert){
//				std::string vertDefinition;
//				s >> vertDefinition;
//				std::istringstream v(vertDefinition);
//				uint32_t elementIndices[3];
//
//				for(int32_t element = 0; element < 3; ++element){
//					std::string indexString;
//					std::getline(v,indexString,'/');
//					elementIndices[element] = indexString.empty()?0:std::stoi(indexString);
//				}
//
//				Vector4 position = poss[elementIndices[0] - 1];
//				Vector3 normal = normals[elementIndices[2] - 1];
//				Vector2 texCoord = elementIndices[1] == 0?Vector2{0.0f,0.0f}:texCoords[elementIndices[1] - 1];
//
//				triangle[faceVert] = {position,normal,texCoord};
//			}
//			for(int32_t i = 2; i >= 0; --i){
//				if(vertexMap.find(triangle[i]) == vertexMap.end()){
//					vertexMap[triangle[i]] = static_cast<uint32_t>(vertices.size());
//					vertices.push_back({triangle[i].position,triangle[i].texCoord,triangle[i].normal});
//				}
//				indices.push_back(vertexMap[triangle[i]]);
//			}
//		} else if(identifier[0] == 'm'){ // mtllib
//			s >> currentMaterial;
//		} else if(identifier[0] == 'u'){ // usemtl
//			s >> materialName;
//			data.back()->materialData = LoadMtlFile(directoryPath,currentMaterial,materialName);
//		} else if(identifier[0] == 'o'){
//			if(!vertices.empty() || !indices.empty()){
//				ProcessMeshData(data.back(),vertices,indices);
//				data.back()->material_ = System::getInstance()->getMaterialManager()->Create("white");
//				indices.clear();
//				vertices.clear();
//				data.push_back(std::make_unique<ModelData>());
//			}
//		}
//	}
//
//	// 最後のメッシュデータを処理
//	if(!vertices.empty()){
//		ProcessMeshData(data.back(),vertices,indices);
//		data.back()->material_ = System::getInstance()->getMaterialManager()->Create("white");
//	}
//}
//
//void ModelManager::ProcessMeshData(std::unique_ptr<ModelData> &modelData,const std::vector<TextureVertexData> &vertices,const std::vector<uint32_t> &indices){
//	if(modelData->materialData.textureNumber != nullptr){
//		TextureObject3dMesh *textureMesh = new TextureObject3dMesh();
//
//		modelData->dataSize = sizeof(TextureVertexData) * vertices.size();
//
//		textureMesh->Create(static_cast<UINT>(vertices.size()),static_cast<UINT>(indices.size()));
//		memcpy(textureMesh->vertData,vertices.data(),vertices.size() * sizeof(TextureVertexData));
//		modelData->meshBuff_.reset(textureMesh);
//	} else{
//		PrimitiveObject3dMesh *primitiveMesh = new PrimitiveObject3dMesh();
//
//		std::vector<PrimitiveVertexData> primVert;
//		for(auto &texVert : vertices){
//			primVert.push_back(PrimitiveVertexData(texVert));
//		}
//
//		modelData->dataSize = sizeof(PrimitiveVertexData) * primVert.size();
//
//		primitiveMesh->Create(static_cast<UINT>(primVert.size()),static_cast<UINT>(indices.size()));
//		memcpy(primitiveMesh->vertData,primVert.data(),primVert.size() * sizeof(PrimitiveVertexData));
//		modelData->meshBuff_.reset(primitiveMesh);
//	}
//	memcpy(modelData->meshBuff_->indexData,indices.data(),static_cast<UINT>(static_cast<size_t>(indices.size()) * sizeof(uint32_t)));
//
//	modelData->vertSize = vertices.size();
//	modelData->indexSize = indices.size();
//}
//
//ModelMtl ModelManager::LoadMtlFile(const std::string &directoryPath,const std::string &filename,const std::string &materialName){
//	ModelMtl data{};
//
//	bool isMatchingName = false;
//
//	std::ifstream file(directoryPath + "/" + filename);
//
//	assert(file.is_open());
//
//	std::string line;
//	while(std::getline(file,line)){
//		std::string identifier;
//		std::istringstream s(line);
//		s >> identifier;
//		if(identifier == "newmtl"){
//			std::string mtlName;
//			s >> mtlName;
//			isMatchingName = mtlName == materialName?true:false;
//		} else if(identifier == "map_Kd"){
//			if(!isMatchingName){
//				continue;
//			}
//			std::string textureFilename;
//			s >> textureFilename;
//			data.textureNumber = std::make_unique<int>(TextureManager::LoadTexture(directoryPath + "/" + textureFilename));
//		}
//	}
//	return data;
//}
//#pragma endregion

#pragma region"Model"
Model* Model::Create(const std::string &directoryPath,const std::string &filename){
	return manager_->Create(directoryPath,filename);
}

void Model::Init(){
	manager_ = std::make_unique<ModelManager>();
	manager_->Init();

	fovMa_ = std::make_unique<Matrix4x4>();
	Matrix4x4 *maPtr = new Matrix4x4();
	*maPtr = MakeMatrix::PerspectiveFov(
		0.45f,
		static_cast<float>(System::getInstance()->getWinApp()->getWidth())
		/
		static_cast<float>(System::getInstance()->getWinApp()->getHeight()),
		0.1f,
		100.0f);
	fovMa_.reset(
		maPtr
	);

	dxCommand_ = std::make_unique<DxCommand>();
	dxCommand_->Init(System::getInstance()->getDxDevice()->getDevice(),"main","main");

	size_t index = 0;
	for(auto &primShaderKey : PrimitiveDrawer::getTrianglePsoKeys()){
		primitivePso_[index] = ShaderManager::getInstance()->getPipelineStateObj(primShaderKey);
		index++;
	}

	index = 0;
	for(auto &texShaderKey : System::getInstance()->getTexturePsoKeys()){
		texturePso_[index] = ShaderManager::getInstance()->getPipelineStateObj(texShaderKey);
		index++;
	}
}

void Model::Finalize(){
	manager_->Finalize();
	dxCommand_->Finalize();
}

void Model::DrawThis(const WorldTransform &world,const ViewProjection &view,BlendMode blend){
	auto *commandList = dxCommand_->getCommandList();

	for(auto &model : data_){

		commandList->SetGraphicsRootSignature(texturePso_[static_cast<uint32_t>(blend)]->rootSignature.Get());
		commandList->SetPipelineState(texturePso_[static_cast<uint32_t>(blend)]->pipelineState.Get());
		
		ID3D12DescriptorHeap *ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
		commandList->SetDescriptorHeaps(1,ppHeaps);
		commandList->SetGraphicsRootDescriptorTable(
			4,
			TextureManager::getDescriptorGpuHandle(model->materialData.textureNumber)
		);

		commandList->IASetVertexBuffers(0,1,&model->meshData.meshBuff->vbView);
		commandList->IASetIndexBuffer(&model->meshData.meshBuff->ibView);

		world.SetForRootParameter(commandList,0);
		view.SetForRootParameter(commandList,1);

		model->materialData.material->SetForRootParameter(commandList,2);
		System::getInstance()->getDirectionalLight()->SetForRootParameter(commandList,3);

		// 描画!!!
		commandList->DrawIndexedInstanced(UINT(model->meshData.indexSize),1,0,0,0);
	}
}

void Model::Draw(const WorldTransform &world,const ViewProjection &view,BlendMode blend){
	drawFuncTable_[(size_t)currentState_](world,view,blend);
}
#pragma endregion