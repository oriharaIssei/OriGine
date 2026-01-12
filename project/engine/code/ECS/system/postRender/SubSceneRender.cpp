#include "SubSceneRender.h"

/// engine
// directX12
#include "directX12/RenderTexture.h"

// component
#include "component/SubScene.h"

using namespace OriGine;

/// <summary>
/// コンストラクタ
/// </summary>
SubSceneRender::SubSceneRender() : BasePostRenderingSystem() {}

/// <summary>
/// デストラクタ
/// </summary>
SubSceneRender::~SubSceneRender() = default;

/// <summary>
/// PSO作成 (PSOを作る必要がないので何もしない)
/// </summary>
void SubSceneRender::CreatePSO() {}

/// <summary>
/// レンダリング開始処理
/// </summary>
void SubSceneRender::RenderStart() {
    renderTarget_->PreDraw();
}

/// <summary>
/// レンダリング処理
/// </summary>
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

/// <summary>
/// レンダリング終了処理
/// </summary>
void SubSceneRender::RenderEnd() {
    renderTarget_->PostDraw();

    scenes_.clear();
}

/// <summary>
/// コンポーネントの割り当て
/// </summary>
/// <param name="_handle">エンティティ</param>
void SubSceneRender::DispatchComponent(EntityHandle _handle) {
    auto& subScenes = GetComponents<SubScene>(_handle);
    for (auto& subScene : subScenes) {
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

/// <summary>
/// ポストレンダリングをスキップするかどうか
/// </summary>
/// <returns>描画データがない場合は true</returns>
bool SubSceneRender::ShouldSkipPostRender() const {
    return scenes_.empty();
}
