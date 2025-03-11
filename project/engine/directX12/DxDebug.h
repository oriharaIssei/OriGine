#pragma once

#include <wrl.h>

#include <d3d12.h>

class DxDebug {
public:
	DxDebug();
	~DxDebug();
private:
	Microsoft::WRL::ComPtr <ID3D12Debug1> debugController_ = nullptr;
};