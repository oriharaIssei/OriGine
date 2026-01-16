#pragma once

/// engine
#include "scene/Scene.h"

/// externals
#include <nlohmann/json.hpp>

namespace OriGine {

/// <summary>
/// JSON 形式のデータ構造からシーン内のエンティティ、コンポーネント、システムを構築するためのファクトリクラス.
/// エンティティのテンプレート機能や、シーン全体のリロードなどのロジックを管理する.
/// </summary>
class SceneFactory {
public:
    /// <summary>
    /// シーン名を指定して、ファイルまたはレジストリからシーン内容を再構築する.
    /// </summary>
    /// <param name="_scene">構築対象のシーンオブジェクト</param>
    /// <param name="_sceneName">読み込むシーンの名前</param>
    /// <returns>成功した場合は true</returns>
    bool BuildSceneByName(Scene* _scene, const std::string& _sceneName);

    /// <summary>
    /// 直接 JSON データを渡し、それに基づいてシーン内容 (System, Entity) を構築する.
    /// </summary>
    /// <param name="_scene">構築対象のシーン</param>
    /// <param name="_data">ソースとなる JSON データ</param>
    void BuildSceneFromJson(Scene* _scene, const nlohmann::json& _data) {
        if (!_scene) {
            return;
        }

        LoadSystems(_scene, _data["Systems"], _data["CategoryActivity"]);
        LoadEntities(_scene, _data["Entities"]);
    }

    /// <summary>
    /// 指定されたシーンの現在の状態 (配置されたエンティティ等) を JSON 構造体に書き出す.
    /// </summary>
    /// <param name="_scene">対象のシーン</param>
    /// <returns>シリアライズされた JSON データ</returns>
    nlohmann::json CreateSceneJsonFromScene(const Scene* _scene);

    /// <summary>
    /// 事前に登録されたエンティティテンプレート (json) を使用して、指定したシーン内にエンティティを新規構築する.
    /// </summary>
    /// <param name="_scene">追加先のシーン</param>
    /// <param name="_templateTypeName">テンプレートの型名</param>
    /// <returns>生成されたエンティティへのポインタ</returns>
    Entity* BuildEntityFromTemplate(Scene* _scene, const std::string& _templateTypeName);

    /// <summary>
    /// JSON データから単一のエンティティとそのコンポーネント、システムを構築する.
    /// </summary>
    /// <param name="_scene">追加先のシーン</param>
    /// <param name="_entityJson">エンティティの定義 JSON</param>
    /// <returns>生成されたエンティティへのポインタ</returns>
    Entity* BuildEntity(Scene* _scene, const nlohmann::json& _entityJson);

    /// <summary>
    /// 特定のエンティティとその状態を JSON データに変換する.
    /// </summary>
    /// <param name="_scene">エンティティが属するシーン</param>
    /// <param name="_entity">対象のエンティティ</param>
    /// <returns>シリアライズされた JSON データ</returns>
    nlohmann::json CreateEntityJsonFromEntity(const Scene* _scene, Entity* _entity);

private:
    /// <summary>
    /// シーン全体に対してシステムとその設定をロードする.
    /// </summary>
    void LoadSystems(Scene* _scene,
        const nlohmann::json& _systemsJson,
        const nlohmann::json& _catActivityJson);

    /// <summary>
    /// シーン全体に対してエンティティのリストをロードする.
    /// </summary>
    void LoadEntities(Scene* _scene, const nlohmann::json& _entitiesJson);

    /// <summary>
    /// エンティティ個別のシステム設定をロードする.
    /// </summary>
    void LoadEntitySystems(Scene* _scene, EntityHandle _entity, const nlohmann::json& _systemsJson);

    /// <summary>
    /// エンティティ個別のコンポーネントとそのフィールド設定をロードする.
    /// </summary>
    void LoadEntityComponents(
        Scene* _scene,
        EntityHandle _entity,
        const nlohmann::json& _componentsJson);
};

} // namespace OriGine
