#pragma once

/// stl
#include <memory>
#include <string>
#include <vector>

/// engine
#include <Engine.h>
// directX12
#include "directX12/DxCommand.h"
#include "directX12/DxDescriptor.h"
#include "directX12/DxResource.h"
#include "directX12/PipelineStateObj.h"

/// math
#include "Vector2.h"
#include "Vector4.h"
#include <cstdint>
#include <stdint.h>

namespace OriGine {

/// <summary>
/// レンダーターゲットとして使用可能なテクスチャ定数リソースを保持するクラス.
/// RTV (書き込み用) と SRV (読み込み用) の両方を持ち、オフスクリーンレンダリングやポストフェクトのバッファとして利用される.
/// 複数のバッファを持つことで、前フレームの結果を参照しながら次フレームを書くなどのピンポンバッファリングが可能.
/// </summary>
class RenderTexture {
private:
    /// <summary>描画用の共有パイプラインステート</summary>
    static PipelineStateObj* pso_;

public:
    RenderTexture(DxCommand* dxCom);
    RenderTexture()  = default;
    ~RenderTexture() = default;

    /// <summary>
    /// アプリケーション起動時に一度だけ呼び出し、共有リソース(PSO等)を初期化する.
    /// </summary>
    static void Awake();

    /// <summary>
    /// 指定したサイズとフォーマットでレンダーターゲットテクスチャを初期化する.
    /// </summary>
    /// <param name="_bufferCount">生成するバッファの数 (1で通常のターゲット、2でピンポンバッファ)</param>
    /// <param name="textureSize">解像度</param>
    /// <param name="format">ピクセルフォーマット</param>
    /// <param name="_clearColor">クリア時の色</param>
    void Initialize(
        int32_t _bufferCount,
        const Vec2f& textureSize,
        DXGI_FORMAT format       = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        const Vec4f& _clearColor = {0.f, 0.f, 0.f, 1.f});

    /// <summary>
    /// メタデータを指定して初期化する.
    /// </summary>
    void Initialize(int32_t _bufferCount, const DirectX::TexMetadata& _metaData, const Vec4f& _clearColor = {0.f, 0.f, 0.f, 1.f});

    /// <summary>
    /// 解像度を変更する. 既存のリソースは作り直される.
    /// </summary>
    void Resize(const Vec2f& textureSize);

    /// <summary>
    /// リソースを解放する.
    /// </summary>
    void Finalize();

    /// <summary>
    /// 書き込み対象を切り替え、レンダーターゲットとして設定する準備を行う.
    /// </summary>
    void PreDraw();

    /// <summary>
    /// 書き込みを終了し、描画内容をリソースバリアで SRV 状態へ遷移させる.
    /// </summary>
    void PostDraw();

    /// <summary>
    /// この RenderTexture に書き込まれた内容を画面全体に描画（コピー）する.
    /// </summary>
    void DrawTexture();

    /// <summary>
    /// 指定された GPU ディスクリプタハンドルが指すテクスチャをこの RenderTexture の領域に描画する.
    /// </summary>
    /// <param name="_srvHandle">描画元の SRV ハンドル</param>
    void DrawTexture(D3D12_GPU_DESCRIPTOR_HANDLE _srvHandle);

private:
    /// <summary>
    /// 一つのレンダーターゲットリソースと、それに対応する各ビューのセット.
    /// </summary>
    struct RenderTargetCombo {
        /// <summary>テクスチャリソース本体</summary>
        DxResource resource_;
        /// <summary>レンダーターゲットビュー</summary>
        DxRtvDescriptor rtv_;
        /// <summary>シェーダーリソースビュー</summary>
        DxSrvDescriptor srv_;
    };

private:
    /// <summary>ピクセルフォーマット</summary>
    DXGI_FORMAT format_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    /// <summary>内部で保持するレンダーターゲットリソースのリスト</summary>
    std::vector<RenderTargetCombo> renderTargets_;
    /// <summary>現在書き込み対象 (RenderTarget) となっているバッファのインデックス</summary>
    int32_t backBufferIndex_ = 0;
    /// <summary>現在読み取り対象 (ShaderResource) となっているバッファのインデックス</summary>
    int32_t frontBufferIndex_ = 0;
    /// <summary>総バッファ数</summary>
    int32_t bufferCount_ = 0;

    /// <summary>深度バッファへの参照ポインタ (外部管理)</summary>
    DxDsvDescriptor* dxDsv_ = nullptr;

    /// <summary>描画命令の発行に使用するコマンド管理オブジェクト</summary>
    std::unique_ptr<DxCommand> dxCommand_;

    /// <summary>デバッグ用のリソース名</summary>
    std::string textureName_ = "unknown Texture";
    /// <summary>テクスチャ解像度</summary>
    Vec2f textureSize_;
    /// <summary>クリアカラー</summary>
    Vec4f clearColor_;

public:
    /// <summary>
    /// 使用する深度ステンシルビュー (DSV) を登録する.
    /// </summary>
    void SetDxDsv(DxDsvDescriptor* _dsv) { dxDsv_ = _dsv; }

