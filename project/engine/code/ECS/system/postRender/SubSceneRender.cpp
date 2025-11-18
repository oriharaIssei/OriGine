#include "SubSceneRender.h"

/// engine
// directX12
#include "directX12/RenderTexture.h"

// component
#include "component/SubScene.h"

SubSceneRender::SubSceneRender() : BasePostRenderingSystem() {}
SubSceneRender::~SubSceneRender() = default;

void SubSceneRender::CreatePSO() {}

void SubSceneRender::RenderStart() {
    renderTarget_->PreDraw();
    renderTarget_->DrawTexture();
}

void SubSceneRender::Rendering() {
    RenderStart();
    for (auto& scene : scenes_) {
        scene->GetSceneView()->DrawTexture();
    }
    RenderEnd();
}

void SubSceneRender::RenderEnd() {
    renderTarget_->PostDraw();

    scenes_.clear();
}

void SubSceneRender::DispatchComponent(Entity* _entity) {
    auto subScenes = GetComponents<SubScene>(_entity);
    for (auto& subScene : *subScenes) {
        if (!subScene.IsActive()) {
            continue;
        }
        auto scene = subScene.GetSubSceneRef();
        if (!scene) {
            continue;
        }
        scene->Render();
        scenes_.push_back(scene.get());
    }
}

bool SubSceneRender::ShouldSkipPostRender() const {
    return scenes_.empty();
}
