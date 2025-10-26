#pragma once

#include <wrl.h>

#include "d3d12.h"
#include "dxgi1_6.h"

/// <summary>
/// device,adapter,factory をまとめた構造体
/// </summary>
struct DxDevice {
public:
    void Initialize();
    void Finalize();

public:
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
    Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_  = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Device> device_       = nullptr;

};
