#pragma once

#include <d3d12.h>
#include <wrl.h>

#include "DirectXTex/DirectXTex.h"

#include "Vector4.h"
#include <stdint.h>

class DxDevice;
class DxResource {
public:
    DxResource()  = default;
    ~DxResource() = default;

    void CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
    void CreateRenderTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vec4f& clearColor);
    void CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);
    void Finalize();

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> resource_;

public:
    ID3D12Resource* getResource() const { return resource_.Get(); }
    HRESULT setName(const std::wstring& name);
};
