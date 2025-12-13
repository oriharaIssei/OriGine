#pragma once

/// stl
#include <string>
#include <unordered_map>

/// engine
#include "entity/Entity.h"
#include "scene/Scene.h"

/// externals
#include <nlohmann/json.hpp>

namespace OriGine {

constexpr char kSceneJsonFolder[]      = "scene";
constexpr char kEntityTemplateFolder[] = "entities";
constexpr char kEntityExtension[]      = "ent";

/// <summary>
/// 使用されるシーンのJSONデータを保持するクラス
/// </summary>
class SceneJsonRegistry {
public:
    static SceneJsonRegistry* GetInstance();

    /// <summary>
    /// シーンJSON登録
    /// </summary>
    /// <param name="sceneName"></param>
    /// <param name="data"></param>
    void RegisterSceneJson(const std::string& sceneName, const nlohmann::json& data) {
        scenes_[sceneName] = data;
    }

    /// <summary>
    /// シーンJSON取得
    /// </summary>
    /// <param name="sceneName"></param>
    /// <returns></returns>
    const nlohmann::json* GetSceneJson(const std::string& sceneName) const {
        auto it = scenes_.find(sceneName);
        return (it != scenes_.end()) ? &it->second : nullptr;
    }

    /// <summary>
    /// 各シーンJSONデータをfileに保存する
    /// </summary>
    /// <param name="directory"></param>
    /// <returns></returns>
    bool SaveAllScene(const std::string& directory);

    /// <summary>
    /// 単一シーンJSONデータをfileに保存する
    /// </summary>
    /// <param name="sceneName"></param>
    /// <param name="directory"></param>
    /// <returns></returns>
    bool SaveScene(const Scene* scene, const std::string& directory);

    /// <summary>
    /// 指定ディレクトリからすべてのシーンJSONデータを読み込む
    /// </summary>
    /// <param name="directory"></param>
    /// <returns></returns>
    bool LoadAllScene(const std::string& directory);

    /// <summary>
    /// 単一シーンを読み込む
    /// </summary>
    bool LoadScene(const std::string& sceneName, const std::string& directory);

    /// <summary>
    /// 新しい空のシーンJSONを生成して登録
    /// </summary>
    void CreateNewScene(const std::string& sceneName);

    /// <summary>
    /// 指定ディレクトリからすべてのエンティティテンプレートを読み込む
    /// </summary>
    /// <param name="directory"></param>
    bool LoadAllEntityTemplates(const std::string& _directory);

    /// <summary>
    /// 単一エンティティテンプレートを読み込む
    /// </summary>
    /// <param name="_directory"></param>
    /// <param name="typeName"></param>
    /// <returns></returns>
    bool LoadEntityTemplate(const std::string& _directory, const std::string& typeName);

    /// <summary>
    /// 単一エンティティテンプレートを保存する
    /// </summary>
    /// <param name="_directory"></param>
    /// <param name="typeName"></param>
    /// <returns></returns>
    bool SaveEntityTemplate(const std::string& _directory, const std::string& typeName);

    /// <summary>
    /// エンティティテンプレート登録
    /// </summary>
    /// <param name="typeName"></param>
    /// <param name="json"></param>
    void RegisterEntityTemplate(const std::string& typeName, const nlohmann::json& json);

    /// <summary>
    /// 実際の Entity から JSON を生成してテンプレート登録
    /// </summary>
    void RegisterEntityTemplateFromEntity(const std::string& type, Scene* scene, Entity* entity);

    /// <summary>
    /// エンティティテンプレート取得
    /// </summary>
    /// <param name="typeName"></param>
    /// <returns></returns>
    const nlohmann::json* GetEntityTemplate(const std::string& typeName) const;

private:
    std::unordered_map<std::string, nlohmann::json> scenes_;
    std::unordered_map<std::string, nlohmann::json> entityTemplates_;
};

} // namespace OriGine
