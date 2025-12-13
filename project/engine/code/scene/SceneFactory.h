#pragma once

/// engine
#include "scene/Scene.h"

/// externals
#include <nlohmann/json.hpp>

namespace OriGine {

/// <summary>
/// nlohmann::jsonからシーン内容を構築するクラス
/// </summary>
class SceneFactory {
public:
    /// <summary>
    /// シーン名を指定して構築
    /// </summary>
    bool BuildSceneByName(Scene* scene, const std::string& sceneName);

    /// <summary>
    /// JSON からシーン内容を構築
    /// </summary>
    void BuildSceneFromJson(Scene* scene, const nlohmann::json& data) {
        if (!scene) {
            return;
        }

        LoadSystems(scene, data["Systems"], data["CategoryActivity"]);
        LoadEntities(scene, data["Entities"]);
    }

    /// <summary>
    /// シーンからJSONデータを作成する
    /// </summary>
    /// <param name="scene"></param>
    /// <returns></returns>
    nlohmann::json CreateSceneJsonFromScene(const Scene* scene);

    /// <summary>
    /// SceneJsonRegistryが持つテンプレートからエンティティを構築
    /// </summary>
    /// <param name="scene"></param>
    /// <param name="entity"></param>
    /// <param name="templateTypeName"></param>
    Entity* BuildEntityFromTemplate(Scene* scene, const std::string& templateTypeName);

    /// <summary>
    /// エンティティを構築する
    /// </summary>
    /// <param name="scene"></param>
    /// <param name="entityJson"></param>
    /// <returns></returns>
    Entity* BuildEntity(Scene* scene, const nlohmann::json& entityJson);

    /// <summary>
    /// エンティティからJSONデータを作成する
    /// </summary>
    /// <param name="scene"></param>
    /// <param name="entity"></param>
    /// <returns></returns>
    nlohmann::json CreateEntityJsonFromEntity(const Scene* scene, Entity* entity);

private:
    /// <summary>
    /// System を読み込む
    /// </summary>
    /// <param name="scene"></param>
    /// <param name="systemsJson"></param>
    /// <param name="catActivityJson"></param>
    void LoadSystems(Scene* scene,
        const nlohmann::json& systemsJson,
        const nlohmann::json& catActivityJson);

    /// <summary>
    /// Entity を読み込む
    /// </summary>
    /// <param name="scene"></param>
    /// <param name="entitiesJson"></param>
    void LoadEntities(Scene* scene, const nlohmann::json& entitiesJson);

    /// <summary>
    /// エンティティのシステムを読み込む
    /// </summary>
    /// <param name="scene"></param>
    /// <param name="entity"></param>
    /// <param name="systemsJson"></param>
    void LoadEntitySystems(Scene* scene, Entity* entity, const nlohmann::json& systemsJson);
    /// <summary>
    /// エンティティのコンポーネントを読み込む
    /// </summary>
    /// <param name="scene"></param>
    /// <param name="entity"></param>
    /// <param name="componentsJson"></param>
    void LoadEntityComponents(Scene* scene, Entity* entity,
        const nlohmann::json& componentsJson);
};

} // namespace OriGine
