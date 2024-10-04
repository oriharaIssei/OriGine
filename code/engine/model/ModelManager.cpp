#include "ModelManager.h"

#include "System.h"
#include "texture/TextureManager.h"

#include "model/Model.h"

//===========================================================================
// unorderedMap 用
//===========================================================================
struct VertexKey{
	Vector4 position;
	Vector3 normal;
	Vector2 texCoord;

	bool operator==(const VertexKey& other) const{
		return position == other.position &&
			normal == other.normal &&
			texCoord == other.texCoord;
	}
};
namespace std{
	template<>
	struct hash<VertexKey>{
		size_t operator()(const VertexKey& key) const{
			return hash<float>()(key.position.x) ^ hash<float>()(key.position.y) ^ hash<float>()(key.position.z) ^
				hash<float>()(key.normal.x) ^ hash<float>()(key.normal.y) ^ hash<float>()(key.normal.z) ^
				hash<float>()(key.texCoord.x) ^ hash<float>()(key.texCoord.y);
		}
	};
}

Model* ModelManager::Create(const std::string& directoryPath,const std::string& filename){
	const auto itr = modelLibrary_.find(directoryPath + filename);
	if(itr != modelLibrary_.end()){
		return itr->second.get();
	}

	modelLibrary_[directoryPath + filename] = std::make_unique<Model>();

	{
		std::unique_lock<std::mutex> lock(queueMutex_);
		loadingQueue_.emplace(directoryPath,filename);
	}
	queueCondition_.notify_one();//threadに通知
	return modelLibrary_[directoryPath + filename].get();
}

void ModelManager::Init(){
	stopLoadingThread_ = false;
	loadingThread_ = std::thread(&ModelManager::LoadLoop,this);
}

void ModelManager::Finalize(){
	{
		std::unique_lock<std::mutex> lock(queueMutex_);
		stopLoadingThread_ = true;
	}

	queueCondition_.notify_all();
	if(loadingThread_.joinable()){
		loadingThread_.join();
	}

	modelLibrary_.clear();
}

void ModelManager::LoadLoop(){
	while(true){
		std::pair<std::string,std::string> task;
		{
			std::unique_lock<std::mutex> lock(queueMutex_);
			queueCondition_.wait(lock,[this]{ return !loadingQueue_.empty() || stopLoadingThread_; });

			if(stopLoadingThread_ && loadingQueue_.empty()){
				return;
			}

			task = loadingQueue_.front();
			loadingQueue_.pop();
		}

		{
			std::lock_guard guard(queueMutex_);

			Model* model = modelLibrary_[std::get<0>(task) + std::get<1>(task)].get();

			model->currentState_ = Model::LoadState::Loading;
			LoadObjFile(&model->data_,std::get<0>(task),std::get<1>(task));
			model->currentState_ = Model::LoadState::Loaded;
		}
	}
}

