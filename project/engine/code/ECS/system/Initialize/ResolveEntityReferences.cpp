#include "ResolveEntityReferences.h"

/// engine
#include "scene/SceneManager.h"

/// component
#include "component/EntityReferenceList.h"

ResolveEntityReferences::ResolveEntityReferences() : ISystem(SystemCategory::Initialize) {}

void ResolveEntityReferences::Initialize() {}

void ResolveEntityReferences::Finalize() {}

void ResolveEntityReferences::UpdateEntity(Entity* _entity) {
    // EntityReferenceListコンポーネントを取得
    auto* entityRefListArray = GetComponents<EntityReferenceList>(_entity);
    if (entityRefListArray == nullptr) {
        return;
    }

    SceneSerializer serializer(GetScene());

    // EntityReferenceListに登録されているEntityファイルを読み込む
    for (auto& entityRefList : *entityRefListArray) {
        for (const auto& [directory, filename] : entityRefList.GetEntityFileList()) {
#ifdef _DEBUG
            Entity* loadedEntity = serializer.LoadEntity(directory, filename);
            loadedEntity->SetShouldSave(false); // 読み込んだEntityは保存しない
#else
            serializer.LoadEntity(directory, filename);
#endif // _DEBUG
        }
    }
}
