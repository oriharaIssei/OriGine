#include "ModelManager.h"

#include "Engine.h"
#include "Model.h"
#include "directX12/ShaderManager.h"
#include "material/Texture/TextureManager.h"
#include "model/Model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cassert>

//===========================================================================
// unorderedMap 用
//===========================================================================
struct VertexKey {
    Vector4 position;
    Vector3 normal;
    Vector2 texCoord;

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
        return hash<float>()(key.position.x) ^ hash<float>()(key.position.y) ^ hash<float>()(key.position.z) ^
               hash<float>()(key.normal.x) ^ hash<float>()(key.normal.y) ^ hash<float>()(key.normal.z) ^
               hash<float>()(key.texCoord.x) ^ hash<float>()(key.texCoord.y);
    }
};
} // namespace std

ModelManager* ModelManager::getInstance() {
    static ModelManager instance{};
    return &instance;
}

std::unique_ptr<Model> ModelManager::Create(
    const std::string& directoryPath,
    const std::string& filename,
    std::function<void()> callBack) {
    std::unique_ptr<Model> result = std::make_unique<Model>();

    const auto itr = modelLibrary_.find(directoryPath + filename);
    if (itr != modelLibrary_.end()) {
        result->currentState_ = Model::LoadState::Loaded;
        result->meshData_     = itr->second.get();
        result->materialData_ = defaultMaterials_[result->meshData_];
        return result;
    }

    modelLibrary_[directoryPath + filename] = std::make_unique<ModelMeshData>();

    result            = std::make_unique<Model>();
    result->meshData_ = modelLibrary_[directoryPath + filename].get();
    loadThread_->pushTask({directoryPath, filename, result.get(), callBack});

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

void ModelManager::Finalize() {
    loadThread_->Finalize();
    dxCommand_->Finalize();
    modelLibrary_.clear();
}

void ModelManager::pushBackDefaultMaterial(ModelMeshData* key, Material3D material) {
    defaultMaterials_[key].emplace_back(material);
}

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

        // transform の作成
        mesh.transform_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

        aiMesh* loadedMesh = scene->mMeshes[meshIndex];
        assert(loadedMesh->HasNormals() && loadedMesh->HasTextureCoords(0));

        // 頂点データとインデックスデータの処理
        for (uint32_t faceIndex = 0; faceIndex < loadedMesh->mNumFaces; ++faceIndex) {
            aiFace& face = loadedMesh->mFaces[faceIndex];
            assert(face.mNumIndices == 3); // 三角形面のみを扱う

            for (uint32_t i = 0; i < 3; ++i) {
                uint32_t vertexIndex = face.mIndices[i];

                // 頂点データを取得
                Vector4 pos      = {loadedMesh->mVertices[vertexIndex].x, loadedMesh->mVertices[vertexIndex].y, loadedMesh->mVertices[vertexIndex].z, 1.0f};
                Vector3 normal   = {loadedMesh->mNormals[vertexIndex].x, loadedMesh->mNormals[vertexIndex].y, loadedMesh->mNormals[vertexIndex].z};
                Vector2 texCoord = {loadedMesh->mTextureCoords[0][vertexIndex].x, loadedMesh->mTextureCoords[0][vertexIndex].y};

                // X軸反転
                pos.x *= -1.0f;
                normal.x *= -1.0f;

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
            // filepath が localなら directoryPath と結合
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

void ModelManager::LoadTask::Update() {
    model->currentState_ = Model::LoadState::Loading;

    LoadModelFile(model->meshData_, this->directory, this->fileName);
    model->materialData_ = ModelManager::getInstance()->defaultMaterials_[model->meshData_];

    if (callBack) {
        callBack();
    }

    model->currentState_ = Model::LoadState::Loaded;
}
