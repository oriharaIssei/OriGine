#pragma once

#include "DirectXTex/DirectXTex.h"
#include <d3d12.h>

#include <wrl.h>

#include <vector>

#include "directX12/DxHeap.h"

class DxSrvArrayManager;
class DxSrvArray{
	friend class DxSrvArrayManager;
private:
public:
	void Finalize();

	uint32_t CreateView(ID3D12Device *device,
						D3D12_SHADER_RESOURCE_VIEW_DESC &viewDesc,
						ID3D12Resource* resource);
	void DestroyView(uint32_t srvIndex);
private:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="textureSize">テクスチャーマネージャーで静的に確保されている数</param>
	void Init(uint32_t size,uint32_t arrayLocation);
private:
	uint32_t arrayStartLocation_;
	uint32_t size_;
	/// <summary>
	/// コンパイル後にサイズを決めるため vector にしているが，
	/// Init後サイズを変更してはいけない
	/// </summary>
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> resources_;
public:
	uint32_t getSize()const{ return size_; }

	ID3D12Resource *getSrv(uint32_t index)const{ return resources_[index].Get(); }
	Microsoft::WRL::ComPtr<ID3D12Resource> getSrvComPtr(uint32_t index)const{ return resources_[index]; }

	uint32_t getLocationOnHeap(uint32_t index)const{
        if (size_ <= index) {
            assert(false);
            return 0;
        }
        return index + arrayStartLocation_; }
};
