#include "directX12/dxFunctionHelper/DxFunctionHelper.h"

#include <cassert>

void DxFunctionHelper::CreateBufferResource(const DxDevice* device,Microsoft::WRL::ComPtr<ID3D12Resource>& resource,size_t sizeInBytes){
	//頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//UploadHeapを使う
	//頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	//バッファのリソース(テクスチャの場合は別の設定をする)
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;
	//バッファの場合、これらは 1 にする
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT hr = device->getDevice()->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));
}

Microsoft::WRL::ComPtr<ID3D12Resource> DxFunctionHelper::CreateRenderTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device,uint32_t width,uint32_t height,DXGI_FORMAT format,const Vector4& clearColor){
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource;
	D3D12_RESOURCE_DESC resourceDesc;
	// RenderTarget として 利用可能に
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resourceDesc.Width = width;
	resourceDesc.Height = height;

	D3D12_HEAP_PROPERTIES heapProps{};
	// VRAM 上に 生成
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = format;
	clearValue.Color[0] = clearColor.x;
	clearValue.Color[1] = clearColor.y;
	clearValue.Color[2] = clearColor.z;
	clearValue.Color[3] = clearColor.w;

	device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET, // 描画すること を 前提とした テクスチャ なので RenderTarget として 扱う
		&clearValue,
		IID_PPV_ARGS(&renderTextureResource)
	);

	return renderTextureResource;
}

void DxFunctionHelper::ClearRenderTarget(const DxCommand* command,const DxSwapChain* swapChain){
	UINT backBufferIndex = swapChain->getCurrentBackBufferIndex();

	DxHeap* heap = DxHeap::getInstance();

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = heap->getDsvHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE backBufferRtvHandle = heap->getRtvCpuHandle(backBufferIndex);
	command->getCommandList()->OMSetRenderTargets(
		1,
		&backBufferRtvHandle,
		false,
		&dsvHandle
	);

	float clearColor[] = {0.1f,0.25f,0.5f,1.0f};
	command->getCommandList()->ClearRenderTargetView(
		backBufferRtvHandle,clearColor,0,nullptr
	);

	command->getCommandList()->ClearDepthStencilView(
		dsvHandle,D3D12_CLEAR_FLAG_DEPTH,1.0f,0,0,nullptr
	);
}

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

void DxFunctionHelper::SetViewportsAndScissor(const DxCommand* dxCommand,const Vector2& rectSize){
	ID3D12GraphicsCommandList* commandList = dxCommand->getCommandList();
	//ビューポートの設定
	D3D12_VIEWPORT viewPort{};
	viewPort.Width = rectSize.x;
	viewPort.Height = rectSize.y;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	commandList->RSSetViewports(1,&viewPort);

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = static_cast<LONG>(rectSize.x);
	scissorRect.top = 0;
	scissorRect.bottom = static_cast<LONG>(rectSize.y);

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

	ClearRenderTarget(command,swapChain);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DxFunctionHelper::PreDraw(const DxCommand* dxCommand,const Vector2& rectSize,const DxSwapChain* dxSwapChain){
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
	viewPort.Width = static_cast<FLOAT>(rectSize.x);
	viewPort.Height = static_cast<FLOAT>(rectSize.y);
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	commandList->RSSetViewports(1,&viewPort);

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = static_cast<LONG>(rectSize.x);
	scissorRect.top = 0;
	scissorRect.bottom = static_cast<LONG>(rectSize.y);

	commandList->RSSetScissorRects(1,&scissorRect);

	ClearRenderTarget(dxCommand,dxSwapChain);
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