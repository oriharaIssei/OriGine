#include "ModelManager.h"

/// stl
// assert
#include <cassert>

/// engine
#include "Engine.h"
#include "winApp/WinApp.h"
// assetes
#include "Model.h"
// dx12Object
#include "directX12/DxDevice.h"
#include "directX12/ShaderManager.h"
#include "texture/TextureManager.h"


#include "logger/Logger.h"

/// util
#include "util/NormalizeString.h"

/// externals
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
        return position == other.position && normal == other.normal && texCoord == other.texCoord;
    }
};
namespace std {
template <>
struct hash<VertexKey> {
    size_t operator()(const VertexKey& key) const {
        return hash<float>()(key.position[X]) ^ hash<float>()(key.position[Y]) ^ hash<float>()(key.position[Z]) ^ hash<float>()(key.normal[X]) ^ hash<float>()(key.normal[Y]) ^ hash<float>()(key.normal[Z]) ^ hash<float>()(key.texCoord[X]) ^ hash<float>()(key.texCoord[Y]);
    }
};
} // namespace std

#pragma region "LoadFunctions"
static void ProcessMeshData(TextureMesh& meshData, const std::vector<TextureVertexData>& vertices, const std::vector<uint32_t>& indices) {

    meshData.Initialize(static_cast<UINT>(vertices.size()), static_cast<UINT>(indices.size()));

    // 頂点データのコピー
    meshData.copyVertexData(vertices.data(), static_cast<uint32_t>(vertices.size()));
    // インデックスデータのコピー
    meshData.copyIndexData(indices.data(), static_cast<uint32_t>(indices.size()));

    meshData.TransferData();
}

static ModelNode ReadNode(aiNode* node) {
    ModelNode result;
    /// Transform の取得
    aiVector3D aiScale, aiTranslate;
    aiQuaternion aiRotate;
    node->mTransformation.Decompose(aiScale, aiRotate, aiTranslate);
    result.transform.scale     = Vec3f(aiScale.x, aiScale.y, aiScale.z);
    result.transform.rotate    = Quaternion(aiRotate.x, -aiRotate.y, -aiRotate.z, aiRotate.w); // X軸反転
    result.transform.translate = Vec3f(-aiTranslate.x, aiTranslate.y, aiTranslate.z); // X軸反転
    result.localMatrix         = MakeMatrix::Affine(result.transform.scale, result.transform.rotate, result.transform.translate);

    /// Name を Copy
    result.name = node->mName.C_Str();

    /// Children を Copy
    result.children.resize(node->mNumChildren);
    for (uint32_t childIndex = 0; childIndex < node->mNumChildren; childIndex++) {
        result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
    }

    return result;
}

static int32_t CreateJoint(
    const ModelNode& node,
    const std::optional<int32_t>& parent,
    std::vector<Joint>& joints) {
    Joint joint;

    joint.name  = node.name;
    joint.index = static_cast<int32_t>(joints.size());

    joint.transform           = node.transform;
    joint.localMatrix         = node.localMatrix;
    joint.skeletonSpaceMatrix = MakeMatrix::Identity();

    joint.parent = parent;

    joints.push_back(joint);

    for (const ModelNode& child : node.children) {
        // 子ジョイントを再帰的に作成, そのインデックスを登録
        int32_t childIndex = CreateJoint(child, joint.index, joints);
        joints[joint.index].children.push_back(childIndex);
    }

    return joint.index;
}

static Skeleton CreateSkeleton(const ModelNode& rootNode) {
    Skeleton skeleton;
    skeleton.rootJointIndex = CreateJoint(rootNode, {}, skeleton.joints);

    // 名前とIndex を バインド
    for (const Joint& joint : skeleton.joints) {
        skeleton.jointIndexBinder.emplace(joint.name, joint.index);
    }

    skeleton.Update();

    return skeleton;
}

