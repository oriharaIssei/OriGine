#pragma once

#include <memory>
#include <wrl.h>

#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "directX12/DxRtvArray.h"
#include <WinUser.h>

/// math
#include "Vector4.h"

class DxDevice;
class DxCommand;
class WinApp;
class DxSwapChain {
public:
   void Initialize(const WinApp* winApp, const DxDevice* device, const DxCommand* command);
   void Finalize();

   void Present();

   void CurrentBackBufferClear(ID3D12GraphicsCommandList* commandList) const;

   void ResizeBuffer(const DxDevice* device, UINT width, UINT height);

private:
   Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

   std::shared_ptr<DxRtvArray> backBuffers_;
   UINT bufferCount_;

   UINT bufferWidth_  = 0;
   UINT bufferHeight_ = 0;

   const Vec4f clearColor_ = Vec4f{0.2f, 0.2f, 0.2f, 1.0f}; 

public:
   UINT getBufferCount() const { return bufferCount_; }
   UINT getCurrentBackBufferIndex() const { return swapChain_->GetCurrentBackBufferIndex(); }

   IDXGISwapChain4* getSwapChain() const { return swapChain_.Get(); }

   ID3D12Resource* getBackBuffer(UINT index) const { return backBuffers_->getRtv(index); }
   ID3D12Resource* getCurrentBackBuffer() const { return backBuffers_->getRtv(swapChain_->GetCurrentBackBufferIndex()); }
};
