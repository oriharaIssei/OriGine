#pragma once

#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include "directX12/dxHeap/DxHeap.h"

#include "Vector2.h"
#include <stdint.h>

class DxRtvArray{
	friend class DxRtvArrayManager;
private:
public:
	void Finalize();

	uint32_t CreateView(ID3D12Device *device,
						D3D12_RENDER_TARGET_VIEW_DESC &viewDesc,
						const Microsoft::WRL::ComPtr<ID3D12Resource> &resource);
	void DestroyView(uint32_t index);

	void ClearTarget(uint32_t index,ID3D12GraphicsCommandList* commandList,const float* color);

private:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="textureSize">Array の Viewの格納数</param>
	void Init(uint32_t size,uint32_t arrayLocation);
private:
	uint32_t arrayStartLocation_;
	uint32_t size_;
	/// <summary>
	/// コンパイル後にサイズを決めるためvectorにしているが，
	/// Init後サイズを変更してはいけない
	/// </summary>
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> resources_;
public:
	uint32_t getSize()const{ return size_; }

	ID3D12Resource *getRtv(uint32_t index)const{ return resources_[index].Get(); }
	Microsoft::WRL::ComPtr<ID3D12Resource> getRtvComPtr(uint32_t index)const{ return resources_[index]; }

	uint32_t getLocationOnHeap(uint32_t index)const{ return index + arrayStartLocation_; }
};