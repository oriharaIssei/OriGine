#pragma once

#include "system/render/base/BaseRenderSystem.h"

/// stl
#include <memory>
#include <unordered_map>

/// engine
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"
// component
class SpriteRenderer;
/// math
#include "Matrix4x4.h"

/// <summary>
/// 背景スプライトの描画を行うシステム
/// </summary>
class BackGroundSpriteRenderSystem
    : public BaseRenderSystem {
public:
    BackGroundSpriteRenderSystem();
    ~BackGroundSpriteRenderSystem() override;
    void Initialize() override;
    // void Update() override;
    void Finalize() override;

protected:
    /// <summary>
    /// PSOの作成
    /// </summary>
    void CreatePSO() override;

    /// <summary>
    /// レンダリング開始処理
    /// </summary>
    virtual void StartRender() override;

    /// <summary>
    /// レンダリング処理(StartRenderから描画まですべてを行う)
    /// </summary>
    void Rendering() override;

    /// <summary>
    /// 描画する物を登録
    /// </summary>
    /// <param name="_entity"></param>
    void DispatchRenderer(Entity* _entity) override;

    /// <summary>
    /// レンダリングをスキップするかどうか(描画オブジェクトが無いときは描画をスキップする)
    /// </summary>
    /// <returns>true ＝ 描画をスキップする / false = 描画スキップしない</returns>
    bool IsSkipRendering() const override;

private:
    std::vector<SpriteRenderer*> renderers_;
    Matrix4x4 viewPortMat_;

    std::array<PipelineStateObj*, kBlendNum> psoByBlendMode_{};
};
