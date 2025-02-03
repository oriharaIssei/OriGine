#include "ModelManager.h"

///stl
//assert
#include <cassert>

///engine
#include "Engine.h"
//assetes
#include "Model.h"
//dx12Object
#include "directX12/ShaderManager.h"
#include "material/Texture/TextureManager.h"

/// lib
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

//===========================================================================
// unorderedMap 用
//===========================================================================
struct VertexKey {
    Vec4f position;
    Vec3f normal;
    Vec2f texCoord;

    bool operator==(const VertexKey& other) const {
        return position == other.position &&
               normal == other.normal &&
               texCoord == other.texCoord;
    }
};
namespace std {
template <>
struct hash<VertexKey> {
    size_t operator()(const VertexKey& key) const {
        return hash<float>()(key.position[X]) ^ hash<float>()(key.position[Y]) ^ hash<float>()(key.position[Z]) ^
               hash<float>()(key.normal[X]) ^ hash<float>()(key.normal[Y]) ^ hash<float>()(key.normal[Z]) ^
               hash<float>()(key.texCoord[X]) ^ hash<float>()(key.texCoord[Y]);
    }
};
} // namespace std

#pragma region "LoadFunctions"
void ProcessMeshData(Mesh3D& meshData, const std::vector<TextureVertexData>& vertices, const std::vector<uint32_t>& indices) {
    TextureObject3dMesh* textureMesh = new TextureObject3dMesh();

    meshData.dataSize = static_cast<int32_t>(sizeof(TextureVertexData) * vertices.size());

    textureMesh->Create(static_cast<UINT>(vertices.size()), static_cast<UINT>(indices.size()));
    memcpy(textureMesh->vertData, vertices.data(), vertices.size() * sizeof(TextureVertexData));
    meshData.meshBuff.reset(textureMesh);

    memcpy(meshData.meshBuff->indexData, indices.data(), static_cast<UINT>(static_cast<size_t>(indices.size()) * sizeof(uint32_t)));

    meshData.vertSize  = static_cast<int32_t>(vertices.size());
    meshData.indexSize = static_cast<int32_t>(indices.size());
}

ModelNode ReadNode(aiNode* node) {
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
    for (uint32_t childIndex = 0; childIndex < node->mNumChildren; childIndex++) {
        result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
    }

    return result;
}

void BuildMeshNodeMap(aiNode* node, std::unordered_map<unsigned int, std::string>& meshNodeMap) {
    // 現在のノードが参照するすべてのメッシュに対してノード名を記録
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        meshNodeMap[node->mMeshes[i]] = node->mName.C_Str();
    }

    // 子ノードを再帰的に処理
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        BuildMeshNodeMap(node->mChildren[i], meshNodeMap);
    }
}

std::unordered_map<unsigned int, std::string> CreateMeshNodeMap(const aiScene* scene) {
    std::unordered_map<unsigned int, std::string> meshNodeMap;
    BuildMeshNodeMap(scene->mRootNode, meshNodeMap);
    return meshNodeMap;
}

void LoadModelFile(ModelMeshData* data, const std::string& directoryPath, const std::string& filename) {
    Assimp::Importer importer;
    std::string filePath = directoryPath + "/" + filename;
    const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
    assert(scene->HasMeshes());

    std::unordered_map<VertexKey, uint32_t> vertexMap;
    std::vector<TextureVertexData> vertices;
    std::vector<uint32_t> indices;

    // ノードとメッシュの対応表を作成
    std::unordered_map<unsigned int, std::string> meshNodeMap = CreateMeshNodeMap(scene);
    for (const auto& [nodeIndex, nodeName] : meshNodeMap) {
        data->meshIndexes[nodeName] = nodeIndex;
    }

    /// node 読み込み
    data->rootNode = ReadNode(scene->mRootNode);

    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        auto& mesh = data->mesh_.emplace_back(Mesh3D());

        aiMesh* loadedMesh = scene->mMeshes[meshIndex];

        // 頂点データとインデックスデータの処理
        for (uint32_t faceIndex = 0; faceIndex < loadedMesh->mNumFaces; ++faceIndex) {
            aiFace& face = loadedMesh->mFaces[faceIndex];
            assert(face.mNumIndices == 3); // 三角形面のみを扱う

            for (uint32_t i = 0; i < 3; ++i) {
                uint32_t vertexIndex = face.mIndices[i];

                // 頂点データを取得
                Vec4f pos    = {loadedMesh->mVertices[vertexIndex][X], loadedMesh->mVertices[vertexIndex][Y], loadedMesh->mVertices[vertexIndex][Z], 1.0f};
                Vec3f normal = {0.f, 0.f, 0.f};
                if (loadedMesh->HasNormals()) {
                    normal = {loadedMesh->mNormals[vertexIndex][X], loadedMesh->mNormals[vertexIndex][Y], loadedMesh->mNormals[vertexIndex][Z]};
                } else {
                    normal = {pos[X], pos[Y], -pos[Z]};
                    normal = Vec3f::Normalize(normal);
                }
                Vec2f texCoord = {0.f, 0.f};
                if (loadedMesh->HasTextureCoords(0)) {
                    texCoord = {loadedMesh->mTextureCoords[0][vertexIndex][X], loadedMesh->mTextureCoords[0][vertexIndex][Y]};
                }

                // X軸反転
                pos[X] *= -1.0f;
                normal[X] *= -1.0f;

                // VertexKeyを生成
                VertexKey vertexKey = {pos, normal, texCoord};

                // vertexMapに存在するか確認し、無ければ追加
                if (vertexMap.find(vertexKey) == vertexMap.end()) {
                    vertexMap[vertexKey] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back({pos, texCoord, normal});
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
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath) == AI_SUCCESS) {
            std::string texturePath = textureFilePath.C_Str();

            if (texturePath.find("/") == std::string::npos) {
                texturePath = directoryPath + "/" + texturePath;
            }
            textureIndex = TextureManager::LoadTexture(texturePath);
        } else {
            textureIndex = 0;
        }

        ModelManager::getInstance()->pushBackDefaultMaterial(data, {textureIndex, Engine::getInstance()->getMaterialManager()->Create("white")});

        // メッシュデータを処理
        ProcessMeshData(mesh, vertices, indices);

        // リセット
        vertices.clear();
        indices.clear();
        vertexMap.clear();
    }
}

