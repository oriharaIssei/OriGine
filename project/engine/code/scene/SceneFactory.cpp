#include "SceneFactory.h"

/// engine
#include "scene/SceneJsonRegistry.h"

/// ECS
// system
#include "system/SystemRunner.h"

using namespace OriGine;

bool OriGine::SceneFactory::BuildSceneByName(Scene* scene, const std::string& sceneName) {
    auto registry              = SceneJsonRegistry::GetInstance();
    const nlohmann::json* json = registry->GetSceneJson(sceneName);

    if (!json) {
        LOG_ERROR("BuildSceneByName: JSONが登録されていません: {}", sceneName);
        return false;
    }

    BuildSceneFromJson(scene, *json);
    return true;
}

Entity* OriGine::SceneFactory::BuildEntityFromTemplate(Scene* scene, const std::string& templateTypeName) {
    auto registry              = SceneJsonRegistry::GetInstance();
    const nlohmann::json* json = registry->GetEntityTemplate(templateTypeName);
    if (!json) {
        LOG_ERROR("BuildEntityFromTemplate: エンティティテンプレートが登録されていません: {}", templateTypeName);
        return nullptr;
    }
    return BuildEntity(scene, *json);
}

nlohmann::json OriGine::SceneFactory::CreateSceneJsonFromScene(const Scene* scene) {
    nlohmann::json sceneJson = nlohmann::json::object();

    if (!scene) {
        return sceneJson;
    }

    // システム情報の保存
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
        for (int32_t i = 0; i < static_cast<int32_t>(SystemCategory::Count); ++i) {
            bool isActive = scene->systemRunner_->GetCategoryActivity((SystemCategory)i);
            sceneJson["CategoryActivity"].push_back(isActive);
        }
    }
    // エンティティ情報の保存
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

            entityData = CreateEntityJsonFromEntity(scene, entity);

            sceneJson["Entities"].push_back(entityData);
        }
    }
    return sceneJson;
}

void OriGine::SceneFactory::LoadSystems(Scene* scene, const nlohmann::json& systemsJson, const nlohmann::json& catActivityJson) {
    // システムの登録
    for (auto& systemByType : systemsJson) {
        for (auto& [systemName, sysData] : systemByType.items()) {
            int priority = sysData["Priority"];
            scene->systemRunner_->RegisterSystem(systemName, priority, true);
        }
    }
    // システムカテゴリのアクティビティ設定
    for (int32_t i = 0; i < static_cast<int32_t>(SystemCategory::Count); ++i) {
        if (i < catActivityJson.size()) {
            scene->systemRunner_->SetCategoryActivity(
                (SystemCategory)i, catActivityJson[i].get<bool>());
        }
    }
}

void OriGine::SceneFactory::LoadEntities(Scene* scene, const nlohmann::json& entitiesJson) {
    // エンティティの構築
    for (auto& entityJson : entitiesJson) {
        BuildEntity(scene, entityJson);
    }
}

Entity* OriGine::SceneFactory::BuildEntity(Scene* scene, const nlohmann::json& entityJson) {
    // エンティティの作成
    std::string name = entityJson["Name"];
    bool isUnique    = entityJson["isUnique"];
    EntityHandle handle;
    if (entityJson.contains("Handle")) {
        handle = entityJson["Handle"];
    }

    handle         = scene->entityRepository_->CreateEntity(handle, name, isUnique);
    Entity* entity = scene->entityRepository_->GetEntity(handle);
    // システムの読み込み
    LoadEntitySystems(scene, handle, entityJson["Systems"]);
    // コンポーネントの読み込み
    LoadEntityComponents(scene, handle, entityJson["Components"]);
    return entity;
}

nlohmann::json OriGine::SceneFactory::CreateEntityJsonFromEntity(const Scene* scene, Entity* entity) {
    nlohmann::json entityData = nlohmann::json::object();

    if (!entity || !entity->IsAlive()) {
        // 無効なエンティティの場合、空のJSONオブジェクトを返す
        return entityData;
    }
    EntityHandle handle = entity->GetHandle();

    entityData["Name"]     = entity->GetDataType();
    entityData["Handle"]   = handle;
    entityData["isUnique"] = entity->IsUnique();

    // 所属するシステムを保存
    entityData["Systems"]    = nlohmann::json::array();
    auto& systemsJson        = entityData["Systems"];
    const auto& sceneSystems = scene->systemRunner_->GetSystems();
    for (const auto& [systemName, system] : sceneSystems) {
        if (!system) {
            LOG_WARN("System not found: {}", systemName);
            continue; // 無効なシステムはスキップ
        }
        if (system->HasEntity(handle)) {
            systemsJson.push_back({{"SystemCategory", system->GetCategory()}, {"SystemName", systemName}});
        }
    }

    // コンポーネントを保存
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

void OriGine::SceneFactory::LoadEntitySystems(Scene* scene, EntityHandle entity, const nlohmann::json& systemsJson) {
    // システムの取得
    auto& systems = scene->systemRunner_->GetSystemsRef();
    for (auto& sys : systemsJson) {
        // システム名からシステムを取得してエンティティを追加
        std::string systemName = sys["SystemName"];
        auto* system           = systems[systemName].get();
        if (system) {
            system->AddEntity(entity);
        }
    }
}

void OriGine::SceneFactory::LoadEntityComponents(
    Scene* scene,
    EntityHandle _entity,
    const nlohmann::json& componentsJson) {
    // コンポーネントの読み込み
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
