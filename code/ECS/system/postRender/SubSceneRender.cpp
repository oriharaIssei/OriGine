#include "SubSceneRender.h"

/// engine
// directX12
#include "directX12/RenderTexture.h"

// component
#include "component/scene/SubScene.h"

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
    // renderTarget_(メインシーンのビュー)を書き込み対象として PreDraw するだけで、
    // 専用のPSOやルートシグネチャは使わない(各シーンの DrawTexture が自身のPSOで描画するため)
    renderTarget_->PreDraw();
}

/// <summary>
/// レンダリング処理
/// </summary>
void SubSceneRender::Rendering() {
    // メインシーン自身も優先度0として合成対象に加える
    // 描画するシーンを優先度順にソート
    scenes_.push_back(std::make_pair<int32_t, Scene*>(0, GetScene()));
    // 優先度の昇順にソート
    std::sort(scenes_.begin(), scenes_.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });

    RenderStart();

    // 各シーンのレンダーテクスチャを優先度の低い順に renderTarget_ へ上書き合成していく。
    // 後に描画したシーンほど手前に重なる
    for (auto& [priority, scene] : scenes_) {
        scene->GetSceneView()->DrawTexture();
    }

    RenderEnd();
}

/// <summary>
/// レンダリング終了処理
/// </summary>
void SubSceneRender::RenderEnd() {
    // renderTarget_ を読み取り可能な状態へ遷移
    renderTarget_->PostDraw();

    scenes_.clear();
}

/// <summary>
/// コンポーネントの割り当て
/// </summary>
/// <param name="_handle">エンティティ</param>
void SubSceneRender::DispatchComponent(const EntityHandle& _handle) {
    auto& subScenes = GetComponents<SubScene>(_handle);
    for (auto& subScene : subScenes) {
        if (!subScene.IsActive()) {
            continue;
        }
        auto scene = subScene.GetSubSceneRef();
        if (!scene) {
            continue;
        }
        // サブシーンを自身のレンダーテクスチャへ描画してから、合成対象リストに登録する
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