#pragma endregion

ModelManager* ModelManager::getInstance() {
    static ModelManager instance{};
    return &instance;
}

std::shared_ptr<Model> ModelManager::Create(
    const std::string& directoryPath,
    const std::string& filename,
    std::function<void(Model*)> callBack) {
    std::shared_ptr<Model> result = std::make_shared<Model>();

    std::string filePath = directoryPath + "/" + filename;

    const auto itr = modelLibrary_.find(filePath);
    // すでに読み込まれている場合
    if (itr != modelLibrary_.end()) {
        auto* targetModelMesh = itr->second.get();
        while (true) {
            if (targetModelMesh->currentState_ == LoadState::Loaded) {
                break;
            }
        }
        result->meshData_     = targetModelMesh;
        result->materialData_ = defaultMaterials_[result->meshData_];

        for (auto& mesh : result->meshData_->mesh_) {
            result->transformBuff_[&mesh].CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

            result->transformBuff_[&mesh].openData_.UpdateMatrix();
            result->transformBuff_[&mesh].ConvertToBuffer();
        }

        if (callBack != nullptr) {
            callBack(result.get());
        }

        return result;
    }

    /// モデルデータを読み込む
    modelLibrary_[filePath] = std::make_unique<ModelMeshData>();

    result            = std::make_unique<Model>();
    result->meshData_ = modelLibrary_[filePath].get();
    loadThread_->pushTask(
        {.directory = directoryPath,
         .fileName  = filename,
         .model     = result,
         .callBack  = callBack});

    return result;
}

void ModelManager::Init() {
    loadThread_ = std::make_unique<TaskThread<ModelManager::LoadTask>>();
    loadThread_->Init(1);

    fovMa_           = std::make_unique<Matrix4x4>();
    Matrix4x4* maPtr = new Matrix4x4();
    *maPtr           = MakeMatrix::PerspectiveFov(
        0.45f,
        static_cast<float>(Engine::getInstance()->getWinApp()->getWidth()) /
            static_cast<float>(Engine::getInstance()->getWinApp()->getHeight()),
        0.1f,
        100.0f);
    fovMa_.reset(
        maPtr);

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Init(Engine::getInstance()->getDxDevice()->getDevice(), "main", "main");

    size_t index = 0;

    for (auto& texShaderKey : Engine::getInstance()->getTexturePsoKeys()) {
        texturePso_[index] = ShaderManager::getInstance()->getPipelineStateObj(texShaderKey);
        index++;
    }
}

void ModelManager::StartUpLoad() {
    /// モデルデータを読み込む
    std::vector<std::pair<std::string, std::string>> filePaths = {
        {"resource/Models", "BattleField.obj"},
        {"resource/Models", "Skydome.obj"},
        {"resource/Models", "Player.obj"},
        {"resource/Models", "Enemy.obj"}};
    std::vector<std::shared_ptr<Model>> results;

    std::string filePath;
    for (const auto& [directory, filename] : filePaths) {
        filePath = directory + "/" + filename;

        auto& result = results.emplace_back(std::make_shared<Model>());

        modelLibrary_[filePath] = std::make_unique<ModelMeshData>();

        result->meshData_ = modelLibrary_[filePath].get();
        loadThread_->pushTask(
            {.directory = directory,
             .fileName  = filename,
             .model     = result,
             .callBack  = nullptr});
    }
}

void ModelManager::Finalize() {
    loadThread_->Finalize();
    dxCommand_->Finalize();
    modelLibrary_.clear();
}

void ModelManager::pushBackDefaultMaterial(ModelMeshData* key, Material3D material) {
    defaultMaterials_[key].emplace_back(material);
}

void ModelManager::LoadTask::Update() {
    model->meshData_->currentState_ = LoadState::Unloaded;

    try {
        LoadModelFile(model->meshData_, this->directory, this->fileName);
    } catch (const std::exception& e) {
        // エラーハンドリング
        std::cerr << "Error loading model file: " << e.what() << std::endl;
        return;
    }

    model->materialData_ = ModelManager::getInstance()->defaultMaterials_[model->meshData_];

    auto device = Engine::getInstance()->getDxDevice()->getDevice();
    std::mutex mutex;
    for (auto& mesh : model->meshData_->mesh_) {
        try {
            std::lock_guard<std::mutex> lock(mutex);
            model->transformBuff_[&mesh] = IConstantBuffer<Transform>();
            model->transformBuff_[&mesh].CreateBuffer(device);
            model->transformBuff_[&mesh].openData_.UpdateMatrix();
            model->transformBuff_[&mesh].ConvertToBuffer();
        } catch (const std::exception& e) {
            // エラーハンドリング
            std::cerr << "Error creating or updating buffer: " << e.what() << std::endl;
            return;
        }
    }

    if (callBack != nullptr) {
        callBack(model.get());
    }

    model->meshData_->currentState_ = LoadState::Loaded;
}
