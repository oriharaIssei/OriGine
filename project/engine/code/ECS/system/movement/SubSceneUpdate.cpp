#include "SubSceneUpdate.h"

/// engine
#include "scene/Scene.h"
/// ECS
// component
#include "component/SubScene.h"

void SubSceneUpdate::UpdateEntity(Entity* _entity) {
    auto subScenes = getComponents<SubScene>(_entity);
    if (subScenes == nullptr) {
        return;
    }
    // サブシーンの更新
    for (auto& subScene : *subScenes) {
        // 非アクティブならスキップ
        if (subScene.isActive() == false) {
            continue;
        }

        auto scene = subScene.getSubSceneRef();
        if (scene) {
            // サブシーンが非アクティブになっていたら 非アクティブ化する
            if (!scene->isActive()) {
                subScene.Deactivate();
                continue;
            }
            scene->Update();
        }
    }
}
