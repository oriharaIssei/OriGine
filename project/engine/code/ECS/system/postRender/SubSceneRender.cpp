#include "SubSceneRender.h"

/// ECS
#include "directX12/RenderTexture.h"

// component
#include "component/SubScene.h"

void SubSceneRender::Update() {
    if (entityIDs_.empty()) {
        return;
    }
    ISystem::eraseDeadEntity();
    scenes_.clear();

    for (auto id : entityIDs_) {
        auto entity = getEntity(id);
        if (entity) {
            UpdateEntity(entity);
        }
    }
    EndRender();
}

void SubSceneRender::EndRender() {
    if (scenes_.empty()) {
        return;
    }

    auto currentSceneView = getScene()->getSceneView();

    currentSceneView->PreDraw();
    currentSceneView->DrawTexture();
    for (auto& scene : scenes_) {
        scene->getSceneView()->DrawTexture();
    }
    currentSceneView->PostDraw();
}

void SubSceneRender::UpdateEntity(GameEntity* _entity) {
    auto subScenes = getComponents<SubScene>(_entity);
    for (auto& subScene : *subScenes) {
        if (!subScene.isActive()) {
            continue;
        }
        auto scene = subScene.getSubSceneRef();
        if (!scene) {
            continue;
        }

        scene->Render();

        scenes_.push_back(scene.get());
    }
}
