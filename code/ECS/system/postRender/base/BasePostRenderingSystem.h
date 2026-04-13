#pragma once

#include "system/ISystem.h"

/// stl
#include <memory>

/// engine
// drecitX12
#include "directX12/DxCommand.h"
#include "directX12/RenderTexture.h"

namespace OriGine {

/// <summary>
/// ポストレンダリングを行うシステムの基底クラス
/// </summary>
class BasePostRenderingSystem
    : public ISystem {
public:
    BasePostRenderingSystem(int32_t _priority = 0);
    virtual ~BasePostRenderingSystem();

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

protected:
    /// <summary>
    /// PSO(パイプラインステートオブジェクト)の作成
    /// </summary>
    virtual void CreatePSO() = 0;

    /// <summary>
    /// レンダリング開始処理（シェーダーパラメータセット等）
    /// </summary>
    virtual void RenderStart() = 0;

    /// <summary>
    /// レンダリング処理（描画命令の発行）
    /// </summary>
    virtual void Rendering() = 0;

    /// <summary>
    /// レンダリング終了処理（リソースのバリア変更等）
    /// </summary>
    virtual void RenderEnd() = 0;

    /// <summary>
    /// ポストエフェクトに使用するコンポーネントを有効な場合にリスト化する等の前処理
    /// </summary>
    /// <param name="_owner">エンティティハンドル</param>
    virtual void DispatchComponent(EntityHandle /*_owner*/) {}

    /// <summary>
    /// ポストレンダリングをスキップするかどうか
    /// </summary>
    /// <returns>true = スキップする / false = スキップしない</returns>
    virtual bool ShouldSkipPostRender() const {
        return false;
    }

protected:
    std::unique_ptr<DxCommand> dxCommand_ = nullptr; // DirectXコマンド管理
    RenderTexture* renderTarget_          = nullptr; // 描画先のレンダーテクスチャ

public:
    /// <summary>
    /// 描画先のレンダーテクスチャを取得する
    /// </summary>
    /// <returns>レンダーテクスチャへのポインタ</returns>
    RenderTexture* GetRenderTarget() const {
        return renderTarget_;
    }
    /// <summary>
    /// 描画先のレンダーテクスチャを設定する
    /// </summary>
    /// <param name="_renderTarget">レンダーテクスチャへのポインタ</param>
    void SetRenderTarget(RenderTexture* _renderTarget) {
        renderTarget_ = _renderTarget;
    }
};

// BasePostRenderingSystemを継承しているかどうか
template <typename T>
concept IsPostRenderSystem = std::is_base_of<BasePostRenderingSystem, T>::value;

} // namespace OriGine
