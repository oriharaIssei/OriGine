#include "directX12/DxSwapChain.h"

#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "winApp/WinApp.h"

#include <cassert>

#include "directX12/DxRtvArrayManager.h"

void DxSwapChain::Initialize(const WinApp *winApp,const DxDevice *device,const DxCommand *command){
	///================================================
	///	SwapChain の生成
	///================================================
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
	swapchainDesc.Width = winApp->getWidth();                    //画面の幅。windowと同じにする
	swapchainDesc.Height = winApp->getHeight();                  //画面の高さ。windowと同じにする
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.SampleDesc.Count = 1;                          //マルチサンプルしない
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //描画のターゲットとして利用する
	swapchainDesc.BufferCount = 2;                               //ダブルバッファ
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;    //モニタに移したら中身を破棄する

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
	HRESULT result = device->getDxgiFactory()->CreateSwapChainForHwnd(
		command->getCommandQueue(),
		winApp->getHwnd(), //描画対象のWindowのハンドル
		&swapchainDesc,
		nullptr,
		nullptr,
		&swapChain1
	);
	assert(SUCCEEDED(result));

	// SwapChain4を得る
	result = swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain_));
	assert(SUCCEEDED(result));
	///================================================
	///	Resource の初期化
	///================================================
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	bufferCount_ = 2;

	backBuffers_ = DxRtvArrayManager::getInstance()->Create(bufferCount_);

	for(int i = 0; i < (int)bufferCount_; ++i){
		Microsoft::WRL::ComPtr<ID3D12Resource> buff;
		result = swapChain_->GetBuffer(
			i,IID_PPV_ARGS(&buff)
		);
		assert(SUCCEEDED(result));

		backBuffers_->CreateView(device->getDevice(),rtvDesc,buff);
	}
	///================================================
}

void DxSwapChain::Finalize(){
	backBuffers_->Finalize();
	swapChain_.Reset();
}

void DxSwapChain::Present(){
	swapChain_->Present(1,0);
}

void DxSwapChain::CurrentBackBufferClear(ID3D12GraphicsCommandList* commandList)const{
	backBuffers_->ClearTarget(swapChain_->GetCurrentBackBufferIndex(),commandList,clearColor_);
}
