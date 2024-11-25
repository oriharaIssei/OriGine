#include "ModelManager.h"

#include <cassert>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "directX12/ShaderManager.h"
#include "material/TextureManager.h"
#include "Model.h"
#include "System.h"

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

ModelManager* ModelManager::getInstance(){
	static ModelManager instance{};
	return &instance;
}

Model* ModelManager::Create(const std::string& directoryPath,const std::string& filename){
	const auto itr = modelLibrary_.find(directoryPath + filename);
	if(itr != modelLibrary_.end()){
		return itr->second.get();
	}

	modelLibrary_[directoryPath + filename] = std::make_unique<Model>();

	loadThread_->pushTask({directoryPath,filename,modelLibrary_[directoryPath + filename].get()});

	return modelLibrary_[directoryPath + filename].get();
}

void ModelManager::Init(){
	loadThread_ = std::make_unique<TaskThread<ModelManager::LoadTask>>();
	loadThread_->Init(1);

	fovMa_ = std::make_unique<Matrix4x4>();
	Matrix4x4* maPtr = new Matrix4x4();
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

	for(auto& texShaderKey : System::getInstance()->getTexturePsoKeys()){
		texturePso_[index] = ShaderManager::getInstance()->getPipelineStateObj(texShaderKey);
		index++;
	}
}

void ModelManager::Finalize(){
	loadThread_->Finalize();
	dxCommand_->Finalize();
	modelLibrary_.clear();
}

void ProcessMeshData(ModelData& modelData,const std::vector<TextureVertexData>& vertices,const std::vector<uint32_t>& indices){
	TextureObject3dMesh* textureMesh = new TextureObject3dMesh();

	modelData.meshData.dataSize = static_cast<int32_t>(sizeof(TextureVertexData) * vertices.size());

	textureMesh->Create(static_cast<UINT>(vertices.size()),static_cast<UINT>(indices.size()));
	memcpy(textureMesh->vertData,vertices.data(),vertices.size() * sizeof(TextureVertexData));
	modelData.meshData.meshBuff.reset(textureMesh);

	memcpy(modelData.meshData.meshBuff->indexData,indices.data(),static_cast<UINT>(static_cast<size_t>(indices.size()) * sizeof(uint32_t)));

	modelData.meshData.vertSize
		= static_cast<int32_t>(vertices.size());
	modelData.meshData.indexSize = static_cast<int32_t>(indices.size());
}

void LoadObjFile(std::vector<ModelData>& data,const std::string& directoryPath,const std::string& filename){
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(),aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes());

	std::unordered_map<VertexKey,uint32_t> vertexMap;
	std::vector<TextureVertexData> vertices;
	std::vector<uint32_t> indices;

	for(uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex){
		data.emplace_back(ModelData());
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals() && mesh->HasTextureCoords(0));

		// 頂点データとインデックスデータの処理を統合
		for(uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex){
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);  // 三角形面のみを扱う

			for(uint32_t i = 0; i < 3; ++i){
				uint32_t vertexIndex = face.mIndices[i];

				// 頂点データを取得
				Vector4 pos = {mesh->mVertices[vertexIndex].x,mesh->mVertices[vertexIndex].y,mesh->mVertices[vertexIndex].z,1.0f};
				Vector3 normal = {mesh->mNormals[vertexIndex].x,mesh->mNormals[vertexIndex].y,mesh->mNormals[vertexIndex].z};
				Vector2 texCoord = {mesh->mTextureCoords[0][vertexIndex].x,mesh->mTextureCoords[0][vertexIndex].y};

				// X軸反転
				pos.x *= -1.0f;
				normal.x *= -1.0f;

				// VertexKeyを生成
				VertexKey vertexKey = {pos,normal,texCoord};

				// vertexMapに存在するか確認し、無ければ追加
				if(vertexMap.find(vertexKey) == vertexMap.end()){
					vertexMap[vertexKey] = static_cast<uint32_t>(vertices.size());
					vertices.push_back({pos,texCoord,normal});
				}

				// インデックスを追加
				indices.push_back(vertexMap[vertexKey]);
			}
		}

		// マテリアルとテクスチャの処理
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		aiString textureFilePath;
		if(material->GetTexture(aiTextureType_DIFFUSE,0,&textureFilePath) == AI_SUCCESS){
			std::string texturePath = directoryPath + "/" + textureFilePath.C_Str();
			data.back().materialData.textureNumber = TextureManager::LoadTexture(texturePath);
		} else{
			data.back().materialData.textureNumber = 0;
		}
		// マテリアル名の設定（仮）
		data.back().materialData.material = System::getInstance()->getMaterialManager()->Create("white");

		// メッシュデータを処理
		ProcessMeshData(data.back(),vertices,indices);

		// リセット
		vertices.clear();
		indices.clear();
		vertexMap.clear();
	}
}

void ModelManager::LoadTask::Update(){
	model->currentState_ = Model::LoadState::Loading;
	LoadObjFile(model->data_,this->directory,this->fileName);
	model->currentState_ = Model::LoadState::Loaded;
}