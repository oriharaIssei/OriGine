#pragma once

#include <stdint.h>
#include <string>

#include "directX12/DxCommand.h"
#include "directX12/DxDescriptor.h"
#include "directX12/DxResource.h"
#include "directX12/PipelineStateObj.h"
#include <Engine.h>

#include "Vector2.h"
#include "Vector4.h"
#include <cstdint>

/// <summary>
/// RTVとSRVを持つレンダーターゲット用テクスチャ
/// </summary>
class RenderTexture {
private:
    static PipelineStateObj* pso_;

public:
    RenderTexture(DxCommand* dxCom);
    RenderTexture()  = default;
    ~RenderTexture() = default;

    /// <summary>
    /// Exe起動時に一度だけ呼ばれる.
    /// </summary>
    static void Awake();

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize(
        int32_t _bufferCount,
        const Vec2f& textureSize,
        DXGI_FORMAT format       = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        const Vec4f& _clearColor = {0.f, 0.f, 0.f, 1.f});
    void Initialize(int32_t _bufferCount, const DirectX::TexMetadata& _metaData, const Vec4f& _clearColor = {0.f, 0.f, 0.f, 1.f});

    /// <summary>
    /// リサイズ処理
    /// </summary>
    void Resize(const Vec2f& textureSize);

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

    /// <summary>
    /// RenderTexture への 書き込み準備
    /// </summary>
    void PreDraw();
    /// <summary>
    /// RenderTexture への 書き込み開始
    /// </summary>
    void PostDraw();

    /// <summary>
    /// RenderTextureの内容を描画
    /// </summary>
    void DrawTexture();
    /// <summary>
    /// RenderTextureに srvHandleのテクスチャを描画
    /// </summary>
    /// <param name="_srvHandle"></param>
    void DrawTexture(D3D12_GPU_DESCRIPTOR_HANDLE _srvHandle);

private:
    struct RenderTargetCombo {
        DxResource resource_;

        DxRtvDescriptor rtv_;
        DxSrvDescriptor srv_;
    };

private:
    DXGI_FORMAT format_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    std::vector<RenderTargetCombo> renderTargets_;
    int32_t backBufferIndex_  = 0;
    int32_t frontBufferIndex_ = 0;
    int32_t bufferCount_      = 0;

    DxDsvDescriptor* dxDsv_ = nullptr;

    std::unique_ptr<DxCommand> dxCommand_;

    std::string textureName_ = "unknown Texture";
    Vec2f textureSize_;
    Vec4f clearColor_;

public:
    void SetDxDsv(DxDsvDescriptor* _dsv) { dxDsv_ = _dsv; }

    void SetDxCommand(const std::string& _listName, const std::string& _queueName);

    void SetTextureName(const std::string& _name);

    const Vec2f& GetTextureSize() const { return textureSize_; }

    // back
    int32_t GetBackBufferIndex() const { return backBufferIndex_; }
    const Microsoft::WRL::ComPtr<ID3D12Resource>& GetBackBuffer() const { return renderTargets_[backBufferIndex_].resource_.GetResource(); }
    const DxRtvDescriptor& GetBackBufferRtv() const { return renderTargets_[backBufferIndex_].rtv_; }
    const DxSrvDescriptor& GetBackBufferSrv() const { return renderTargets_[backBufferIndex_].srv_; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetBackBufferSrvHandle() const { return renderTargets_[backBufferIndex_].srv_.GetGpuHandle(); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferRtvHandle() const { return renderTargets_[backBufferIndex_].rtv_.GetCpuHandle(); }
    // front
    int32_t GetFrontBufferIndex() const { return frontBufferIndex_; }
    const Microsoft::WRL::ComPtr<ID3D12Resource>& GetFrontBuffer() const { return renderTargets_[frontBufferIndex_].resource_.GetResource(); }
    const DxRtvDescriptor& GetFrontBufferRtv() const { return renderTargets_[frontBufferIndex_].rtv_; }
    const DxSrvDescriptor& GetFrontBufferSrv() const { return renderTargets_[frontBufferIndex_].srv_; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetFrontBufferSrvHandle() const { return renderTargets_[frontBufferIndex_].srv_.GetGpuHandle(); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetFrontBufferRtvHandle() const { return renderTargets_[frontBufferIndex_].rtv_.GetCpuHandle(); }

    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandle(int32_t index) const {
        return renderTargets_[index].srv_.GetGpuHandle();
    }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle(int32_t index) const {
        return renderTargets_[index].rtv_.GetCpuHandle();
    }
};
