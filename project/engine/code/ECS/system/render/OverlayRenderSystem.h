#pragma once
#include "TexturedMeshRenderSystem.h"

/// <summary>
/// オーバーレイ描画を行うシステム
/// 3d Objectを深度テスト無しで描画する
/// Model やPrimitiveのRendererを持つEntityを対象とする
/// </summary>
class OverlayRenderSystem
    : public TexturedMeshRenderSystem {
public:
    OverlayRenderSystem();
    ~OverlayRenderSystem() override;

    /// <summary>
    /// PSOの作成
    /// </summary>
    void CreatePSO() override;
};
