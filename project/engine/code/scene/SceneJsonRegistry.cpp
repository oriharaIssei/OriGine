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

void OriGine::SceneJsonRegistry::CreateNewScene(const std::string& sceneName) {
    nlohmann::json empty;
    empty["Systems"]          = nlohmann::json::array();
    empty["CategoryActivity"] = nlohmann::json::array();
    empty["Entities"]         = nlohmann::json::array();

    scenes_[sceneName] = empty;
}

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

void SceneJsonRegistry::RegisterEntityTemplate(const std::string& typeName, const nlohmann::json& json) {
    entityTemplates_[typeName] = json;
}

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
