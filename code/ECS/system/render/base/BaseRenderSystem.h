#pragma once

#include "system/ISystem.h"

/// stl
#include <memory>

/// engine
// directX12Object
#include "directX12/BlendMode.h"
#include "directX12/DxCommand.h"

namespace OriGine {

/// <summary>
/// 全てのレンダリングシステムの基底となる抽象クラス。
/// DirectX12のコマンド管理、PSOの生成、描画サイクルの制御を担当する。
/// </summary>
class BaseRenderSystem
    : public ISystem {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="_priority">システムの更新優先順位</param>
    BaseRenderSystem(int32_t _priority = 0);

    /// <summary>
    /// デストラクタ
    /// </summary>
    virtual ~BaseRenderSystem();

    /// <summary>
    /// システムの初期化。DxCommandの生成とPSOの作成を行う。
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// システムの更新処理。レンダラーの登録とレンダリングの実行を行う。
    /// </summary>
    void Update() override;

    /// <summary>
    /// システムの終了処理。DxCommandの解放を行う。
    /// </summary>
    void Finalize() override;

    /// <summary>
    /// パイプラインステートオブジェクト(PSO)を作成する
    /// </summary>
    virtual void CreatePSO() = 0;

    /// <summary>
    /// レンダリングの開始処理（コマンドリストの設定など）
    /// </summary>
    virtual void StartRender() = 0;

    /// <summary>
    /// 指定されたブレンドモードとカリング設定で描画を行う
    /// </summary>
    /// <param name="_blendMode">ブレンドモード</param>
    /// <param name="_isCulling">カリングを有効にするかどうか</param>
    virtual void RenderingBy(BlendMode /*_blendMode*/, bool /*_isCulling*/) {};

    /// <summary>
    /// レンダリングの一連の処理（StartRender -> RenderingByのループ）を実行する
    /// </summary>
    virtual void Rendering();

    /// <summary>
    /// エンティティに関連する描画情報をレンダリングパイプラインに登録する
    /// </summary>
    /// <param name="_entity">対象のエンティティハンドル</param>
    virtual void DispatchRenderer(EntityHandle /*_entity*/){};

    /// <summary>
    /// レンダリングをスキップするかどうかを判定する
    /// </summary>
    /// <returns>true = 描画をスキップする / false = 描画を実行する</returns>
    virtual bool ShouldSkipRender() const {
        return true;
    }

protected:
    /// <summary>
    /// レンダリング用のDirectX12コマンド管理オブジェクト
    /// </summary>
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;

public:
    /// <summary>
    /// DirectX12コマンド管理オブジェクトを取得する
    /// </summary>
    /// <returns>DxCommandポインタ</returns>
    DxCommand* GetDxCommand() {
        return dxCommand_.get();
    }
};

// BaseRenderSystemを継承しているかどうか
template <typename T>
concept IsRenderSystem = std::is_base_of<BaseRenderSystem, T>::value;

} // namespace OriGine