static void CreateSkinCluster(
    SkinCluster& _cluster,
    const Microsoft::WRL::ComPtr<ID3D12Device>& _device,
    aiMesh* _loadedMesh,
    ModelMeshData* _meshData) {

    // skinClusterData を初期化
    for (uint32_t boneIndex = 0; boneIndex < _loadedMesh->mNumBones; ++boneIndex) {
        aiBone* bone                     = _loadedMesh->mBones[boneIndex];
        JointWeightData& jointWeightData = _meshData->jointWeightData[bone->mName.C_Str()];

        /// バインドポーズ座標系での逆行列を設定
        // decompose
        aiMatrix4x4 bindPoseMatAssimp = bone->mOffsetMatrix.Inverse();
        aiVector3D scale, translate;
        aiQuaternion rotate;
        bindPoseMatAssimp.Decompose(scale, rotate, translate);
        // X軸反転 して Decomposeした値から 計算
        Matrix4x4 bindPoseMatrix = MakeMatrix::Affine(
            Vec3f(scale.x, scale.y, scale.z),
            Quaternion(rotate.x, -rotate.y, -rotate.z, rotate.w),
            Vec3f(-translate.x, translate.y, translate.z));
        // 逆行列を 保持
        jointWeightData.inverseBindPoseMat = bindPoseMatrix.inverse();

        /// 頂点ウェイトの設定
        jointWeightData.vertexWeights.resize(bone->mNumWeights);
        for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
            aiVertexWeight& vertexWeight = bone->mWeights[weightIndex];
            VertexWeightData& weightData = jointWeightData.vertexWeights[weightIndex];
            weightData.weight            = vertexWeight.mWeight;
            weightData.vertexIndex       = vertexWeight.mVertexId;
        }
    }

    // SkinClusterData 作成
    const Skeleton& skeleton = _meshData->skeleton.value();

    // skeletonMatrixPalette Buffer 作成
    _cluster.skeletonMatrixPaletteBuffer_.CreateBuffer(_device, uint32_t(skeleton.joints.size()));
    _cluster.skeletonMatrixPaletteBuffer_.openData_.resize(skeleton.joints.size());

    // influence Buffer 作成
    _cluster.vertexInfluencesBuffer_.CreateBuffer(_device, _loadedMesh->mNumVertices);
    _cluster.vertexInfluencesBuffer_.openData_.resize(_loadedMesh->mNumVertices);

    // inverseBindPoseMatrices を 初期化(単位行列で埋めとく)
    _cluster.inverseBindPoseMatrices.resize(skeleton.joints.size());
    std::generate(_cluster.inverseBindPoseMatrices.begin(), _cluster.inverseBindPoseMatrices.end(), MakeMatrix::Identity);

    // ModelData を 解析, Influence を 設定
    for (const auto& jointWeight : _meshData->jointWeightData) {
        // skeletonに 対応するジョイントが存在するか確認
        auto jointIndexItr = skeleton.jointIndexBinder.find(jointWeight.first);

        // 存在しない場合はスキップ
        if (jointIndexItr == skeleton.jointIndexBinder.end()) {
            continue;
        }

        // Jointのインデックスに対応する場所に inverseBindePoseMatrix を代入
        _cluster.inverseBindPoseMatrices[jointIndexItr->second] = jointWeight.second.inverseBindPoseMat;
        for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
            auto& currentInfluence = _cluster.vertexInfluencesBuffer_.openData_[vertexWeight.vertexIndex];
            for (uint32_t i = 0; i < kNumMaxInfluence; ++i) {
                // 空いているウェイトの場所に設定 (weight == 0 なら設定されていないとみなす)
                if (currentInfluence.weights[i] == 0.f) {
                    currentInfluence.weights[i]      = vertexWeight.weight;
                    currentInfluence.jointIndices[i] = (*jointIndexItr).second;
                    break;
                }
            }
        }
    }

    _cluster.skinningInfoBuffer_.CreateBuffer(_device);
    _cluster.skinningInfoBuffer_.openData_.vertexSize = _loadedMesh->mNumVertices;

    _cluster.skeletonMatrixPaletteBuffer_.ConvertToBuffer();
    _cluster.vertexInfluencesBuffer_.ConvertToBuffer();
    _cluster.skinningInfoBuffer_.ConvertToBuffer();
}

