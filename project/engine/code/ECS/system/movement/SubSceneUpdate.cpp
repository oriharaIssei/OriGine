#include "SubSceneUpdate.h"

/// engine
#include "camera/CameraManager.h"
#include "scene/Scene.h"
/// ECS
// component
#include "component/SubScene.h"

using namespace OriGine;

/// <summary>
/// 各エンティティが持つサブシーンを更新する
/// </summary>
/// <param name="_handle">対象のエンティティハンドル</param>
void SubSceneUpdate::UpdateEntity(EntityHandle _handle) {
    auto& subScenes = GetComponents<SubScene>(_handle);
    if (subScenes.empty()) {
        return;
    }
    // サブシーンの更新

    for (auto& subScene : subScenes) {
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
                GetScene()->GetSceneManager());

            // 入力デバイスを継承
            scene->SetInputDevices(
                GetScene()->GetKeyboardInput(),
                GetScene()->GetMouseInput(),
                GetScene()->GetGamepadInput());

            scene->Update();
        }
    }
}
