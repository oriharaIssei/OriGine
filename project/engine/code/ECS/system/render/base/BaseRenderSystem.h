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
    virtual ~BaseRenderSystem();

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
    /// <param name="blendMode">ブレンドモード</param>
    /// <param name="_isCulling">カリングの有効化</param>
    virtual void RenderingBy(BlendMode /*_blendMode*/, bool /*_isCulling*/) {};

    /// <summary>
    /// レンダリング処理(StartRenderから描画まですべてを行う)
    /// </summary>
    virtual void Rendering();

    /// <summary>
    /// 描画する物を登録
    /// </summary>
    /// <param name="_entity"></param>
    virtual void DispatchRenderer(Entity* /*_entity*/) {};

    /// <summary>
    /// レンダリングをスキップするかどうか(描画オブジェクトが無いときは描画をスキップする)
    /// </summary>
    /// <returns>true ＝ 描画をスキップする / false = 描画スキップしない</returns>
    virtual bool ShouldSkipRender() const {
        return true;
    }

protected:
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;

public:
    DxCommand* GetDxCommand() {
        return dxCommand_.get();
    }
};

// BaseRenderSystemを継承しているかどうか
template <typename T>
concept IsRenderSystem = std::is_base_of<BaseRenderSystem, T>::value;
