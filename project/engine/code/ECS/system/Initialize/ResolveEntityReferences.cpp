#include "ResolveEntityReferences.h"

/// engine
#include "scene/SceneManager.h"

/// component
#include "component/EntityReferenceList.h"

ResolveEntityReferences::ResolveEntityReferences() : ISystem(SystemCategory::Initialize) {}

void ResolveEntityReferences::Initialize() {}

void ResolveEntityReferences::Finalize() {}

void ResolveEntityReferences::UpdateEntity(GameEntity* _entity) {
    auto* entityRefListArray = getComponents<EntityReferenceList>(_entity);
    if (entityRefListArray == nullptr) {
        return;
    }

    SceneSerializer serializer(getScene());

    for (auto& entityRefList : *entityRefListArray) {
        for (const auto& [directory, filename] : entityRefList.getEntityFileList()) {
            serializer.LoadEntity(directory, filename);
        }
    }
}
