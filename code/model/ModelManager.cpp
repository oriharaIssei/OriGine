#include "ModelManager.h"

/// engine
#include "Engine.h"
// asset
#include "asset/AssetSystem.h"
#include "asset/manager/ModelAssetManager.h"
#include "Model.h"
// dx12Object
#include "directX12/DxDevice.h"

/// util
#include "logger/Logger.h"
#include "util/StringUtil.h"

using namespace OriGine;

namespace {
/// <summary>
/// ModelAssetManager を取得する（未登録なら nullptr）.
/// </summary>
AssetManager<ModelAsset>* GetModelAssetManager() {
    return AssetSystem::GetInstance()->GetManager<ModelAsset>();
}
} // namespace

ModelManager* ModelManager::GetInstance() {
    static ModelManager instance{};
    return &instance;
}

/// <summary>
/// ディレクトリとファイル名から、AssetManager に渡すアセットパスを組み立てる.
/// AssetManager はパス文字列をキャッシュのキーにするため、
/// 同じファイルが別表記で二重ロードされないよう正規化しておく.
/// </summary>
std::string ModelManager::MakeAssetPath(const std::string& _directoryPath, const std::string& _filename) {
    return NormalizeString(_directoryPath + "/" + _filename);
}

/// <summary>
/// モデルアセットをロード（またはキャッシュから取得）し、Model インスタンスを作成する.
/// </summary>
std::shared_ptr<Model> ModelManager::Create(
    const std::string& _directoryPath,
    const std::string& _filename,
    std::function<void(Model*)> _callBack) {

    ModelMeshData* meshData = GetModelMeshData(_directoryPath, _filename);
    if (meshData == nullptr) {
        LOG_ERROR("Failed to create Model. path : {}", MakeAssetPath(_directoryPath, _filename));
        return nullptr;
    }

    auto result       = std::make_shared<Model>();
    result->meshData_ = meshData;

    // マテリアルは共有アセットの既定値を複製したうえで、
    // インスタンスごとに GPU バッファを作り直す
    // （同じモデルを複数配置したときに個別の色を設定できるようにするため）
    result->materialData_ = meshData->defaultMaterials;
    for (auto& materialData : result->materialData_) {
        materialData.material.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
        materialData.material->UpdateUvMatrix();
        materialData.material.ConvertToBuffer();
    }

    for (auto& [name, data] : meshData->meshGroup) {
        result->transforms_[&data] = Transform();
        result->transforms_[&data].UpdateMatrix();
    }

    if (_callBack != nullptr) {
        _callBack(result.get());
    }

    return result;
}

void ModelManager::Initialize() {}

void ModelManager::Finalize() {}

/// <summary>
/// モデルデータを取得する. 未ロードの場合はここでロードを行う.
/// </summary>
ModelMeshData* ModelManager::GetModelMeshData(const std::string& _directoryPath, const std::string& _filename) {
    auto* manager = GetModelAssetManager();
    if (manager == nullptr) {
        LOG_ERROR("ModelAssetManager is not registered.");
        return nullptr;
    }

    const std::string assetPath = MakeAssetPath(_directoryPath, _filename);

    // 既に読み込み済みなら参照カウントを増やさずにそのまま返す。
    // 未読み込みならここでロードする（AssetManager 側がキャッシュを持つ）
    size_t index = manager->GetAssetIndex(assetPath);
    if (index == kInvalidAssetIndex) {
        LOG_TRACE("Load Model \n Path : {}", assetPath);
        index = manager->LoadAsset(assetPath);
    }

    ModelAsset* asset = manager->GetMutableAsset(index);
    return asset ? &asset->meshData : nullptr;
}

const std::vector<TexturedMaterial>& ModelManager::GetDefaultMaterials(ModelMeshData* _key) const {
    static const std::vector<TexturedMaterial> empty;
    return _key ? _key->defaultMaterials : empty;
}

const std::vector<TexturedMaterial>& ModelManager::GetDefaultMaterials(const std::string& _directoryPath, const std::string& _filename) {
    return GetDefaultMaterials(GetModelMeshData(_directoryPath, _filename));
}
