#include "directX12/DxFunctionHelper.h"

#include <cassert>

void DxFunctionHelper::SetViewportsAndScissor(const DxCommand* dxCommand,const WinApp* window){
	ID3D12GraphicsCommandList* commandList = dxCommand->getCommandList();
	//ビューポートの設定
	D3D12_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(window->getWidth());
	viewPort.Height = static_cast<float>(window->getHeight());
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	commandList->RSSetViewports(1,&viewPort);

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = window->getWidth();
	scissorRect.top = 0;
	scissorRect.bottom = window->getHeight();

	commandList->RSSetScissorRects(1,&scissorRect);
}

void DxFunctionHelper::SetViewportsAndScissor(const DxCommand* dxCommand,const Vec2f& rectSize){
	ID3D12GraphicsCommandList* commandList = dxCommand->getCommandList();
	//ビューポートの設定
	D3D12_VIEWPORT viewPort{};
	viewPort.Width = rectSize[X];
	viewPort.Height = rectSize[Y];
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	commandList->RSSetViewports(1,&viewPort);

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = static_cast<LONG>(rectSize[X]);
	scissorRect.top = 0;
	scissorRect.bottom = static_cast<LONG>(rectSize[Y]);

	commandList->RSSetScissorRects(1,&scissorRect);
}

void DxFunctionHelper::SetRenderTargets(const DxCommand* dxCommand,const DxSwapChain* dxSwapChain){
	ID3D12GraphicsCommandList* commandList = dxCommand->getCommandList();

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = DxHeap::getInstance()->getRtvCpuHandle(dxSwapChain->getCurrentBackBufferIndex());
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DxHeap::getInstance()->getDsvCpuHandle(0);
	commandList->OMSetRenderTargets(1,&rtvHandle,FALSE,&dsvHandle);
}

void DxFunctionHelper::SetRenderTargets(const DxCommand* dxCommand,D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle){
	ID3D12GraphicsCommandList* commandList = dxCommand->getCommandList();

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DxHeap::getInstance()->getDsvCpuHandle(0);
	commandList->OMSetRenderTargets(1,&rtvHandle,FALSE,&dsvHandle);
}

void DxFunctionHelper::PreDraw(const DxCommand* command,const WinApp* window,const DxSwapChain* swapChain){
	///=========================================
	//	TransitionBarrierの設定
	///=========================================
	ID3D12GraphicsCommandList* commandList = command->getCommandList();

	ResourceBarrierManager::Barrier(
		commandList,
		swapChain->getCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = DxHeap::getInstance()->getRtvCpuHandle(swapChain->getCurrentBackBufferIndex());
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DxHeap::getInstance()->getDsvCpuHandle(0);
	commandList->OMSetRenderTargets(1,&rtvHandle,FALSE,&dsvHandle);

	//ビューポートの設定
	D3D12_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(window->getWidth());
	viewPort.Height = static_cast<float>(window->getHeight());
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	commandList->RSSetViewports(1,&viewPort);

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = window->getWidth();
	scissorRect.top = 0;
	scissorRect.bottom = window->getHeight();

	commandList->RSSetScissorRects(1,&scissorRect);

	swapChain->CurrentBackBufferClear(commandList);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DxFunctionHelper::PreDraw(const DxCommand* dxCommand,const Vec2f& rectSize,const DxSwapChain* dxSwapChain){
	///=========================================
	//	TransitionBarrierの設定
	///=========================================
	ID3D12GraphicsCommandList* commandList = dxCommand->getCommandList();

	ResourceBarrierManager::Barrier(
		commandList,
		dxSwapChain->getCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = DxHeap::getInstance()->getRtvCpuHandle(dxSwapChain->getCurrentBackBufferIndex());
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DxHeap::getInstance()->getDsvCpuHandle(0);
	commandList->OMSetRenderTargets(1,&rtvHandle,FALSE,&dsvHandle);

	//ビューポートの設定
	D3D12_VIEWPORT viewPort{};
	viewPort.Width = static_cast<FLOAT>(rectSize[X]);
	viewPort.Height = static_cast<FLOAT>(rectSize[Y]);
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	commandList->RSSetViewports(1,&viewPort);

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = static_cast<LONG>(rectSize[X]);
	scissorRect.top = 0;
	scissorRect.bottom = static_cast<LONG>(rectSize[Y]);

	commandList->RSSetScissorRects(1,&scissorRect);

	dxSwapChain->CurrentBackBufferClear(dxCommand->getCommandList());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DxFunctionHelper::PostDraw(DxCommand* dxCommand,DxFence* fence,DxSwapChain* swapChain){
	HRESULT hr;
	ID3D12GraphicsCommandList* commandList = dxCommand->getCommandList();
	///===============================================================
	///	バリアの更新(描画->表示状態)
	///===============================================================
	ResourceBarrierManager::Barrier(
		commandList,
		swapChain->getCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT
	);
	///===============================================================

	// コマンドの受付終了 -----------------------------------
	hr = commandList->Close();
	//----------------------------------------------------

	///===============================================================
	/// コマンドリストの実行
	///===============================================================
	ID3D12CommandList* ppHeaps[] = {commandList};
	dxCommand->getCommandQueue()->ExecuteCommandLists(1,ppHeaps);
	///===============================================================

	swapChain->Present();

	///===============================================================
	/// コマンドリストの実行を待つ
	///===============================================================
	fence->Signal(dxCommand->getCommandQueue());
	fence->WaitForFence();
	///===============================================================

	///===============================================================
	/// リセット
	///===============================================================
	dxCommand->CommandReset();
	///===============================================================
}