static void LoadModelFile(ModelMeshData* data, const std::string& directoryPath, const std::string& filename) {
    Assimp::Importer importer;
    std::string filePath = directoryPath + "/" + filename;
    const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
    assert(scene->HasMeshes());

    auto& device = Engine::getInstance()->getDxDevice()->getDeviceRef();

    std::unordered_map<VertexKey, uint32_t> vertexMap;
    std::vector<TextureVertexData> vertices;
    std::vector<uint32_t> indices;

    /// node 読み込み
    data->rootNode = ReadNode(scene->mRootNode);
    // スケルトンの作成
    data->skeleton = CreateSkeleton(data->rootNode);

    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        aiMesh* loadedMesh = scene->mMeshes[meshIndex];

        auto& mesh = data->meshGroup[loadedMesh->mName.C_Str()] = TextureMesh();
        mesh.setName(loadedMesh->mName.C_Str());

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

        // マテリアルとテクスチャの処理
        aiMaterial* material = scene->mMaterials[loadedMesh->mMaterialIndex];
        aiString textureFilePath;
        uint32_t textureIndex   = 0;
        std::string texturePath = "";
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath) == AI_SUCCESS) {
            texturePath = textureFilePath.C_Str();
            if ((texturePath.find("/") == std::string::npos)) {
                texturePath = directoryPath + "/" + texturePath;
            }
            textureIndex = TextureManager::LoadTexture(texturePath);
        }

        ModelManager::getInstance()->pushBackDefaultMaterial(data, {texturePath, textureIndex, IConstantBuffer<Material>()});

        // メッシュデータを処理
        ProcessMeshData(mesh, vertices, indices);

        CreateSkinCluster(data->skinClusterDataMap[mesh.getName()], device, loadedMesh, data);

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
    std::shared_ptr<Model> result = std::make_unique<Model>();

    std::string filePath = normalizeString(directoryPath + "/" + filename);

    LOG_TRACE("Load Model \n Path : {}", filePath);

    const auto itr = modelLibrary_.find(filePath);
    // すでに読み込まれている場合
    if (itr != modelLibrary_.end()) {
        LOG_TRACE("Model already loaded: {}", filePath);

        auto* targetModelMesh = itr->second.get();

        result->meshData_     = targetModelMesh;
        result->materialData_ = defaultMaterials_[result->meshData_];

        for (auto& materialData : result->materialData_) {
            materialData.material.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
            materialData.material->UpdateUvMatrix();
            materialData.material.ConvertToBuffer();
        }

        for (auto& [name, data] : result->meshData_->meshGroup) {
            result->transforms_[&data].UpdateMatrix();
        }

        if (callBack != nullptr) {
            callBack(result.get());
        }

        return result;
    }

    /// モデルデータを読み込む
    modelLibrary_[filePath] = std::make_unique<ModelMeshData>();
    result                  = std::make_unique<Model>();
    result->meshData_       = modelLibrary_[filePath].get();
#ifdef _DEBUG
    /* loadThread_->pushTask(
         {.directory   = directoryPath,
             .fileName = filename,
             .model    = result,
             .callBack = callBack});*/
    LoadTask task;
    task.directory = directoryPath;
    task.fileName  = filename;
    task.model     = result;
    task.callBack  = callBack;
    task.Update();
#else
    LoadTask task;
    task.directory = directoryPath;
    task.fileName  = filename;
    task.model     = result;
    task.callBack  = callBack;
    task.Update();
#endif // _DEBUG

    return result;
}

void ModelManager::Initialize() {
    /*loadThread_ = std::make_unique<TaskThread<ModelManager::LoadTask>>();
    loadThread_->Initialize(1);*/

    fovMa_           = std::make_unique<Matrix4x4>();
    Matrix4x4* maPtr = new Matrix4x4();
    *maPtr           = MakeMatrix::PerspectiveFov(
        0.45f,
        static_cast<float>(Engine::getInstance()->getWinApp()->getWidth()) / static_cast<float>(Engine::getInstance()->getWinApp()->getHeight()),
        0.1f,
        100.0f);
    fovMa_.reset(
        maPtr);

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
}

void ModelManager::Finalize() {
    /*loadThread_->Finalize();*/
    dxCommand_->Finalize();
    modelLibrary_.clear();
}

void ModelManager::pushBackDefaultMaterial(ModelMeshData* key, TexturedMaterial material) {
    defaultMaterials_[key].emplace_back(material);
}

ModelMeshData* ModelManager::getModelMeshData(const std::string& directoryPath, const std::string& filename) {
    std::string filePath = normalizeString(directoryPath + "/" + filename);
    auto it              = modelLibrary_.find(filePath);
    if (it != modelLibrary_.end()) {
        return it->second.get();
    }
    return nullptr;
}

void ModelManager::LoadTask::Update() {
    DeltaTime timer;
    timer.Initialize();

    try {
        LoadModelFile(model->meshData_, this->directory, this->fileName);
    } catch (const std::exception& e) {
        // エラーハンドリング
        std::cerr << "Error loading model file: " << e.what() << std::endl;
        return;
    }

    model->materialData_ = ModelManager::getInstance()->defaultMaterials_[model->meshData_];
    for (auto& material : model->materialData_) {
        material.material.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
        material.material->UpdateUvMatrix();
        material.material.ConvertToBuffer();
    }

    std::mutex mutex;
    for (auto& [name, data] : model->meshData_->meshGroup) {
        try {
            std::lock_guard<std::mutex> lock(mutex);
            model->transforms_[&data] = Transform();
            model->transforms_[&data].UpdateMatrix();
        } catch (const std::exception& e) {
            // エラーハンドリング
            std::cerr << "Error creating or updating buffer: " << e.what() << std::endl;
            return;
        }
    }

    // ロード完了後にコールバックを呼び出す
    if (callBack != nullptr) {
        callBack(model.get());
    }

    // ロード完了のログ
    timer.Update();
    LOG_TRACE("Model Load Complete : {}/{} \n Lading Time : {}", this->directory, this->fileName, std::to_string(timer.getDeltaTime()));
}