    /// <summary>
    /// 内部で使用する DxCommand に名前を設定して有効化する.
    /// </summary>
    void SetDxCommand(const std::string& _listName, const std::string& _queueName);

    /// <summary>
    /// テクスチャ名を設定する. (リソース名にも反映される)
    /// </summary>
    void SetTextureName(const std::string& _name);

    /// <summary>
    /// 現在のテクスチャ解像度を取得する.
    /// </summary>
    const Vec2f& GetTextureSize() const { return textureSize_; }

    // --- Back Buffer (Writing target) ---

    /// <summary>
    /// 現在書き込み対象となっているバックバッファのインデックスを取得する.
    /// </summary>
    int32_t GetBackBufferIndex() const { return backBufferIndex_; }

    /// <summary>
    /// バックバッファの実体リソースを取得する.
    /// </summary>
    const Microsoft::WRL::ComPtr<ID3D12Resource>& GetBackBuffer() const { return renderTargets_[backBufferIndex_].resource_.GetResource(); }

    /// <summary>
    /// バックバッファの RTV ディスクリプタラッパーを取得する.
    /// </summary>
    const DxRtvDescriptor& GetBackBufferRtv() const { return renderTargets_[backBufferIndex_].rtv_; }

    /// <summary>
    /// バックバッファの SRV ディスクリプタラッパーを取得する. (書き込み中でも参照が必要な場合のみ)
    /// </summary>
    const DxSrvDescriptor& GetBackBufferSrv() const { return renderTargets_[backBufferIndex_].srv_; }

    /// <summary>
    /// バックバッファの SRV GPU ハンドルを取得する.
    /// </summary>
    D3D12_GPU_DESCRIPTOR_HANDLE GetBackBufferSrvHandle() const { return renderTargets_[backBufferIndex_].srv_.GetGpuHandle(); }

    /// <summary>
    /// バックバッファの RTV CPU ハンドルを取得する. (OMSetRenderTargets 用)
    /// </summary>
    D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferRtvHandle() const { return renderTargets_[backBufferIndex_].rtv_.GetCpuHandle(); }

    // --- Front Buffer (Reading target / Previous frame result) ---

    /// <summary>
    /// 現在読み込み対象となっているフロントバッファ（前回書き込まれたバッファ）のインデックスを取得する.
    /// </summary>
    int32_t GetFrontBufferIndex() const { return frontBufferIndex_; }

    /// <summary>
    /// フロントバッファの実体リソースを取得する.
    /// </summary>
    const Microsoft::WRL::ComPtr<ID3D12Resource>& GetFrontBuffer() const { return renderTargets_[frontBufferIndex_].resource_.GetResource(); }

    /// <summary>
    /// フロントバッファの RTV ディスクリプタラッパーを取得する.
    /// </summary>
    /// <returns>RTV ディスクリプタ</returns>
    const DxRtvDescriptor& GetFrontBufferRtv() const { return renderTargets_[frontBufferIndex_].rtv_; }

    /// <summary>
    /// フロントバッファの SRV ディスクリプタラッパーを取得する. (通常はこちらをシェーダーに渡す)
    /// </summary>
    /// <returns>SRV ディスクリプタ</returns>
    const DxSrvDescriptor& GetFrontBufferSrv() const { return renderTargets_[frontBufferIndex_].srv_; }

    /// <summary>
    /// フロントバッファの SRV GPU ハンドルを取得する.
    /// </summary>
    /// <returns>GPU ハンドル</returns>
    D3D12_GPU_DESCRIPTOR_HANDLE GetFrontBufferSrvHandle() const { return renderTargets_[frontBufferIndex_].srv_.GetGpuHandle(); }

    /// <summary>
    /// フロントバッファの RTV CPU ハンドルを取得する.
    /// </summary>
    /// <returns>CPU ハンドル</returns>
    D3D12_CPU_DESCRIPTOR_HANDLE GetFrontBufferRtvHandle() const { return renderTargets_[frontBufferIndex_].rtv_.GetCpuHandle(); }

    /// <summary>
    /// 指定インデックスのリソースから SRV GPU ハンドルを取得する.
    /// </summary>
    /// <param name="index">バッファインデックス</param>
    /// <returns>GPU ハンドル</returns>
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandle(int32_t index) const {
        return renderTargets_[index].srv_.GetGpuHandle();
    }

    /// <summary>
    /// 指定インデックスのリソースから RTV CPU ハンドルを取得する.
    /// </summary>
    /// <param name="index">バッファインデックス</param>
    /// <returns>CPU ハンドル</returns>
    D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle(int32_t index) const {
        return renderTargets_[index].rtv_.GetCpuHandle();
    }
};

} // namespace OriGine
