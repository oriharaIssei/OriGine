#pragma once

#include <wrl.h>

#include "winApp/WinApp.h"

#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/DxFence.h"
#include "directX12/DxHeap.h"
#include "directX12/DxSwapChain.h"
#include "directX12/ResourceBarrierManager.h"

#include "Vector2.h"
#include "Vector4.h"

class DxFunctionHelper{
public:

	static void SetViewportsAndScissor(const DxCommand* dxCommand,const WinApp* window);
	static void SetViewportsAndScissor(const DxCommand* dxCommand,const Vector2& rectSize);

	static void SetRenderTargets(const DxCommand* dxCommand,const DxSwapChain* dxSwapChain);
	static void SetRenderTargets(const DxCommand* dxCommand,D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

	static void PreDraw(const DxCommand* dxCommand,const WinApp* window,const DxSwapChain* dxSwapChain);
	static void PreDraw(const DxCommand* dxCommand,const Vector2& rectSize,const DxSwapChain* dxSwapChain);

	static void PostDraw(DxCommand* dxCommands,DxFence* dxFence,DxSwapChain* dxSwapChain);
};
using DxFH = DxFunctionHelper;