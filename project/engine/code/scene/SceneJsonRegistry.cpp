#include "SceneJsonRegistry.h"

/// stl
#include <fstream>

/// engine
#include "scene/SceneFactory.h"

/// util
#include "logger/Logger.h"
#include "myFileSystem/MyFileSystem.h"

using namespace OriGine;

SceneJsonRegistry* OriGine::SceneJsonRegistry::GetInstance() {
    static SceneJsonRegistry inst;
    return &inst;
}

/// <summary>
/// 現在レジストリに保持しているすべてのシーン JSON データを指定ディレクトリに保存する.
/// </summary>
bool SceneJsonRegistry::SaveAllScene(const std::string& _directory) {
    myfs::CreateFolder(_directory);
    for (auto& [name, jsonData] : scenes_) {
        std::ofstream ofs(_directory + "/" + name + ".json");
        if (!ofs) {
            LOG_CRITICAL("シーンJSONの保存に失敗しました: {}", name);
            return false;
        }
        ofs << std::setw(4) << jsonData;
    }
    return true;
}

/// <summary>
/// 指定された実行中のシーンをシリアライズし、即座にファイルとして保存する.
/// </summary>
bool OriGine::SceneJsonRegistry::SaveScene(const Scene* _scene, const std::string& _directory) {

    if (!_scene) {
        LOG_ERROR("SaveScene: scene が nullptr");
        return false;
    }
    SceneFactory factory;
    nlohmann::json sceneJson = factory.CreateSceneJsonFromScene(const_cast<Scene*>(_scene));

    std::string path = _directory + "/" + _scene->GetName() + ".json";
    std::ofstream ofs(path);
    if (!ofs) {
        LOG_CRITICAL("シーンJSONの保存に失敗: {}", _scene->GetName());
        return false;
    }

    ofs << std::setw(4) << sceneJson;
    return true;
}

/// <summary>
/// 指定ディレクトリ配下から .json ファイルを検索し、すべてシーンデータとしてロードする.
/// </summary>
bool SceneJsonRegistry::LoadAllScene(const std::string& _directory) {
    // すべての .json を読む
    for (auto& [directory, filename] : myfs::SearchFile(_directory, "json")) {
        std::ifstream ifs(_directory + "/" + filename + ".json");
        if (ifs) {
            nlohmann::json data;
            ifs >> data;
            scenes_[filename] = data;
        }
    }
    return true;
}

/// <summary>
/// 単体ファイルから特定のシーンデータをロードする.
/// </summary>
bool OriGine::SceneJsonRegistry::LoadScene(const std::string& _sceneName, const std::string& _directory) {
    std::string path = _directory + "/" + _sceneName + ".json";
    std::ifstream ifs(path);
    if (!ifs) {
        LOG_ERROR("LoadScene: 読込失敗: {}", path);
        return false;
    }

    nlohmann::json data;
    ifs >> data;
    scenes_[_sceneName] = data;
    return true;
}

/// <summary>
/// メモリ上に空のシーンデータ定義を作成する.
/// システムやエンティティの配列が空の状態で初期化される.
/// </summary>
void OriGine::SceneJsonRegistry::CreateNewScene(const std::string& _sceneName) {
    nlohmann::json empty;
    empty["Systems"]          = nlohmann::json::array();
    empty["CategoryActivity"] = nlohmann::json::array();
    empty["Entities"]         = nlohmann::json::array();

    scenes_[_sceneName] = empty;
}

/// <summary>
/// 拡張子 .ent (kEntityExtension) を持つテンプレートファイルを一括でロードする.
/// </summary>
bool OriGine::SceneJsonRegistry::LoadAllEntityTemplates(const std::string& _directory) {

    // すべての .json を読む
    for (auto& [directory, filename] : myfs::SearchFile(_directory, kEntityExtension)) {
        std::ifstream ifs(_directory + "/" + filename + '.' + kEntityExtension);
        if (ifs) {
            nlohmann::json data;
            ifs >> data;
            entityTemplates_[filename] = data;
        }
    }
    return true;
}

/// <summary>
/// 指定ディレクトリから特定のエンティティテンプレートをロードする.
/// </summary>
bool OriGine::SceneJsonRegistry::LoadEntityTemplate(const std::string& _directory, const std::string& _typeName) {
    std::string path = _directory + "/" + _typeName + '.' + kEntityExtension;
    std::ifstream ifs(path);
    if (!ifs) {
        LOG_ERROR(" 読込失敗: {}", path);
        return false;
    }
    nlohmann::json data;
    ifs >> data;
    entityTemplates_[_typeName] = data;
    return true;
}

/// <summary>
/// 指定ディレクトリに特定のエンティティテンプレートを保存する.
/// </summary>
bool SceneJsonRegistry::SaveEntityTemplate(const std::string& _directory, const std::string& _typeName) {
    auto itr = entityTemplates_.find(_typeName);
    if (itr == entityTemplates_.end()) {
        LOG_ERROR("指定エンティティテンプレートが存在しません: {}", _typeName);
        return false;
    }
    myfs::CreateFolder(_directory);
    std::string path = _directory + "/" + _typeName + '.' + kEntityExtension;
    std::ofstream ofs(path);
    if (!ofs) {
        LOG_CRITICAL("エンティティテンプレートの保存に失敗: {}",
            _typeName);
        return false;
    }
    ofs << std::setw(4) << itr->second;
    return true;
}

/// <summary>
/// JSON データを直接エンティティテンプレートとしてレジストリに登録する.
/// </summary>
void SceneJsonRegistry::RegisterEntityTemplate(const std::string& _typeName, const nlohmann::json& _json) {
    entityTemplates_[_typeName] = _json;
}

/// <summary>
/// 稼働中のエンティティから最新の状態を抽出し、テンプレートとしてレジストリに登録する.
/// </summary>
void OriGine::SceneJsonRegistry::RegisterEntityTemplateFromEntity(const std::string& _typeName, Scene* _scene, Entity* _entity) {
    if (!_scene || !_entity) {
        LOG_ERROR("scene または entity が nullptr");
        return;
    }
    SceneFactory factory;

    // テンプレートとして登録
    entityTemplates_[_typeName] = factory.CreateEntityJsonFromEntity(_scene, _entity);
}

const nlohmann::json* SceneJsonRegistry::GetEntityTemplate(const std::string& _typeName) const {
    auto it = entityTemplates_.find(_typeName);
    return it != entityTemplates_.end() ? &it->second : nullptr;
}
