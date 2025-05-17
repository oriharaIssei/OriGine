#pragma once

#include <unordered_map>

#include <wrl.h>

#include <d3d12.h>

class ResourceBarrierManager{
public:
	ResourceBarrierManager() = default;

	// リソースを登録
	static void RegisterReosurce(ID3D12Resource* resource,D3D12_RESOURCE_STATES initialState){
		resourceStates_[resource] = initialState;
	}

	// リソースの状態遷移バリアを生成
	static void Barrier(ID3D12GraphicsCommandList* commandList,ID3D12Resource* resource,D3D12_RESOURCE_STATES stateAfter);

	static void SetState(ID3D12Resource* resource,D3D12_RESOURCE_STATES stateAfter);

private:
	static std::unordered_map<ID3D12Resource*,D3D12_RESOURCE_STATES> resourceStates_;
};
