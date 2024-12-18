#include "ModelManager.h"
#include "model/Model.h"

#include <cassert>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "directX12/ShaderManager.h"
#include "material/Texture/TextureManager.h"
#include "Model.h"
#include "Engine.h"

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

std::unique_ptr<Model> ModelManager::Create(const std::string& directoryPath,const std::string& filename){
	std::unique_ptr<Model> result = std::make_unique<Model>();
	const auto itr = modelLibrary_.find(directoryPath + filename);
	if(itr != modelLibrary_.end()){
		result->currentState_ = Model::LoadState::Loaded;
		result->meshData_ = itr->second.get();
		result->materialData_ = defaultMaterials_[result->meshData_];
		return result;
	}

	modelLibrary_[directoryPath + filename] = std::make_unique<ModelMeshData>();

	result = std::make_unique<Model>();
	result->meshData_ = modelLibrary_[directoryPath + filename].get();
	loadThread_->pushTask({directoryPath,filename,result.get()});

	return result;
}

void ModelManager::Init(){
	loadThread_ = std::make_unique<TaskThread<ModelManager::LoadTask>>();
	loadThread_->Init(1);

	fovMa_ = std::make_unique<Matrix4x4>();
	Matrix4x4* maPtr = new Matrix4x4();
	*maPtr = MakeMatrix::PerspectiveFov(
		0.45f,
		static_cast<float>(Engine::getInstance()->getWinApp()->getWidth())
		/
		static_cast<float>(Engine::getInstance()->getWinApp()->getHeight()),
		0.1f,
		100.0f);
	fovMa_.reset(
		maPtr
	);

	dxCommand_ = std::make_unique<DxCommand>();
	dxCommand_->Init(Engine::getInstance()->getDxDevice()->getDevice(),"main","main");

	size_t index = 0;

	for(auto& texShaderKey : Engine::getInstance()->getTexturePsoKeys()){
		texturePso_[index] = ShaderManager::getInstance()->getPipelineStateObj(texShaderKey);
		index++;
	}
}

void ModelManager::Finalize(){
	loadThread_->Finalize();
	dxCommand_->Finalize();
	modelLibrary_.clear();
}

void ModelManager::pushBackDefaultMaterial(ModelMeshData* key,Material3D material){
	defaultMaterials_[key].emplace_back(material);
}

void ProcessMeshData(Mesh3D& meshData,const std::vector<TextureVertexData>& vertices,const std::vector<uint32_t>& indices){
	TextureObject3dMesh* textureMesh = new TextureObject3dMesh();

	meshData.dataSize = static_cast<int32_t>(sizeof(TextureVertexData) * vertices.size());

	textureMesh->Create(static_cast<UINT>(vertices.size()),static_cast<UINT>(indices.size()));
	memcpy(textureMesh->vertData,vertices.data(),vertices.size() * sizeof(TextureVertexData));
	meshData.meshBuff.reset(textureMesh);

	memcpy(meshData.meshBuff->indexData,indices.data(),static_cast<UINT>(static_cast<size_t>(indices.size()) * sizeof(uint32_t)));

	meshData.vertSize
		= static_cast<int32_t>(vertices.size());
	meshData.indexSize = static_cast<int32_t>(indices.size());
}

ModelNode ReadNode(aiNode* node){
	ModelNode result;
	/// LocalMatrix の 取得
	aiMatrix4x4 aiLocalMatrix = node->mTransformation;
	/// 列ベクトル を 行ベクトル に
	aiLocalMatrix.Transpose();
	/// localMatrix を Copy
	result.localMatrix[0][0] = aiLocalMatrix[0][0];
	result.localMatrix[0][1] = aiLocalMatrix[0][1];
	result.localMatrix[0][2] = aiLocalMatrix[0][2];
	result.localMatrix[0][3] = aiLocalMatrix[0][3];

	result.localMatrix[1][0] = aiLocalMatrix[1][0];
	result.localMatrix[1][1] = aiLocalMatrix[1][1];
	result.localMatrix[1][2] = aiLocalMatrix[1][2];
	result.localMatrix[1][3] = aiLocalMatrix[1][3];

	result.localMatrix[2][0] = aiLocalMatrix[2][0];
	result.localMatrix[2][1] = aiLocalMatrix[2][1];
	result.localMatrix[2][2] = aiLocalMatrix[2][2];
	result.localMatrix[2][3] = aiLocalMatrix[2][3];

	result.localMatrix[3][0] = aiLocalMatrix[3][0];
	result.localMatrix[3][1] = aiLocalMatrix[3][1];
	result.localMatrix[3][2] = aiLocalMatrix[3][2];
	result.localMatrix[3][3] = aiLocalMatrix[3][3];

	/// Name を Copy
	result.name = node->mName.C_Str();

	/// Children を Copy
	result.children.resize(node->mNumChildren);

	/// Children すべてを Copy
	for(uint32_t childIndex = 0; childIndex < node->mNumChildren; childIndex++){
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}

	return result;
}

void BuildMeshNodeMap(aiNode* node,std::unordered_map<unsigned int,std::string>& meshNodeMap){
	// 現在のノードが参照するすべてのメッシュに対してノード名を記録
	for(unsigned int i = 0; i < node->mNumMeshes; ++i){
		meshNodeMap[node->mMeshes[i]] = node->mName.C_Str();
	}

	// 子ノードを再帰的に処理
	for(unsigned int i = 0; i < node->mNumChildren; ++i){
		BuildMeshNodeMap(node->mChildren[i],meshNodeMap);
	}
}

