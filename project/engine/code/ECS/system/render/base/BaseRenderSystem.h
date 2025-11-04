#pragma once

#include "system/ISystem.h"

/// stl
#include <memory>

/// engine
// directX12Object
#include "directX12/BlendMode.h"
#include "directX12/DxCommand.h"

/// <summary>
/// レンダリング系システムの基底クラス
/// </summary>
class BaseRenderSystem
    : public ISystem {
public:
    BaseRenderSystem(int32_t _priority = 0);
    virtual ~BaseRenderSystem() = 0;

    void Initialize() override;
    void Update() override;
    void Finalize() override;

    /// <summary>
    /// PSOの作成
    /// </summary>
    virtual void CreatePSO() = 0;

    /// <summary>
    /// レンダリング開始処理
    /// </summary>
    virtual void StartRender() = 0;

    /// <summary>
    /// BlendModeごとに描画を行う
    /// </summary>
    /// <param name="blendMode"></param>
    virtual void RenderingBy(BlendMode blendMode) = 0;

    /// <summary>
    /// 描画する物を登録
    /// </summary>
    /// <param name="_entity"></param>
    virtual void DispatchRenderer(Entity* _entity) = 0;

    /// <summary>
    /// レンダリングをスキップするかどうか(描画オブジェクトが無いときは描画をスキップする)
    /// </summary>
    /// <returns>true ＝ 描画をスキップする / false = 描画スキップしない</returns>
    virtual bool IsSkipRendering() const {
        return true;
    }

protected:
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;
};
