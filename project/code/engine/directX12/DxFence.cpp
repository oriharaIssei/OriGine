#include "directX12/DxFence.h"

#include <cassert>

void DxFence::Init(ID3D12Device *device) {
	//初期値0でFenceを生成
	HRESULT hr = device->CreateFence(
		fenceValue_,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&fence_)
	);
	assert(SUCCEEDED(hr));
}

void DxFence::Finalize() {
	fence_.Reset();
}

void DxFence::WaitForFence() {
	if(fence_->GetCompletedValue() < fenceValue_) {
		HANDLE fenceEvent = CreateEvent(nullptr,false,false,nullptr);
		fence_->SetEventOnCompletion(fenceValue_,fenceEvent);
		WaitForSingleObject(fenceEvent,INFINITE);
		CloseHandle(fenceEvent);
	}
}