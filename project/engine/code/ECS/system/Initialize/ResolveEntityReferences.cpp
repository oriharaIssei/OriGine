#include "ResolveEntityReferences.h"

/// engine
#include "scene/SceneFactory.h"
#include "scene/SceneJsonRegistry.h"

/// component
#include "component/EntityReferenceList.h"

using namespace OriGine;

ResolveEntityReferences::ResolveEntityReferences() : ISystem(SystemCategory::Initialize) {}

/// <summary>
/// 初期化
/// </summary>
void ResolveEntityReferences::Initialize() {}

/// <summary>
/// 終了処理
/// </summary>
void ResolveEntityReferences::Finalize() {}

/// <summary>
/// エンティティの更新（Entity間の参照を解決する）
/// </summary>
/// <param name="_handle">エンティティハンドル</param>
void ResolveEntityReferences::UpdateEntity(EntityHandle _handle) {
    // EntityReferenceListコンポーネントを取得
    auto& entityRefListArray = GetComponents<EntityReferenceList>(_handle);
    if (entityRefListArray.empty()) {
        return;
    }

    SceneFactory sceneFactory{};

    // EntityReferenceListに登録されているEntityファイルを読み込む
    for (auto& entityRefList : entityRefListArray) {
        for (const auto& [directory, filename] : entityRefList.GetEntityFileList()) {
            Entity* loadedEntity = sceneFactory.BuildEntityFromTemplate(GetScene(), filename);
            if (!loadedEntity) { // 読み込まれていなかったら、読みに行く
                if (!SceneJsonRegistry::GetInstance()->LoadEntityTemplate(directory, filename)) {
                    LOG_ERROR("Failed to load entity template from file '{}/{}'.", directory, filename);
                    continue;
                }
                loadedEntity = sceneFactory.BuildEntityFromTemplate(GetScene(), filename);
            }
            loadedEntity->SetShouldSave(false); // 読み込んだEntityは保存しない
        }
    }
}
