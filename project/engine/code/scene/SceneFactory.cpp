#include "SceneFactory.h"

/// engine
#include "scene/SceneJsonRegistry.h"

/// ECS
// system
#include "system/SystemRunner.h"

using namespace OriGine;

/// <summary>
/// 指定されたシーン名に基づいて、レジストリから取得した JSON データを展開しシーンを構築する.
/// </summary>
bool SceneFactory::BuildSceneByName(Scene* scene, const std::string& sceneName) {
    auto registry              = SceneJsonRegistry::GetInstance();
    const nlohmann::json* json = registry->GetSceneJson(sceneName);

    if (!json) {
        LOG_ERROR("BuildSceneByName: JSONが登録されていません: {}", sceneName);
        return false;
    }

    BuildSceneFromJson(scene, *json);
    return true;
}

/// <summary>
/// 登録済みのエンティティテンプレート名を使用して、シーン内に新しいエンティティを生成する.
/// </summary>
Entity* SceneFactory::BuildEntityFromTemplate(Scene* scene, const std::string& templateTypeName) {
    auto registry              = SceneJsonRegistry::GetInstance();
    const nlohmann::json* json = registry->GetEntityTemplate(templateTypeName);
    if (!json) {
        LOG_ERROR("BuildEntityFromTemplate: エンティティテンプレートが登録されていません: {}", templateTypeName);
        return nullptr;
    }
    return BuildEntity(scene, *json);
}

/// <summary>
/// 現在のシーンの状態 (登録システム、配置エンティティ) を解析し、保存用の JSON データを生成する.
/// </summary>
nlohmann::json SceneFactory::CreateSceneJsonFromScene(const Scene* scene) {
    nlohmann::json sceneJson = nlohmann::json::object();

    if (!scene) {
        return sceneJson;
    }

    // システム情報の保存処理
    {
        sceneJson["Systems"]          = nlohmann::json::array();
        sceneJson["CategoryActivity"] = nlohmann::json::array();
        auto& systems                 = scene->systemRunner_->GetSystemsRef();
        for (auto& [name, sys] : systems) {
            if (!sys) {
                continue;
            }
            nlohmann::json sysJson;
            sysJson["Priority"] = sys->GetPriority();
            sceneJson["Systems"].push_back({{name, sysJson}});
        }
        // 各カテゴリの有効/無効状態を記録
        for (int32_t i = 0; i < static_cast<int32_t>(SystemCategory::Count); ++i) {
            bool isActive = scene->systemRunner_->GetCategoryActivity((SystemCategory)i);
            sceneJson["CategoryActivity"].push_back(isActive);
        }
    }
    // エンティティ情報の保存処理 (ShouldSave フラグが true のもののみ対象)
    {
        auto& entities = scene->entityRepository_;
        ::std::list<Entity*> aliveEntities;

        for (auto& entity : entities->GetEntitiesRef()) {
            if (entity.IsAlive() && entity.ShouldSave()) {
                aliveEntities.push_back(&entity);
            }
        }

        for (auto& entity : aliveEntities) {
            if (!entity->IsAlive()) {
                continue;
            }
            nlohmann::json entityData = nlohmann::json::object();
            entityData                = CreateEntityJsonFromEntity(scene, entity);
            sceneJson["Entities"].push_back(entityData);
        }
    }
    return sceneJson;
}

/// <summary>
/// JSON データから、シーン全体に適用されるシステム構成をロードする.
/// </summary>
void SceneFactory::LoadSystems(Scene* scene, const nlohmann::json& systemsJson, const nlohmann::json& catActivityJson) {
    // システムの登録。名前と優先度に基づいて SystemRunner に追加
    for (auto& systemByType : systemsJson) {
        for (auto& [systemName, sysData] : systemByType.items()) {
            int priority = sysData["Priority"];
            scene->systemRunner_->RegisterSystem(systemName, priority, true);
        }
    }
    // 各更新カテゴリ (Input, Render 等) の実行フラグを設定
    for (int32_t i = 0; i < static_cast<int32_t>(SystemCategory::Count); ++i) {
        if (i < catActivityJson.size()) {
            scene->systemRunner_->SetCategoryActivity(
                (SystemCategory)i, catActivityJson[i].get<bool>());
        }
    }
}