void ModelManager::LoadObjFile(std::vector<std::unique_ptr<ModelData>>* data,const std::string& directoryPath,const std::string& filename){
	// 変数の宣言
	std::vector<Vector4> poss;
	std::vector<Vector3> normals;
	std::vector<Vector2> texCoords;
	std::unordered_map<VertexKey,uint32_t> vertexMap;

	std::vector<TextureVertexData> vertices;
	std::vector<uint32_t> indices;

	std::string currentMaterial;
	std::string materialName;

	data->emplace_back(new ModelData());
	// ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	std::string line;
	// ファイル読み込み
	while(std::getline(file,line)){
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if(identifier[0] == 'v'){
			if(identifier[1] == '\0'){
				Vector4 pos;
				s >> pos.x >> pos.y >> pos.z;
				pos.x *= -1.0f;
				pos.w = 1.0f;
				poss.push_back(pos);
			} else if(identifier[1] == 't'){
				Vector2 texcoord;
				s >> texcoord.x >> texcoord.y;
				texcoord.y = 1.0f - texcoord.y;
				texCoords.push_back(texcoord);
			} else if(identifier[1] == 'n'){
				Vector3 normal;
				s >> normal.x >> normal.y >> normal.z;
				normal.x *= -1.0f;
				normals.push_back(normal);
			}
		} else if(identifier[0] == 'f'){
			VertexKey triangle[3];
			for(int32_t faceVert = 0; faceVert < 3; ++faceVert){
				std::string vertDefinition;
				s >> vertDefinition;
				std::istringstream v(vertDefinition);
				uint32_t elementIndices[3];

				for(int32_t element = 0; element < 3; ++element){
					std::string indexString;
					std::getline(v,indexString,'/');
					elementIndices[element] = indexString.empty()?0:std::stoi(indexString);
				}

				Vector4 position = poss[elementIndices[0] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				Vector2 texCoord = elementIndices[1] == 0?Vector2{0.0f,0.0f}:texCoords[elementIndices[1] - 1];

				triangle[faceVert] = {position,normal,texCoord};
			}
			for(int32_t i = 2; i >= 0; --i){
				if(vertexMap.find(triangle[i]) == vertexMap.end()){
					vertexMap[triangle[i]] = static_cast<uint32_t>(vertices.size());
					vertices.push_back({triangle[i].position,triangle[i].texCoord,triangle[i].normal});
				}
				indices.push_back(vertexMap[triangle[i]]);
			}
		} else if(identifier[0] == 'm'){ // mtllib
			s >> currentMaterial;
		} else if(identifier[0] == 'u'){ // usemtl
			s >> materialName;
			data->back()->materialData = LoadMtlFile(directoryPath,currentMaterial,materialName);
		} else if(identifier[0] == 'o'){
			if(!vertices.empty() || !indices.empty()){
				ProcessMeshData(data->back(),vertices,indices);
				data->back()->materialData.material = 
					System::getInstance()->getMaterialManager()->Create("white");
				indices.clear();
				vertices.clear();
				data->push_back(std::make_unique<ModelData>());
			}
		}
	}

	// 最後のメッシュデータを処理
	if(!vertices.empty()){
		ProcessMeshData(data->back(),vertices,indices);
		data->back()->materialData.material = System::getInstance()->getMaterialManager()->Create("white");
	}
}

void ModelManager::ProcessMeshData(std::unique_ptr<ModelData>& modelData,const std::vector<TextureVertexData>& vertices,const std::vector<uint32_t>& indices){
	TextureObject3dMesh* textureMesh = new TextureObject3dMesh();

	modelData->meshData.dataSize = static_cast<int32_t>(sizeof(TextureVertexData) * vertices.size());

	textureMesh->Create(static_cast<UINT>(vertices.size()),static_cast<UINT>(indices.size()));
	memcpy(textureMesh->vertData,vertices.data(),vertices.size() * sizeof(TextureVertexData));
	modelData->meshData.meshBuff.reset(textureMesh);
	
	memcpy(modelData->meshData.meshBuff->indexData,indices.data(),static_cast<UINT>(static_cast<size_t>(indices.size()) * sizeof(uint32_t)));

	modelData->meshData.vertSize = static_cast<int32_t>(vertices.size());
	modelData->meshData.indexSize = static_cast<int32_t>(indices.size());
}

ModelMaterial ModelManager::LoadMtlFile(const std::string& directoryPath,const std::string& filename,const std::string& materialName){
	ModelMaterial data{};

	bool isMatchingName = false;

	std::ifstream file(directoryPath + "/" + filename);

	assert(file.is_open());

	std::string line;
	while(std::getline(file,line)){
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;
		if(identifier == "newmtl"){
			std::string mtlName;
			s >> mtlName;
			isMatchingName = mtlName == materialName?true:false;
		} else if(identifier == "map_Kd"){
			if(!isMatchingName){
				continue;
			}
			std::string textureFilename;
			s >> textureFilename;
			data.textureNumber = TextureManager::LoadTexture(directoryPath + "/" + textureFilename);
		}
	}
	return data;
}