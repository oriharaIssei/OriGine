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

    void Initialize() override;
    void Update() override;
    void Finalize() override;

protected:
    /// <summary>
    /// PSO作成
    /// </summary>
    virtual void CreatePSO() = 0;

    /// <summary>
    /// レンダリング開始処理
    /// </summary>
    virtual void RenderStart() = 0;
    /// <summary>
    /// レンダリング処理
    /// </summary>
    virtual void Rendering() = 0;
    /// <summary>
    /// レンダリング終了処理
    /// </summary>
    virtual void RenderEnd() = 0;

    /// <summary>
    /// PostEffectに使用するComponentを登録する
    /// (Systemによっては使用しない)
    /// </summary>
    virtual void DispatchComponent(EntityHandle /*_owner*/) {}

    /// <summary>
    /// ポストレンダリングをスキップするかどうか
    /// </summary>
    /// <returns>true = skipする / false = skipしない</returns>
    virtual bool ShouldSkipPostRender() const {
        return false;
    }

protected:
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
    RenderTexture* renderTarget_          = nullptr;

public:
    RenderTexture* GetRenderTarget() const {
        return renderTarget_;
    }
    void SetRenderTarget(RenderTexture* _renderTarget) {
        renderTarget_ = _renderTarget;
    }
};


// BasePostRenderingSystemを継承しているかどうか
template <typename T>
concept IsPostRenderSystem = std::is_base_of<BasePostRenderingSystem, T>::value;

} // namespace OriGine