/// <summary>
/// JSON 配列から複数のエンティティを一括でロード・構築する.
/// </summary>
void SceneFactory::LoadEntities(Scene* scene, const nlohmann::json& entitiesJson) {
    // エンティティの順次構築
    for (auto& entityJson : entitiesJson) {
        BuildEntity(scene, entityJson);
    }
}

/// <summary>
/// 単一のエンティティを JSON 定義に基づき生成し、コンポーネントとシステムを紐付ける.
/// </summary>
/// <param name="scene">配置先のシーン</param>
/// <param name="entityJson">エンティティの定義データ</param>
/// <returns>生成されたエンティティポインタ</returns>
Entity* SceneFactory::BuildEntity(Scene* scene, const nlohmann::json& entityJson) {
    // エンティティの作成処理
    std::string name = entityJson["Name"];
    bool isUnique    = entityJson["isUnique"];
    EntityHandle handle;
    if (entityJson.contains("Handle")) {
        handle = entityJson["Handle"];
    }

    // 指定されたハンドルまたは新規ハンドルでリポジトリにエンティティを登録
    handle         = scene->entityRepository_->CreateEntity(handle, name, isUnique);
    Entity* entity = scene->entityRepository_->GetEntity(handle);

    // 所属システムの紐付け
    LoadEntitySystems(scene, handle, entityJson["Systems"]);
    // 各種コンポーネントデータのロード
    LoadEntityComponents(scene, handle, entityJson["Components"]);
    return entity;
}

/// <summary>
/// 指定したエンティティの現在の全コンポーネントおよび所属システム情報を JSON 形式でシリアライズする.
/// </summary>
nlohmann::json SceneFactory::CreateEntityJsonFromEntity(const Scene* scene, Entity* entity) {
    nlohmann::json entityData = nlohmann::json::object();

    if (!entity || !entity->IsAlive()) {
        return entityData;
    }
    EntityHandle handle = entity->GetHandle();

    entityData["Name"]     = entity->GetDataType();
    entityData["Handle"]   = handle;
    entityData["isUnique"] = entity->IsUnique();

    // エンティティが所属している全システムを探索・記録
    entityData["Systems"]    = nlohmann::json::array();
    auto& systemsJson        = entityData["Systems"];
    const auto& sceneSystems = scene->systemRunner_->GetSystems();
    for (const auto& [systemName, system] : sceneSystems) {
        if (!system) {
            LOG_WARN("System not found: {}", systemName);
            continue;
        }
        if (system->HasEntity(handle)) {
            systemsJson.push_back({{"SystemCategory", system->GetCategory()}, {"SystemName", systemName}});
        }
    }

    // エンティティが保持している全コンポーネントデータを保存
    const auto& componentArrayMap = scene->componentRepository_->GetComponentArrayMap();
    nlohmann::json componentsData;
    for (const auto& [componentTypeName, componentArray] : componentArrayMap) {
        if (componentArray->HasEntity(handle)) {
            componentArray->SaveComponents(handle, componentsData);
        }
    }
    entityData["Components"] = componentsData;

    return entityData;
}

/// <summary>
/// 指定したエンティティを JSON 配列に定義されたシステム群に登録する.
/// </summary>
void SceneFactory::LoadEntitySystems(
    Scene* scene,
    EntityHandle entity,
    const nlohmann::json& systemsJson) {
    auto& systems = scene->systemRunner_->GetSystemsRef();
    for (auto& sys : systemsJson) {
        std::string systemName = sys["SystemName"];
        auto* system           = systems[systemName].get();
        if (system) {
            system->AddEntity(entity);
        }
    }
}

/// <summary>
/// JSON データに基づき、エンティティにコンポーネントを追加し、各フィールドの値をロードする.
/// </summary>
void SceneFactory::LoadEntityComponents(
    Scene* scene,
    EntityHandle _entity,
    const nlohmann::json& componentsJson) {
    for (auto& [componentTypename, componentData] : componentsJson.items()) {
        auto compArray = scene->componentRepository_->GetComponentArray(componentTypename);
        if (!compArray) {
            LOG_WARN("Don't Registered Component. Typename {}", componentTypename);
            continue;
        }
        compArray->LoadComponents(_entity, componentData);

        auto loadedComps = compArray->GetIComponents(_entity);
        for (auto& comp : loadedComps) {
            comp->Initialize(scene, _entity);
        }
    }
}
