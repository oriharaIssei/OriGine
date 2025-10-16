#include "SubSceneUpdate.h"

/// engine
#include "scene/Scene.h"
/// ECS
// component
#include "component/SubScene.h"

void SubSceneUpdate::UpdateEntity(GameEntity* _entity) {
    auto subScenes = getComponents<SubScene>(_entity);
    if (subScenes == nullptr) {
        return;
    }
    for (auto& subScene : *subScenes) {
        if (subScene.isActive() == false) {
            continue;
        }
        auto scene = subScene.getSubSceneRef();
        if (scene) {
            if (!scene->isActive()) {
                subScene.Deactivate();
                continue;
            }
            scene->Update();
        }
    }
}
