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
bool OriGine::SceneJsonRegistry::SaveScene(const Scene* scene, const std::string& directory) {

    if (!scene) {
        LOG_ERROR("SaveScene: scene が nullptr");
        return false;
    }
    SceneFactory factory;
    nlohmann::json sceneJson = factory.CreateSceneJsonFromScene(const_cast<Scene*>(scene));

    std::string path = directory + "/" + scene->GetName() + ".json";
    std::ofstream ofs(path);
    if (!ofs) {
        LOG_CRITICAL("シーンJSONの保存に失敗: {}", scene->GetName());
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
bool OriGine::SceneJsonRegistry::LoadScene(const std::string& sceneName, const std::string& directory) {
    std::string path = directory + "/" + sceneName + ".json";
    std::ifstream ifs(path);
    if (!ifs) {
        LOG_ERROR("LoadScene: 読込失敗: {}", path);
        return false;
    }

    nlohmann::json data;
    ifs >> data;
    scenes_[sceneName] = data;
    return true;
}

/// <summary>
/// メモリ上に空のシーンデータ定義を作成する.
/// システムやエンティティの配列が空の状態で初期化される.
/// </summary>
void OriGine::SceneJsonRegistry::CreateNewScene(const std::string& sceneName) {
    nlohmann::json empty;
    empty["Systems"]          = nlohmann::json::array();
    empty["CategoryActivity"] = nlohmann::json::array();
    empty["Entities"]         = nlohmann::json::array();

    scenes_[sceneName] = empty;
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
bool OriGine::SceneJsonRegistry::LoadEntityTemplate(const std::string& _directory, const std::string& typeName) {
    std::string path = _directory + "/" + typeName + '.' + kEntityExtension;
    std::ifstream ifs(path);
    if (!ifs) {
        LOG_ERROR(" 読込失敗: {}", path);
        return false;
    }
    nlohmann::json data;
    ifs >> data;
    entityTemplates_[typeName] = data;
    return true;
}

/// <summary>
/// 指定ディレクトリに特定のエンティティテンプレートを保存する.
/// </summary>
bool OriGine::SceneJsonRegistry::SaveEntityTemplate(const std::string& _directory, const std::string& typeName) {
    auto itr = entityTemplates_.find(typeName);
    if (itr == entityTemplates_.end()) {
        LOG_ERROR("指定エンティティテンプレートが存在しません: {}", typeName);
        return false;
    }
    myfs::CreateFolder(_directory);
    std::string path = _directory + "/" + typeName + '.' + kEntityExtension;
    std::ofstream ofs(path);
    if (!ofs) {
        LOG_CRITICAL("エンティティテンプレートの保存に失敗: {}",
            typeName);
        return false;
    }
    ofs << std::setw(4) << itr->second;
    return true;
}

/// <summary>
/// JSON データを直接エンティティテンプレートとしてレジストリに登録する.
/// </summary>
void SceneJsonRegistry::RegisterEntityTemplate(const std::string& typeName, const nlohmann::json& json) {
    entityTemplates_[typeName] = json;
}

/// <summary>
/// 稼働中のエンティティから最新の状態を抽出し、テンプレートとしてレジストリに登録する.
/// </summary>
void OriGine::SceneJsonRegistry::RegisterEntityTemplateFromEntity(const std::string& type, Scene* scene, Entity* entity) {
    if (!scene || !entity) {
        LOG_ERROR("scene または entity が nullptr");
        return;
    }
    SceneFactory factory;

    // テンプレートとして登録
    entityTemplates_[type] = factory.CreateEntityJsonFromEntity(scene, entity);
}

const nlohmann::json* SceneJsonRegistry::GetEntityTemplate(const std::string& typeName) const {
    auto it = entityTemplates_.find(typeName);
    return it != entityTemplates_.end() ? &it->second : nullptr;
}
