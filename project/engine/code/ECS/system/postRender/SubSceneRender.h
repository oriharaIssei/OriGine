#pragma once

#include "system/postRender/base/BasePostRenderingSystem.h"

/// stl
#include <vector>
/// engine
#include "scene/Scene.h"

/// <summary>
/// サブシーンのレンダリングを行うシステム
/// </summary>
class SubSceneRender
    : public BasePostRenderingSystem {
public:
    SubSceneRender();
    ~SubSceneRender() override;

    void Initialize() override {}
    void Finalize() override {}

protected:
    /// <summary>
    /// PSO作成 (PSOを作る必要がないので何もしない)
    /// </summary>
    void CreatePSO() override;

    /// <summary>
    /// レンダリング開始処理
    /// </summary>
    void RenderStart() override;
    /// <summary>
    /// レンダリング処理
    /// </summary>
    void Rendering() override;
    /// <summary>
    /// レンダリング終了処理
    /// </summary>
    void RenderEnd() override;

    /// <summary>
    /// PostEffectに使用するComponentを登録する
    /// (Systemによっては使用しない)
    /// </summary>
    void DispatchComponent(Entity* _entity) override;

    /// <summary>
    /// ポストレンダリングをスキップするかどうか
    /// </summary>
    /// <returns>true = skipする / false = skipしない</returns>
    bool ShouldSkipPostRender() const override;

private:
    std::vector<std::pair<int32_t, Scene*>> scenes_;
};
