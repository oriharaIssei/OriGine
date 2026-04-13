#pragma once

/// Microsoft
#include <wrl.h>

#include <d3d12.h>

/// stl
#include <list>
#include <string>

namespace OriGine {
/// <summary>
/// パイプラインステートオブジェクト (PSO) とルートシグネチャをペアで保持する構造体.
/// 描画パス（不透明、半透明、影など）ごとのレンダリング設定を一括で管理する.
/// </summary>
struct PipelineStateObj {
    /// <summary>
    /// 各オブジェクトを解放し、リセットする.
    /// </summary>
    void Finalize();

    /// <summary>ルートシグネチャ（シェーダーへの定数/ディスクリプタの受け渡しルール）</summary>
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    /// <summary>パイプラインステート（ブレンド、ラスタライザ、深度設定、各種シェーダーバイトコードの集合）</summary>
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState = nullptr;
};

} // namespace OriGine
