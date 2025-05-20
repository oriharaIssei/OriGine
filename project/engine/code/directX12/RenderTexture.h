#pragma once

#include <stdint.h>
#include <string>

#include "directX12/DxCommand.h"
#include "directX12/DxResource.h"
#include "directX12/DxRtvArray.h"
#include "directX12/DxSrvArray.h"

#include "Vector2.h"
#include "Vector4.h"

#include "directX12/PipelineStateObj.h"

class RenderTexture {
public:
    RenderTexture(DxCommand* dxCom, DxRtvArray* rtvArray, DxSrvArray* srvArray) : dxCommand_(dxCom), rtvArray_(rtvArray), srvArray_(srvArray) {}
    ~RenderTexture() = default;

    static void Awake();
    void Initialize(int32_t _bufferCount, const Vec2f& textureSize, DXGI_FORMAT format, const Vec4f& _clearColor);
    void Resize(const Vec2f& textureSize);

    void Finalize();

    /// <summary>
    /// RenderTexture への 書き込み準備
    /// </summary>
    void PreDraw();
    /// <summary>
    /// RenderTexture への 書き込み開始
    /// </summary>
    void PostDraw();

    void DrawTexture();

private:
    static PipelineStateObj* pso_;

    struct RenderTargetCombo {
        DxResource resource_;

        uint32_t rtvIndex_ = 0;
        uint32_t srvIndex_ = 0;
    };

    DXGI_FORMAT format_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    std::vector<RenderTargetCombo> renderTargets_;
    int32_t backBufferIndex_  = 0;
    int32_t frontBufferIndex_ = 0;
    int32_t bufferCount_      = 0;

    DxCommand* dxCommand_;

    DxRtvArray* rtvArray_;
    DxSrvArray* srvArray_;

    Vec2f textureSize_;
    Vec4f clearColor_;

public:
    void setTextureName(const std::wstring& _name);

    const Vec2f& getTextureSize() const { return textureSize_; }

    // back
    ID3D12Resource* getBackBuffer() const { return renderTargets_[backBufferIndex_].resource_.getResource(); }
    D3D12_GPU_DESCRIPTOR_HANDLE getBackBufferSrvHandle() const { return DxHeap::getInstance()->getSrvGpuHandle(srvArray_->getLocationOnHeap(renderTargets_[backBufferIndex_].srvIndex_)); }
    D3D12_CPU_DESCRIPTOR_HANDLE getBackBufferRtvHandle() const { return DxHeap::getInstance()->getRtvCpuHandle(rtvArray_->getLocationOnHeap(renderTargets_[backBufferIndex_].rtvIndex_)); }
    // front
    ID3D12Resource* getFrontBuffer() const { return renderTargets_[frontBufferIndex_].resource_.getResource(); }
    D3D12_CPU_DESCRIPTOR_HANDLE getFrontBufferRtvHandle() const { return DxHeap::getInstance()->getRtvCpuHandle(rtvArray_->getLocationOnHeap(renderTargets_[frontBufferIndex_].rtvIndex_)); }
    D3D12_GPU_DESCRIPTOR_HANDLE getFrontBufferSrvHandle() const { return DxHeap::getInstance()->getSrvGpuHandle(srvArray_->getLocationOnHeap(renderTargets_[frontBufferIndex_].srvIndex_)); }
};
