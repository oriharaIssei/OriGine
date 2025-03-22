#pragma once

#include <wrl.h>

#include "d3d12.h"
#include "dxgi1_6.h"

class DxDevice{
public:
	void Initialize();
	void Finalize();
private:
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;
public:
	IDXGIFactory7* getDxgiFactory()const{ return dxgiFactory_.Get(); }
	IDXGIAdapter4* getAdapter()const{ return useAdapter_.Get(); }
	ID3D12Device* getDevice()const{ return device_.Get(); }
};