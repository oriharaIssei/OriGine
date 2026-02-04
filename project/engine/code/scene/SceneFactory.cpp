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
bool SceneFactory::BuildSceneByName(Scene* _scene, const std::string& _sceneName) {
    auto registry              = SceneJsonRegistry::GetInstance();
    const nlohmann::json* json = registry->GetSceneJson(_sceneName);

    if (!json) {
        LOG_ERROR("BuildSceneByName: JSONが登録されていません: {}", _sceneName);
        return false;
    }

    BuildSceneFromJson(_scene, *json);
    return true;
}

/// <summary>
/// 登録済みのエンティティテンプレート名を使用して、シーン内に新しいエンティティを生成する.
/// Handleは常に新規生成される.
/// </summary>
Entity* SceneFactory::BuildEntityFromTemplate(Scene* _scene, const std::string& _templateTypeName, HandleAssignMode _assignMode) {
    auto registry              = SceneJsonRegistry::GetInstance();
    const nlohmann::json* json = registry->GetEntityTemplate(_templateTypeName);
    if (!json) {
        LOG_ERROR("BuildEntityFromTemplate: エンティティテンプレートが登録されていません: {}", _templateTypeName);
        return nullptr;
    }
    return BuildEntity(_scene, *json, _assignMode);
}

/// <summary>
/// 現在のシーンの状態 (登録システム、配置エンティティ) を解析し、保存用の JSON データを生成する.
/// </summary>
nlohmann::json SceneFactory::CreateSceneJsonFromScene(const Scene* _scene) {
    nlohmann::json sceneJson = nlohmann::json::object();

    if (!_scene) {
        return sceneJson;
    }

    // システム情報の保存処理
    {
        sceneJson["Systems"]          = nlohmann::json::array();
        sceneJson["CategoryActivity"] = nlohmann::json::array();
        auto& systems                 = _scene->systemRunner_->GetSystemsRef();
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
            bool isActive = _scene->systemRunner_->GetCategoryActivity((SystemCategory)i);
            sceneJson["CategoryActivity"].push_back(isActive);
        }
    }
    // エンティティ情報の保存処理 (ShouldSave フラグが true のもののみ対象)
    {
        auto& entities = _scene->entityRepository_;
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
            entityData                = CreateEntityJsonFromEntity(_scene, entity);
            sceneJson["Entities"].push_back(entityData);
        }
    }
    return sceneJson;
}

/// <summary>
/// JSON データから、シーン全体に適用されるシステム構成をロードする.
/// </summary>
void SceneFactory::LoadSystems(Scene* _scene, const nlohmann::json& _systemsJson, const nlohmann::json& _catActivityJson) {
    // システムの登録。名前と優先度に基づいて SystemRunner に追加
    for (auto& systemByType : _systemsJson) {
        for (auto& [systemName, sysData] : systemByType.items()) {
            int priority = sysData["Priority"];
            _scene->systemRunner_->RegisterSystem(systemName, priority, true);
        }
    }
    // 各更新カテゴリ (Input, Render 等) の実行フラグを設定
    for (int32_t i = 0; i < static_cast<int32_t>(SystemCategory::Count); ++i) {
        if (i < _catActivityJson.size()) {
            _scene->systemRunner_->SetCategoryActivity(
                (SystemCategory)i, _catActivityJson[i].get<bool>());
        }
    }
}

/// <summary>
/// JSON 配列から複数のエンティティを一括でロード・構築する.
/// </summary>
void SceneFactory::LoadEntities(
    Scene* _scene,
    const nlohmann::json& _entitiesJson,
    HandleAssignMode _handleMode) {
    // エンティティの順次構築
    for (auto& entityJson : _entitiesJson) {
        BuildEntity(_scene, entityJson, _handleMode);
    }
}

