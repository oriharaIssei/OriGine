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
}

void SubSceneRender::Rendering() {
    // 描画するシーンを優先度順にソート
    scenes_.push_back(std::make_pair<int32_t, Scene*>(0, GetScene()));
    // 優先度の昇順にソート
    std::sort(scenes_.begin(), scenes_.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });

    RenderStart();

    for (auto& [priority, scene] : scenes_) {
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
        scenes_.push_back(std::make_pair<int32_t, Scene*>(subScene.GetRenderingPriority(), scene.get()));
    }
}

bool SubSceneRender::ShouldSkipPostRender() const {
    return scenes_.empty();
}