std::unordered_map<unsigned int,std::string> CreateMeshNodeMap(const aiScene* scene){
	std::unordered_map<unsigned int,std::string> meshNodeMap;
	BuildMeshNodeMap(scene->mRootNode,meshNodeMap);
	return meshNodeMap;
}

void LoadModelFile(ModelMeshData* data,const std::string& directoryPath,const std::string& filename){
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(),aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes());

	std::unordered_map<VertexKey,uint32_t> vertexMap;
	std::vector<TextureVertexData> vertices;
	std::vector<uint32_t> indices;

	// ノードとメッシュの対応表を作成
	std::unordered_map<unsigned int,std::string> meshNodeMap = CreateMeshNodeMap(scene);

	/// node 読み込み
	data->rootNode = ReadNode(scene->mRootNode);

	for(uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex){
		auto& mesh = data->mesh_.emplace_back(Mesh3D());

		// transform の作成
		mesh.transform_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

		aiMesh* loadedMesh = scene->mMeshes[meshIndex];
		assert(loadedMesh->HasNormals() && loadedMesh->HasTextureCoords(0));

		// 頂点データとインデックスデータの処理
		for(uint32_t faceIndex = 0; faceIndex < loadedMesh->mNumFaces; ++faceIndex){
			aiFace& face = loadedMesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);  // 三角形面のみを扱う

			for(uint32_t i = 0; i < 3; ++i){
				uint32_t vertexIndex = face.mIndices[i];

				// 頂点データを取得
				Vector4 pos = {loadedMesh->mVertices[vertexIndex].x,loadedMesh->mVertices[vertexIndex].y,loadedMesh->mVertices[vertexIndex].z,1.0f};
				Vector3 normal = {loadedMesh->mNormals[vertexIndex].x,loadedMesh->mNormals[vertexIndex].y,loadedMesh->mNormals[vertexIndex].z};
				Vector2 texCoord = {loadedMesh->mTextureCoords[0][vertexIndex].x,loadedMesh->mTextureCoords[0][vertexIndex].y};

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

		// メッシュに対応するノード名を設定
		mesh.nodeName = meshNodeMap[meshIndex];

		// マテリアルとテクスチャの処理
		aiMaterial* material = scene->mMaterials[loadedMesh->mMaterialIndex];
		aiString textureFilePath;
		uint32_t textureIndex;
		if(material->GetTexture(aiTextureType_DIFFUSE,0,&textureFilePath) == AI_SUCCESS){
			std::string texturePath = directoryPath + "/" + textureFilePath.C_Str();
			textureIndex = TextureManager::LoadTexture(texturePath);
		} else{
			textureIndex = 0;
		}

		ModelManager::getInstance()->pushBackDefaultMaterial(data,{
			textureIndex,
			Engine::getInstance()->getMaterialManager()->Create("white")
															 });

		// メッシュデータを処理
		ProcessMeshData(mesh,vertices,indices);

		// リセット
		vertices.clear();
		indices.clear();
		vertexMap.clear();
	}
}

Animation ModelManager::LoadAnimation(const std::string& directoryPath,
									  const std::string& filename){
	Animation result;
	Assimp::Importer importer;

	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(),0);

	// アニメーションがなかったら assert
	assert(scene->mNumAnimations != 0);

	aiAnimation* animationAssimp = scene->mAnimations[0];
	/// 時間の単位を 秒 に 合わせる
	// mTicksPerSecond ： 周波数
	// mDuration      : mTicksPerSecond で 指定された 周波数 における長さ
	result.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);

	for(uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex){
		aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		NodeAnimation& nodeAnimation = result.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];

		///=============================================
		/// Scale 解析
		///=============================================
		for(uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex){
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			KeyframeVector3 keyframe;
			// 時間単位を 秒 に変換
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			// スケール値をそのまま使用
			keyframe.value = {keyAssimp.mValue.x,keyAssimp.mValue.y,keyAssimp.mValue.z};
			nodeAnimation.scale.push_back(keyframe);
		}

		///=============================================
		/// Rotate 解析
		///=============================================
		for(uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex){
			aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			KeyframeQuaternion keyframe;
			// 時間単位を 秒 に変換
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			// クォータニオンの値を変換 (右手座標系 → 左手座標系)
			keyframe.value = {
				keyAssimp.mValue.x,
				-keyAssimp.mValue.y,
				-keyAssimp.mValue.z,
				keyAssimp.mValue.w
			};
			nodeAnimation.rotate.push_back(keyframe);
		}

		///=============================================
		/// Translate 解析
		///=============================================
		for(uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex){
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
			KeyframeVector3 keyframe;
			// 時間単位を 秒 に変換
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			// 元が 右手座標系 なので 左手座標系 に 変換する
			keyframe.value = {-keyAssimp.mValue.x,keyAssimp.mValue.y,keyAssimp.mValue.z};
			nodeAnimation.translate.push_back(keyframe);
		}
	}

	return result;
}

void ModelManager::LoadTask::Update(){
	model->currentState_ = Model::LoadState::Loading;

	LoadModelFile(model->meshData_,this->directory,this->fileName);
	model->materialData_ = ModelManager::getInstance()->defaultMaterials_[model->meshData_];

	model->currentState_= Model::LoadState::Loaded;
}
