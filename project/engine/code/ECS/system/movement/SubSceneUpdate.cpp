#include "SubSceneUpdate.h"

/// engine
#include "scene/Scene.h"
/// ECS
// component
#include "component/SubScene.h"

void SubSceneUpdate::UpdateEntity(Entity* _entity) {
    auto subScenes = GetComponents<SubScene>(_entity);
    if (subScenes == nullptr) {
        return;
    }
    // サブシーンの更新
    for (auto& subScene : *subScenes) {
        // 非アクティブならスキップ
        if (subScene.IsActive() == false) {
            continue;
        }

        auto scene = subScene.GetSubSceneRef();
        if (scene) {
            // サブシーンが非アクティブになっていたら 非アクティブ化する
            if (!scene->IsActive()) {
                subScene.Deactivate();
                continue;
            }

            // シーンマネージャー を継承
            scene->SetSceneManager(
                GetScene()->GetSceneManager()
            );

            // 入力デバイスを継承
            scene->SetInputDevices(
                GetScene()->GetKeyboardInput(),
                GetScene()->GetMouseInput(),
                GetScene()->GetGamepadInput());

            scene->Update();
        }
    }
}