/// <summary>
/// 単一のエンティティを JSON 定義に基づき生成し、コンポーネントとシステムを紐付ける.
/// JSONに保存されているHandleを使用する (デフォルト動作).
/// </summary>
/// <param name="_scene">配置先のシーン</param>
/// <param name="_entityJson">エンティティの定義データ</param>
/// <returns>生成されたエンティティポインタ</returns>
Entity* SceneFactory::BuildEntity(Scene* _scene, const nlohmann::json& _entityJson) {
    return BuildEntity(_scene, _entityJson, HandleAssignMode::UseSaved);
}

/// <summary>
/// 単一のエンティティを JSON 定義に基づき生成し、コンポーネントとシステムを紐付ける.
/// </summary>
/// <param name="_scene">配置先のシーン</param>
/// <param name="_entityJson">エンティティの定義データ</param>
/// <param name="_handleMode">Handleの割り当て方法</param>
/// <returns>生成されたエンティティポインタ</returns>
Entity* SceneFactory::BuildEntity(
    Scene* _scene,
    const nlohmann::json& _entityJson,
    HandleAssignMode _handleMode) {
    // エンティティの作成処理
    std::string name = _entityJson["Name"];
    bool isUnique    = _entityJson["isUnique"];
    EntityHandle handle;

    if (_handleMode == HandleAssignMode::UseSaved && _entityJson.contains("Handle")) {
        handle = _entityJson["Handle"];
    }

    // 指定されたハンドルまたは新規ハンドルでリポジトリにエンティティを登録
    handle         = _scene->entityRepository_->CreateEntity(handle, name, isUnique);
    Entity* entity = _scene->entityRepository_->GetEntity(handle);

    // 所属システムの紐付け
    LoadEntitySystems(_scene, handle, _entityJson["Systems"]);
    // 各種コンポーネントデータのロード
    LoadEntityComponents(_scene, handle, _entityJson["Components"], _handleMode);
    return entity;
}

/// <summary>
/// 指定したエンティティの現在の全コンポーネントおよび所属システム情報を JSON 形式でシリアライズする.
/// </summary>
nlohmann::json SceneFactory::CreateEntityJsonFromEntity(const Scene* _scene, Entity* _entity) {
    nlohmann::json entityData = nlohmann::json::object();

    if (!_entity || !_entity->IsAlive()) {
        return entityData;
    }
    EntityHandle handle = _entity->GetHandle();

    entityData["Name"]     = _entity->GetDataType();
    entityData["Handle"]   = handle;
    entityData["isUnique"] = _entity->IsUnique();

    // エンティティが所属している全システムを探索・記録
    entityData["Systems"]    = nlohmann::json::array();
    auto& systemsJson        = entityData["Systems"];
    const auto& sceneSystems = _scene->systemRunner_->GetSystems();
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
    const auto& componentArrayMap = _scene->componentRepository_->GetComponentArrayMap();
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
    Scene* _scene,
    EntityHandle _entity,
    const nlohmann::json& _systemsJson) {
    auto& systems = _scene->systemRunner_->GetSystemsRef();
    for (auto& sys : _systemsJson) {
        std::string systemName = sys["SystemName"];
        auto* system           = systems[systemName].get();
        if (system) {
            system->AddEntity(_entity);
        }
    }
}

/// <summary>
/// JSON データに基づき、エンティティにコンポーネントを追加し、各フィールドの値をロードする.
/// </summary>
void SceneFactory::LoadEntityComponents(
    Scene* _scene,
    EntityHandle _entity,
    const nlohmann::json& _componentsJson,
    HandleAssignMode _handleMode) {
    for (auto& [componentTypename, componentData] : _componentsJson.items()) {
        auto compArray = _scene->componentRepository_->GetComponentArray(componentTypename);
        if (!compArray) {
            LOG_WARN("Don't Registered Component. Typename {}", componentTypename);
            continue;
        }
        compArray->LoadComponents(_entity, componentData, _handleMode);

        auto loadedComps = compArray->GetIComponents(_entity);
        for (auto& comp : loadedComps) {
            comp->Initialize(_scene, _entity);
        }
    }
}
